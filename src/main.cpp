// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include <QCoreApplication>
#include <KPluginFactory>
#include <KDEDModule>

#include "dbusobjectmanagerserver.h"
#include "device.h"
#include "smartmonitor.h"
#include "smartnotifier.h"

class SMARTModule : public KDEDModule
{
    Q_OBJECT
public:
    explicit SMARTModule(QObject *parent, const QVariantList &args)
        : KDEDModule(parent)
    {
        Q_UNUSED(args);
        connect(&m_monitor, &SMARTMonitor::deviceAdded,
                this, [this](Device *device) {
            dbusDeviceServer.serve(device);
        });
        connect(&m_monitor, &SMARTMonitor::deviceRemoved,
                &dbusDeviceServer, [this](Device *device) {
            dbusDeviceServer.unserve(device);
        });
        m_monitor.start();
    }

private:
    SMARTMonitor m_monitor { new SMARTCtl };
    SMARTNotifier m_notifier { &m_monitor };
    KDBusObjectManagerServer dbusDeviceServer;
};

K_PLUGIN_FACTORY_WITH_JSON(SMARTModuleFactory,
                           "smart.json",
                           registerPlugin<SMARTModule>();)

#include "main.moc"
