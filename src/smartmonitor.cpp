// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartmonitor.h"

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/Block>
#include <Solid/StorageDrive>
#include <Solid/StorageVolume>

#include <QDebug>

#include "device.h"
#include "smartctl.h"
#include "smartdata.h"

SMARTMonitor::SMARTMonitor(AbstractSMARTCtl *ctl, QObject *parent)
    : QObject(parent)
    , m_ctl(ctl)
{
    connect(&m_reloadTimer, &QTimer::timeout,
            this, &SMARTMonitor::reloadData);
    connect(ctl, &AbstractSMARTCtl::finished,
            this, &SMARTMonitor::onSMARTCtlFinished);
    m_reloadTimer.setInterval(1000 * 60 /*minute*/ * 60 /*hour*/ * 24 /*day*/);
}

void SMARTMonitor::start()
{
    qDebug() << "starting";
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
            this, &SMARTMonitor::checkUDI);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved,
            this, &SMARTMonitor::removeUDI);
    QMetaObject::invokeMethod(this, &SMARTMonitor::reloadData);
}

QVector<Device *> SMARTMonitor::devices() const
{
    return m_devices;
}

void SMARTMonitor::checkUDI(const QString &udi)
{
    Solid::Device dev(udi);
    checkDevice(dev);
}

void SMARTMonitor::removeUDI(const QString &udi)
{
    std::remove_if(m_devices.begin(), m_devices.end(), [this, udi](Device *dev) {
        if (dev->udi() != udi) {
            return false;
        }

        emit deviceRemoved(dev);
        dev->deleteLater();
        return true;
    });
}

void SMARTMonitor::reloadData()
{
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    for (const auto &device : devices) {
        checkDevice(device);
    }
    m_reloadTimer.start();
}

void SMARTMonitor::onSMARTCtlFinished(const QString &devicePath, const QJsonDocument &document)
{
    auto pendingIt = m_pendingDevices.find(devicePath);
    if (pendingIt == m_pendingDevices.end()) {
        qDebug() << "unexpected pending result for" << devicePath;
        return;
    }
    Device *device = *pendingIt;
    m_pendingDevices.erase(pendingIt);

    if (document.isEmpty()) { // failed to get data, ignore the device
        device->deleteLater();
        return;
    }

    SMARTData data(document);

    auto existingIt = std::find_if(m_devices.begin(), m_devices.end(), [&device](Device *existing) {
            return *existing == *device;
    });
    if (existingIt != m_devices.cend()) {
        device->deleteLater(); // won't be needing this

        Device *existing = *existingIt;
        // update failure and call it a day. Notification is handled by the Device.
        existing->setFailed(!data.m_status.m_passed);

        return;
    }
    device->setFailed(!data.m_status.m_passed);

    m_devices << device;
    emit deviceAdded(device);
}

void SMARTMonitor::checkDevice(const Solid::Device &device)
{
    qDebug() << "!!!! " << device.udi();

    // This seems fairly awkward on a solid level. The actual device
    // isn't really trivial to identify. It certainly mustn't be a
    // filesystem but beyond that it's entirely unclear.
    // The trouble here is that we'll only want to run smartctl on
    // actual devices, not the partitions on the devices as otherwise
    // we'll have trouble validating the output as we'd not know
    // if it is incomplete because the device wasn't a device or
    // there's no data or smartctl is broken or the auth helper is broken...
    if (!device.is<Solid::StorageVolume>()) {
        qDebug() << "   not a volume";
        return; // certainly not an interesting device
    }
    switch (device.as<Solid::StorageVolume>()->usage()) {
    case Solid::StorageVolume::Unused: Q_FALLTHROUGH();
    case Solid::StorageVolume::FileSystem: Q_FALLTHROUGH();
    case Solid::StorageVolume::Encrypted: Q_FALLTHROUGH();
    case Solid::StorageVolume::Other: Q_FALLTHROUGH();
    case Solid::StorageVolume::Raid:
        qDebug() << "   bad type" << device.as<Solid::StorageVolume>()->usage();
        return;
    case Solid::StorageVolume::PartitionTable:
        break;
    }

    qDebug() << "evaluating!";

    checkDevice(new Device(device));
}

void SMARTMonitor::checkDevice(Device *device)
{
    m_pendingDevices[device->path()] = device;
    m_ctl->run(device->path());
}
