// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

class SMARTMonitor;
class Device;

class SMARTNotifier : public QObject
{
    Q_OBJECT
public:
    SMARTNotifier(SMARTMonitor *monitor, QObject *parent = nullptr);
};
