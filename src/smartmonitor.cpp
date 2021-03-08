// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include "smartmonitor.h"

#include <KLocalizedString>

#include "device.h"
#include "devicenotifier.h"
#include "kded_debug.h"
#include "smartctl.h"
#include "smartdata.h"

SMARTMonitor::SMARTMonitor(std::unique_ptr<AbstractSMARTCtl> ctl, std::unique_ptr<DeviceNotifier> deviceNotifier, QObject *parent)
    : QObject(parent)
    , m_ctl(std::move(ctl))
    , m_deviceNotifier(std::move(deviceNotifier))
{
    connect(&m_reloadTimer, &QTimer::timeout, this, &SMARTMonitor::reloadData);
    connect(m_ctl.get(), &AbstractSMARTCtl::finished, this, &SMARTMonitor::onSMARTCtlFinished);
    m_reloadTimer.setInterval(1000 * 60 /*minute*/ * 60 /*hour*/ * 24 /*day*/);
}

void SMARTMonitor::start()
{
    qCDebug(KDED) << "starting";
    connect(m_deviceNotifier.get(), &DeviceNotifier::addDevice, this, &SMARTMonitor::addDevice);
    connect(m_deviceNotifier.get(), &DeviceNotifier::removeUDI, this, &SMARTMonitor::removeUDI);
    QMetaObject::invokeMethod(m_deviceNotifier.get(), &DeviceNotifier::start, Qt::QueuedConnection); // async to ensure listeners are ready
}

QVector<Device *> SMARTMonitor::devices() const
{
    return m_devices;
}

void SMARTMonitor::removeUDI(const QString &udi)
{
    auto newEnd = std::remove_if(m_devices.begin(), m_devices.end(), [this, udi](Device *dev) {
        if (dev->udi() != udi) {
            return false;
        }

        emit deviceRemoved(dev);
        dev->deleteLater();
        return true;
    });
    m_devices.erase(newEnd, m_devices.end());
}

void SMARTMonitor::reloadData()
{
    m_deviceNotifier->loadData();
    m_reloadTimer.start();
}

void SMARTMonitor::onSMARTCtlFinished(const QString &devicePath, const QJsonDocument &document)
{
    auto pendingIt = m_pendingDevices.find(devicePath);
    if (pendingIt == m_pendingDevices.end()) {
        qCDebug(KDED) << "unexpected pending result for" << devicePath;
        return;
    }
    Device *device = *pendingIt;
    m_pendingDevices.erase(pendingIt);

    if (document.isEmpty()) { // failed to get data, ignore the device
        device->deleteLater();
        return;
    }

    SMARTData data(document);
    if (!devicePath.endsWith(QStringLiteral(".json"))) { // simulation data
        Q_ASSERT(devicePath == data.m_device);
    }

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

void SMARTMonitor::addDevice(Device *device)
{
    m_pendingDevices[device->path()] = device;
    m_ctl->run(device->path());
}

#include "smartmonitor.moc"
