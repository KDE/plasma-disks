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
    auto data = job->data();
    const auto json = data.value(QStringLiteral("data"), QByteArray()).toByteArray();
    if (json.isEmpty()) {
        qDebug() << "looks like we got no data back for" << devicePath;
    }
    return QJsonDocument::fromJson(json);
}
