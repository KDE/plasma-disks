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
#include <KNotification>
#include <KService>
#include <KIO/ApplicationLauncherJob>

#include <KLocalizedString>
#include <QDebug>

class FailureNotification : public QObject
{
    Q_OBJECT
public:
    // Don't use directly, go through makeFailure!
    FailureNotification(const Device &device,
                        QObject *parent = nullptr)
        : QObject(parent)
    {
#warning todo what eventid to use
        m_notification->setContexts({{ QStringLiteral("device"), device.path }});
#warning todo icon
        m_notification->setIconName(QStringLiteral("data-warning"));
        m_notification->setTitle(i18nc("@title notification", "Storage Device Problems"));
        m_notification->setText(i18nc("@info notification text",
                                    "The storage device <emphasis>%1</emphasis> (<filename>%2</filename>) is likely to fail soon!",
                                    device.product, device.path));

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
    KNotification *m_notification = new KNotification {
        QStringLiteral("notification"),
        KNotification::Persistent | KNotification::DefaultEvent,
        nullptr
    };
};

SMARTMonitor::SMARTMonitor(AbstractSMARTCtl *ctl, const FailureFactory &failureFactory, QObject *parent)
    : QObject(parent)
    , m_ctl(ctl)
    , m_failureFactory(failureFactory)
{
    m_reloadTimer.setInterval(1000 * 60 /*minute*/ * 60 /*hour*/ * 24 /*day*/);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded,
            this, &SMARTMonitor::checkUDI);
    QMetaObject::invokeMethod(this, &SMARTMonitor::reloadData);
}

void SMARTMonitor::makeFailure(const Device &device, QObject *parent)
{
    if (m_failureFactory) {
        m_failureFactory(device, parent);
    } else {
        new FailureNotification(device, parent);
    }
}

void SMARTMonitor::checkUDI(const QString &udi)
{
    checkDevice(Solid::Device(udi));
}

void SMARTMonitor::reloadData()
{
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : devices) {
        checkDevice(device);
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
//    if (data.m_status.m_passed) {
//        return;
//    }

    if (m_notified.contains(device.udi)) { // already notified
        return;
    }

    m_notified << device.udi;
    makeFailure(device, this); // failures auto delete
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
