// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTMONITOR_H
#define SMARTMONITOR_H

#include <QObject>
#include <QTimer>

#include <functional>
#include <memory>

#include "smartctl.h"

class Device;

class SMARTMonitor : public QObject
{
    Q_OBJECT
    friend class SMARTMonitorTest;
public:
    explicit SMARTMonitor(AbstractSMARTCtl *ctl,
                          QObject *parent = nullptr);
    void start();

    QVector<Device *> devices() const;

signals:
    void deviceAdded(Device *device);
    void deviceRemoved(Device *device);

private slots:
    void checkUDI(const QString &udi);
    void reloadData();

private:
    void checkDevice(Device *device);

    QTimer m_reloadTimer;
    std::unique_ptr<AbstractSMARTCtl> m_ctl;
    QVector<Device *> m_devices;
};

#endif // SMARTMONITOR_H
