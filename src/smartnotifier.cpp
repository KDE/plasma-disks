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
    // Don't use directly, go through makeFailure!
    FailureNotification(const Device *device, QObject *parent = nullptr)
        : QObject(parent)
    {
#warning todo what eventid to use
        m_notification->setContexts({{ QStringLiteral("device"), device->path() }});
#warning todo icon
        m_notification->setIconName(QStringLiteral("data-warning"));
        m_notification->setTitle(i18nc("@title notification", "Storage Device Problems"));
        m_notification->setText(i18nc("@info notification text",
                                    "The storage device <emphasis>%1</emphasis> (<filename>%2</filename>) is likely to fail soon!",
                                    device->product(), device->path()));

#warning action setup is super awkward not sure how to make it better perhaps absuing actions like this is a bad idea
        KService::Ptr partitionmanager = KService::serviceByDesktopName(QStringLiteral("org.kde.partitionmanager"));
        m_notification->setActions({i18nc("@action:button notification action", "Manage")});
        connect(m_notification, &KNotification::action1Activated,
                this, [partitionmanager] { KIO::ApplicationLauncherJob(partitionmanager).start(); });
        KService::Ptr kup = KService::serviceByDesktopName(QStringLiteral("kcm_kup"));
        qDebug() << kup->isValid() << kup->desktopEntryName();
        if (kup->isValid()) {
            // run through systemsettings directly
            kup->setExec(QStringLiteral("systemsettings5 ") + kup->desktopEntryName());
            m_notification->setActions({i18nc("@action:button notification action", "Manage"),
                                      i18nc("@action:button notification action", "Backup")});
            connect(m_notification, &KNotification::action2Activated,
                    this, [kup] { KIO::ApplicationLauncherJob(kup).start(); });
        }

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
    connect(monitor, &SMARTMonitor::failure,
            this, &SMARTNotifier::fail);
}

void SMARTNotifier::fail(const Device *device)
{
    new FailureNotification(device, this); // auto-delets
}

#include "smartnotifier.moc"
