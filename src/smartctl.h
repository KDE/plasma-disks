// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTCTL_H
#define SMARTCTL_H

#include <QJsonDocument>
#include <QObject>

#include <queue>

class AbstractSMARTCtl : public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractSMARTCtl() = default;
    virtual void run(const QString &devicePath) = 0;

signals:
    void finished(const QString &devicePath, const QJsonDocument &document) const;

protected:
    AbstractSMARTCtl() = default;

private:
    Q_DISABLE_COPY(AbstractSMARTCtl)
};

class SMARTCtl : public AbstractSMARTCtl
{
public:
    void run(const QString &devicePath) override;

private:
    bool m_busy = false;
    std::queue<QString> m_requestQueue;
};

#endif // SMARTCTL_H
