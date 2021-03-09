// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#ifndef SMARTDATA_H
#define SMARTDATA_H

#include <QObject>
#include <QString>

#include "smartfailure.h"

class QJsonObject;
class QJsonDocument;

/** Models "smart_status" blobs */
class SMARTStatus
{
public:
    SMARTStatus(const QJsonObject &object);

    bool m_passed;
};

/** Models "smartctl" blobs */
class SMARTCtlData
{
public:
    SMARTCtlData(const QJsonObject &object);

    SMART::Failures failure() const;

private:
    int m_exitStatus = -1; // only 8 least significant are filled for posix reasons.
};

/** Models the entire json output document */
class SMARTData
{
public:
    SMARTData(const QJsonDocument &document);

    SMARTCtlData m_smartctl;
    SMARTStatus m_status;
    QString m_device;
};

#endif // SMARTDATA_H
