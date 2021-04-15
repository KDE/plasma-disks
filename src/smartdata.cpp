// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include "smartdata.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "kded_debug.h"

SMARTStatus::SMARTStatus(const QJsonObject &object)
    : m_passed(object[QStringLiteral("passed")].toBool())
{
    // Should we decide to map the value. Its meaning is "defined" in
    // nvmeprint.cpp of smartmontools
}
SMARTCtlData::SMARTCtlData(const QJsonObject &object)
    : m_exitStatus(object[QStringLiteral("exit_status")].toInt(static_cast<int>(SMART::Failure::None)))
{
}

SMART::Failures SMARTCtlData::failure() const
{
    return {static_cast<SMART::Failure>(m_exitStatus)};
}

SMARTData::SMARTData(const QJsonDocument &document)
    : m_smartctl(SMARTCtlData(document.object()[QStringLiteral("smartctl")].toObject()))
    , m_status(SMARTStatus(document.object()[QStringLiteral("smart_status")].toObject()))
    , m_device(document.object()[QStringLiteral("device")].toObject()[QStringLiteral("name")].toString())
    , m_valid(checkValid(document))
{
}

bool SMARTData::checkValid(const QJsonDocument &document) const
{
    if (m_smartctl.failure() & SMART::Failure::CmdLineParse) {
        qCDebug(KDED) << "Command line error" << m_device << document.toJson();
        return false;
    }
    if (m_smartctl.failure() & SMART::Failure::DeviceOpen) {
        qCDebug(KDED) << "Failed to open device" << m_device << document.toJson();
        return false;
    }
    const bool hasSMARTStatus = document.object().contains(QStringLiteral("smart_status"));
    const bool internalCommandFailure = (m_smartctl.failure() & SMART::Failure::InternalCommand);
    if (!hasSMARTStatus && internalCommandFailure) {
        // VirtualBox drives return with InternalCommand problems and no SMART data. Consider the data invalid.
        // If we also have other failure codes we'll still want to consider the data valid as it might indicate
        // problems.
        qCDebug(KDED) << "Internal command problems resulted in no smart_status data" << m_device << document.toJson();
        return false;
    }
    return true;
}
