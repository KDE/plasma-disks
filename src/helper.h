// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef HELPER_H
#define HELPER_H
#include <QObject>
#include <kauth_version.h>
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#else
#include <KAuth>
#endif

using namespace KAuth;

class SMARTHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    ActionReply smartctl(const QVariantMap &args);
};

#endif // HELPER_H
