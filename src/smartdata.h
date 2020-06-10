// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTDATA_H
#define SMARTDATA_H

#include <QString>

class QJsonObject;
class QJsonDocument;

class SMARTStatus
{
public:
    SMARTStatus(const QJsonObject &object);

    bool m_passed;
};

class SMARTData
{
public:
    SMARTData(const QJsonDocument &document);

    SMARTStatus m_status;
    QString m_device;
};

#endif // SMARTDATA_H
