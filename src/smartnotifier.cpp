// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartnotifier.h"

#include <KLocalizedString>
#include <KNotification>
#include <KService>
#include <KIO/ApplicationLauncherJob>

#include <QDebug>

#include "smartmonitor.h"
#include "device.h"

class FailureNotification : public QObject
{
    Q_OBJECT
public:
    FailureNotification(const Device *device, QObject *parent = nullptr)
        : QObject(parent)
    {
#warning todo what eventid to use
        m_notification->setContexts({{ QStringLiteral("device"), device->path() }});
        m_notification->setIconName(QStringLiteral("data-warning"));
        m_notification->setTitle(i18nc("@title notification", "Storage Device Problems"));
        m_notification->setText(i18nc("@info notification; text %1 is a pretty product name; %2 the device path e.g. /dev/sda",
                                      "The storage device <emphasis>%1</emphasis> (<filename>%2</filename>) is likely to fail soon!",
                                      device->product(), device->path()));

#warning should we maybe prefix with kcm so as to not have such a generic desktop name
        KService::Ptr kcm = KService::serviceByStorageId(QStringLiteral("smart"));
        Q_ASSERT(kcm); // there's a bug or installation is broken; mustn't happen in production
        m_notification->setActions({i18nc("@action:button notification action to manage device problems",
                                    "Manage")});
        connect(m_notification, &KNotification::action1Activated,
                this, [kcm] { KIO::ApplicationLauncherJob(kcm).start(); });

        connect(m_notification, &KNotification::closed,
                this, [this] {
            deleteLater();
            m_notification = nullptr;
        });

        m_notification->sendEvent();
    }

    ~FailureNotification() override
    {
        if (m_notification) {
            m_notification->close();
        }
    }

private:
#warning use persistent its disabled so i dont go mad when restarting kded
    KNotification *m_notification = new KNotification {
        QStringLiteral("notification"),
//        KNotification::Persistent | KNotification::DefaultEvent,
            KNotification::DefaultEvent,
        nullptr
    };
};

SMARTNotifier::SMARTNotifier(SMARTMonitor *monitor, QObject *parent)
    : QObject(parent)
{
    connect(monitor, &SMARTMonitor::deviceAdded,
            this, [this](const Device *device) {
        connect(device, &Device::failedChanged,
                this, [this, device] {
           if (device->failed()) {
               new FailureNotification(device, this); // auto-deletes
               // once displayed we'll not want to trigger any more notifications
               device->disconnect(this);
           }
        });
    });
    // upon removal the devices are deleted which takes care of disconnecting
}

#include "smartnotifier.moc"
