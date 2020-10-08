// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "helper.h"

#include <QDebug>
#include <QProcess>
#include <QFileInfo>
#include <QScopeGuard>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

// Append name to /dev/ and ensure it is a trustable block device.
static QString nameToPath(const QString &name)
{
    if (name.isEmpty()) {
        return {};
    }

    // This also excludes relative path shenanigans as they'd all need to contain a separator.
    if (name.contains(QLatin1Char('/'))) {
        qWarning() << "Device names must not contain slashes";
        return {};
    }

    const QString path = QStringLiteral("/dev/%1").arg(name);

    int blockFD = open(QFile::encodeName(path), O_PATH | O_NOFOLLOW);
    auto blockFDClose = qScopeGuard([blockFD] { close(blockFD); });
    if (blockFD == -1) {
        const int err = errno;
        qWarning() << "Failed to open block device" << name << strerror(err);
        return {};
    }

    struct stat sb;
    if (fstat(blockFD, &sb) == -1) {
        const int err = errno;
        qWarning() << "Failed to stat block device" << name << strerror(err);
        return {};
    }

    if (!S_ISBLK(sb.st_mode)) {
        qWarning() << "Device is not actually a block device" << name;
        return {};
    }

    if (sb.st_uid != 0) {
        qWarning() << "Device is not owned by root" << name;
        return {};
    }

    return path;
}

ActionReply SMARTHelper::smartctl(const QVariantMap &args)
{
    // For security reasons we only accept fully resolved device names which
    // we use to construct the final /dev/$name path.
    const QString name = args.value(QStringLiteral("deviceName")).toString();
    const QString devicePath = nameToPath(name);
    if (devicePath.isEmpty()) {
        return ActionReply::HelperErrorReply();
    }

    // PATH is super minimal when invoked through dbus
    setenv("PATH", "/usr/sbin:/sbin", 1);
    QProcess p;
    // json=c is badly documented and means "gimme json but don't pretty print"
    p.start(QStringLiteral("smartctl"),
            { QStringLiteral("--all"), QStringLiteral("--json=c"), devicePath },
            QProcess::ReadOnly);
    p.waitForFinished();

    ActionReply reply;
    reply.addData(QStringLiteral("exitCode"), p.exitCode());
    reply.addData(QStringLiteral("data"), p.readAllStandardOutput());
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.kded.smart", SMARTHelper)
