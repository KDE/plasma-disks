// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#ifndef SMARTMONITOR_H
#define SMARTMONITOR_H

#include <QObject>
#include <QTimer>

#include <functional>
#include <memory>

#include "smartctl.h"

namespace Solid {
class Device;
}

struct Device
{
    Device(const QString &udi_, const QString &product_, const QString &path_);
    Device(const Solid::Device &solidDevice);

    const QString udi;
    const QString product;
    const QString path;
};

class SMARTMonitor : public QObject
{
    Q_OBJECT
    friend class SMARTMonitorTest;
public:
    typedef std::function<void(const Device &device, QObject *parent)> FailureFactory;

    explicit SMARTMonitor(AbstractSMARTCtl *ctl,
                          const FailureFactory &failureFactory = nullptr,
                          QObject *parent = nullptr);

private slots:
    void checkUDI(const QString &udi);
    void reloadData();

private:
    void checkDevice(const Device &device);
    void makeFailure(const Device &device, QObject *parent);

    QTimer m_reloadTimer;
    QVector<QString> m_notified;
    std::unique_ptr<AbstractSMARTCtl> m_ctl;
    const FailureFactory &m_failureFactory = nullptr;
};

#endif // SMARTMONITOR_H
