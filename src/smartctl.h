// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTCTL_H
#define SMARTCTL_H

#include <QJsonDocument>

class AbstractSMARTCtl
{
public:
    virtual ~AbstractSMARTCtl() = default;
    virtual QJsonDocument run(const QString &devicePath) const = 0;

    AbstractSMARTCtl() = default;
    Q_DISABLE_COPY(AbstractSMARTCtl)
};

class SMARTCtl : public AbstractSMARTCtl
{
public:
    QJsonDocument run(const QString &devicePath) const override;
};

#endif // SMARTCTL_H
