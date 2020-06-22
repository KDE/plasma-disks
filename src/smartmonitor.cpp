// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartmonitor.h"

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/Block>
#include <Solid/StorageDrive>

#include <QDebug>

#include "device.h"
#include "smartctl.h"
#include "smartdata.h"

SMARTMonitor::SMARTMonitor(AbstractSMARTCtl *ctl, QObject *parent)
    : QObject(parent)
    , m_ctl(ctl)
{
    m_reloadTimer.setInterval(1000 * 60 /*minute*/ * 60 /*hour*/ * 24 /*day*/);
}

void SMARTMonitor::start()
{
    qDebug() << "starting";
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
            this, &SMARTMonitor::checkUDI);
    QMetaObject::invokeMethod(this, &SMARTMonitor::reloadData);
}

QVector<Device *> SMARTMonitor::devices() const
{
    return m_devices;
}

void SMARTMonitor::checkUDI(const QString &udi)
{
    Solid::Device dev(udi);
    if (!dev.is<Solid::Block>()) {
        return; // uninteresting device!
    }
    checkDevice(new Device(dev));
}

void SMARTMonitor::reloadData()
{
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : devices) {
        checkDevice(new Device(device));
    }
    m_reloadTimer.start();
}

void SMARTMonitor::checkDevice(Device *device)
{
    const QJsonDocument document = m_ctl->run(device->path());
    if (document.isEmpty()) {
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

#include "smartmonitor.moc"
