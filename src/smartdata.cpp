// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartdata.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

SMARTStatus::SMARTStatus(const QJsonObject &object)
    : m_passed(object[QStringLiteral("passed")].toBool())
{
    // Should we decide to map the value. Its meaning is "defined" in
    // nvmeprint.cpp of smartmontools
}

SMARTData::SMARTData(const QJsonDocument &document)
    : m_status(SMARTStatus(document.object()[QStringLiteral("smart_status")].toObject()))
    , m_device(document.object()[QStringLiteral("device")].toObject()[QStringLiteral("name")].toString())
{
}
