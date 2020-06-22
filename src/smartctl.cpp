// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartctl.h"

#include <QDebug>
#include <KAuthAction>
#include <KAuthExecuteJob>

QJsonDocument SMARTCtl::run(const QString &devicePath) const
{
    KAuth::Action action(QStringLiteral("org.kde.kded.smart.smartctl"));
    action.setHelperId(QStringLiteral("org.kde.kded.smart"));
    action.addArgument(QStringLiteral("devicePath"), devicePath);
    qDebug() << action.isValid()
             << action.hasHelper()
             << action.helperId()
             << action.status()
                ;
    KAuth::ExecuteJob *job = action.execute();
    job->exec();
    const auto data = job->data();
#warning code handling isnt the hottest
    const auto code = data.value(QStringLiteral("exitCode"), QByteArray()).toInt();
    const auto json = data.value(QStringLiteral("data"), QByteArray()).toByteArray();
    if (json.isEmpty() || code & Failure::BadCmdLine || code & Failure::Internal || code & Failure::DeviceOpen) {
        qDebug() << "looks like we got no data back for" << devicePath << code;
        return QJsonDocument();
    }
    return QJsonDocument::fromJson(json);
}
