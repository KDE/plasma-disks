// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>

namespace Solid {
class Device;
}

class Device : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kded.smart.Device")
    Q_PROPERTY(QString udi READ udi CONSTANT)
    Q_PROPERTY(QString product READ product CONSTANT)
    Q_PROPERTY(QString path READ path CONSTANT)
    // We dbus-expose objects without adaptor so the property API reflects the dbus API
    // and so we don't describe a write as this oughtn't be writable on dbus!
#warning setFailed for testing
    Q_PROPERTY(bool failed READ failed WRITE setFailed NOTIFY failedChanged)
public:
    Device(const QString &udi_, const QString &product_, const QString &path_, QObject *parent = nullptr);
    explicit Device(const Solid::Device &solidDevice, QObject *parent = nullptr);

    bool operator==(const Device &other) const;

    bool failed() const;
    void setFailed(bool failed);

    QString udi() const { return m_udi; }
    QString product() const { return m_product; }
    QString path() const { return m_path; }

signals:
    void failedChanged();

private:
    QString m_udi;
    QString m_product;
    QString m_path;
    bool m_failed = false;
};
