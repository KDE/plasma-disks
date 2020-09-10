// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "module.h"

#include <KAboutData>
#include <KLocalizedString>

#include "devicemodel.h"
#include "servicerunner.h"
#include "version.h"

//--------------------------------------------------
#include <QObject>
#include <QDBusConnection>
#include <QDBusMessage>
class StatusBlob : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

public Q_SLOTS:
    bool smbd()
    {
        auto msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.systemd1"),
                                                  QStringLiteral("/org/freedesktop/systemd1/unit/smbd_2eservice"),
                                                  QStringLiteral("org.freedesktop.DBus.Properties.Get"),
                                                  QStringLiteral("org.freedesktop.systemd1.Unit"));
        msg << QStringLiteral("ActiveState");
        auto reply = QDBusConnection::systemBus().call(msg);
        if (reply.type() == QDBusMessage::ErrorMessage) {
            return false;
        }
        qDebug() << reply.arguments();
        // if we have a service returned then it must have found it
        return !reply.arguments().isEmpty();
    }

    }
};
//--------------------------------------------------

Module::Module(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
{

    auto aboutData = new KAboutData(QStringLiteral("plasma_disks"),
                                    i18nc("@title", "Storage Device Health Monitoring"),
                                    QString::fromLatin1(global_s_versionStringFull),
                                    QString(),
                                    KAboutLicense::LicenseKey::GPL_V3,
                                    i18nc("@info:credit", "Copyright 2020 Harald Sitter"));
    setAboutData(aboutData);
    // We have no help so remove the button from the buttons.
    setButtons(buttons() ^ Help ^ Default ^ Apply);

    qmlRegisterType<DeviceModel>("SMART", 1, 0, "DeviceModel");
    qmlRegisterType<ServiceRunner>("SMART", 1, 0, "ServiceRunner");
}

Module::~Module()
{
}

void Module::load()
{
}

void Module::save()
{
}

void Module::defaults()
{
}

#include "module.moc"
