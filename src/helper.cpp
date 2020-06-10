// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "helper.h"

#include <QDebug>
#include <QProcess>
#include <QFileInfo>

QString pathFrom(const QVariantMap &args)
{
    const auto devicePath = args.value(QStringLiteral("devicePath")).toString();
    QFileInfo info(devicePath);
    return info.absoluteFilePath();
}

ActionReply SMARTHelper::smartctl(const QVariantMap &args)
{
    qInstallMessageHandler(nullptr);
    // I may be better overall to also spin up solid on the root end and only allow
    // UDIs as input. We can then assert expected input. Not sure it makes much
    // of a difference though.
    const QString devicePath = pathFrom(args);
    if (devicePath.isEmpty() || !QFile::exists(devicePath)) {
        qDebug() << "bad path";
        return ActionReply::HelperErrorReply();
    }
    if (!devicePath.startsWith(QStringLiteral("/dev/"))) {
        qDebug() << "unauthorized path";
        return ActionReply::HelperErrorReply(KAuth::ActionReply::AuthorizationDeniedError);
    }

    // PATH is super minimal when invoked through dbus
    setenv("PATH", "/usr/sbin:/sbin", 1);
    QProcess p;
    // json=c is badly documented and means "gimme json but don't pretty print"
    p.start(QStringLiteral("smartctl"),
            { QStringLiteral("--all"), QStringLiteral("--json=c"), devicePath },
            QProcess::ReadOnly);
    p.waitForFinished();
    if (p.exitCode() != 0) {
        return ActionReply::HelperErrorReply();
    }

    ActionReply reply;
    reply.addData(QStringLiteral("data"), p.readAllStandardOutput());
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kded.smart", SMARTHelper)

#include "helper.moc"
