// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#pragma once


#include "org.kde.kded.smart.Device.h"

#include "dbusobjectmanagerserver.h"

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#warning should mabye have this generated but itd need type hints in the xml anyway
/*
 * Proxy class for interface org.freedesktop.DBus.ObjectManager
 */
class OrgFreedesktopDBusObjectManagerInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.DBus.ObjectManager"; }

public:
    OrgFreedesktopDBusObjectManagerInterface(const QString &service,
                                             const QString &path,
                                             const QDBusConnection &connection,
                                             QObject *parent = nullptr)
        : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
    {
#warning magic
        KDBusObjectManagerServer::registerTypes();
    }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<KDBusObjectManagerObjectPathInterfacePropertiesMap> GetManagedObjects()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetManagedObjects"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void InterfacesAdded(const QDBusObjectPath &object_path, const KDBusObjectManagerInterfacePropertiesMap &interfaces_and_properties);
    void InterfacesRemoved(const QDBusObjectPath &object_path, const KDBusObjectManagerInterfaceList &interfaces);
};


class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ItemRole {
        ObjectRole = Qt::UserRole + 1
    };

    QVector<QObject*> m_objects;

    DeviceModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const final;

    int rowCount(const QModelIndex &parent = QModelIndex()) const final;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_SCRIPTABLE int role(const QByteArray &roleName) const;

private Q_SLOTS:
    void propertyChanged();

    void addObject(const QDBusObjectPath &dbusPath, const KDBusObjectManagerInterfacePropertiesMap &interfacePropertyMap);
    void removeObject(const QDBusObjectPath &dbusPath);
    void reset();
    void reload();

private:
    void initRoleNames(QObject *object);

    QMetaMethod propertyChangedMetaMethod() const;

    QHash<int, QByteArray> m_roles;
    QHash<int, QByteArray> m_objectPoperties;
    QHash<int, int> m_signalIndexToProperties;

    OrgFreedesktopDBusObjectManagerInterface *m_iface = nullptr;
};

Q_DECLARE_METATYPE(DeviceModel*)
