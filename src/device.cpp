// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "device.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/Block>

Device::Device(const QString &udi_, const QString &product_, const QString &path_, QObject *parent)
    : QObject(parent)
    , m_udi(udi_)
    , m_product(product_)
    , m_path(path_)
    , m_ignored(KSharedConfig::openConfig("org.kde.kded.smart")
                ->group("Ignores")
                .readEntry(udi_, false))
{
#warning product alone cannot provide a sufficiently hot prettys string see my usb3 sticky
#warning we need a reliable way to make udis safe to use here dbus is very limited in what it will allow for paths
    QString name = m_udi;
    setObjectName(name.remove(0, 1).replace('/', '_'));
}

Device::Device(const Solid::Device &solidDevice, QObject *parent)
    : Device(solidDevice.udi(),
             solidDevice.product(),
             solidDevice.as<Solid::Block>()->device(),
             parent)
{
}

bool Device::operator==(const Device &other) const
{
    return  m_udi == other.m_udi;
}

bool Device::failed() const
{
    return m_failed;
}

void Device::setFailed(bool failed)
{
    if (m_failed == failed) {
        return;
    }
    m_failed = failed;
    emit failedChanged();
}

bool Device::ignore() const
{
    return m_ignored;
}

void Device::setIgnore(bool ignored)
{
    if (m_ignored == ignored) {
        return;
    }
    KSharedConfig::openConfig("org.kde.kded.smart")
            ->group("Ignores")
            .writeEntry(m_udi, ignored);
    m_ignored = ignored;
    emit ignoreChanged();
}
