// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include <QCoreApplication>
#include <KPluginFactory>
#include <KDEDModule>

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
    }

private:
    SMARTMonitor m_monitor { new SMARTCtl };
    SMARTNotifier m_notifier { &m_monitor };
};

K_PLUGIN_FACTORY_WITH_JSON(SMARTModuleFactory,
                           "smart.json",
                           registerPlugin<SMARTModule>();)

#include <unistd.h>
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("plasma-smart"));
    SMARTModule m(nullptr, {});
    sleep(2);
    app.exec();
}

#include "main.moc"
