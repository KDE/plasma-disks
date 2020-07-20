// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartctl.h"

#include <QDebug>
#include <KAuthAction>
#include <KAuthExecuteJob>
#include <KLocalizedString>

void SMARTCtl::run(const QString &devicePath) const
{
#warning an argument could be made that run should be async since it goes through dbus and that can take a while
    KAuth::Action action(QStringLiteral("org.kde.kded.smart.smartctl"));
    // This is technically never used unless the sysadmin forces our action
    // to require authentication. In that case we'll want to give request context
    // as we do requests per-device though.
    action.setDetailsV2({
                            { KAuth::Action::AuthDetail::DetailMessage,
                              i18nc("@label description of authentication request to read SMART data. %1 is a device path e.g. /dev/sda",
                              "Read SMART report for storage device %1",
                              devicePath) }
                        });
    action.setHelperId(QStringLiteral("org.kde.kded.smart"));
    action.addArgument(QStringLiteral("devicePath"), devicePath);
    qDebug() << action.isValid()
             << action.hasHelper()
             << action.helperId()
             << action.status()
                ;
    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result,
            this, [this, job, devicePath] {
        const auto data = job->data();
        const auto code = data.value(QStringLiteral("exitCode"), QByteArray()).toInt();
        const auto json = data.value(QStringLiteral("data"), QByteArray()).toByteArray();
        QJsonDocument document;
        if (json.isEmpty() || code & Failure::CmdLineParse || code & Failure::DeviceOpen) {
            qDebug() << "looks like we got no data back for" << devicePath << code << json.isEmpty();
        } else {
            document = QJsonDocument::fromJson(json);
        }
        emit finished(devicePath, document);
    });
    job->start();
}
