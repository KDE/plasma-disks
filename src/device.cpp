// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "device.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <Solid/Device>
#include <Solid/DeviceInterface>
#include <Solid/Block>

#include <QRegularExpression>

Device::Device(const QString &udi_, const QString &product_, const QString &path_, QObject *parent)
    : QObject(parent)
    , m_udi(udi_)
    , m_product(product_)
    , m_path(path_)
    , m_ignored(KSharedConfig::openConfig("org.kde.kded.smart")
                ->group("Ignores")
                .readEntry(udi_, false))
{
    // A simple replace actually makes any UDI safe to use for dbus.
    // https://dbus.freedesktop.org/doc/dbus-specification.html#message-protocol-marshaling-object-path
    // > No element may be the empty string.
    // > Multiple '/' characters cannot occur in sequence.
    // > A trailing '/' character is not allowed unless the path is the root path (a single '/' character).
    // > Each element must only contain the ASCII characters "[A-Z][a-z][0-9]_"
    // Since our name is put into a pre-existing path we only need to concern ourselves
    // with the content constraint and by extension the character constraint covers all
    // others since our name musn't be a path either.
    static const QRegularExpression filterExpr(QStringLiteral("[^A-Za-z0-9_]"));
    setObjectName(m_udi.replace(filterExpr, QStringLiteral("_")));
    Q_ASSERT(!objectName().isEmpty()); // mustn't be empty!
}

Device::Device(const Solid::Device &solidDevice, QObject *parent)
    : Device(solidDevice.udi(),
             solidDevice.vendor().isEmpty() ? solidDevice.product()
                                            : QStringLiteral("%1 %2").arg(solidDevice.vendor(),
                                                                          solidDevice.product()),
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
