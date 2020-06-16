// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartmonitor.h"
#include "smartctl.h"
#include "smartdata.h"

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/Block>
#include <Solid/StorageDrive>

#include <QDebug>

SMARTMonitor::SMARTMonitor(AbstractSMARTCtl *ctl, QObject *parent)
    : QObject(parent)
    , m_ctl(ctl)
{
    m_reloadTimer.setInterval(1000 * 60 /*minute*/ * 60 /*hour*/ * 24 /*day*/);
}

void SMARTMonitor::start()
{
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
            this, &SMARTMonitor::checkUDI);
    QMetaObject::invokeMethod(this, &SMARTMonitor::reloadData);
}

void SMARTMonitor::checkUDI(const QString &udi)
{
    Solid::Device dev(udi);
    if (!dev.is<Solid::Block>()) {
        return; // uninteresting device!
    }
    checkDevice(Device(dev));
}

void SMARTMonitor::reloadData()
{
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : devices) {
        checkDevice(Device(device));
    }
    m_reloadTimer.start();
}

void SMARTMonitor::checkDevice(const Device &device)
{
    const QJsonDocument document = m_ctl->run(device.path);
    if (document.isEmpty()) {
        return;
    }
    SMARTData data(document);
    qDebug() << data.m_device << data.m_status.m_passed;
#warning testing
        if (data.m_status.m_passed) {
            return;
        }

    if (m_notified.contains(device.udi)) {
        return;
    }
    m_notified << device.udi;

    emit failure(device);
}

Device::Device(const QString &udi_, const QString &product_, const QString &path_)
    : udi(udi_)
    , product(product_)
    , path(path_)
{
}

Device::Device(const Solid::Device &solidDevice)
    : Device(solidDevice.udi(),
             solidDevice.product(),
             solidDevice.as<Solid::Block>()->device())
{
}

#include "smartmonitor.moc"
