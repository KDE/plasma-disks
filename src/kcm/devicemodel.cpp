// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "devicemodel.h"

#include "org.freedesktop.DBus.Properties.h"

DeviceModel::DeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
#warning this bugger needs a servicewatcher in the event kded dies
#warning this isn't all too hot bc wed not get property notifications I rather think we need to ctor Device interfaces
    auto addObj = [this](const QDBusObjectPath &dbusPath, const KDBusObjectManagerInterfacePropertiesMap &interfacePropertyMap) {
        beginResetModel();

        // QDBus doesn't manage to map notfiable properties for its generated interface classes
        // so it brings literally nothing to the table for our Device class.
        // Use QObjects with dynamic properties instead to model the remote objects.
        // Property changes are abstracted via the the ListModel anyway.
        auto obj = new OrgFreedesktopDBusPropertiesInterface(
                    "org.kde.kded5",
                    dbusPath.path(),
                    QDBusConnection::sessionBus(),
                    this);
        m_objects << obj;
        obj->setObjectName(dbusPath.path());
        // Don't care about interfaces, iterate the values i.e. propertymap
        for (const auto &propertyMap : interfacePropertyMap) {
            for (auto propertyIt = propertyMap.cbegin(); propertyIt != propertyMap.cend(); ++propertyIt) {
                obj->setProperty(qPrintable(propertyIt.key()), propertyIt.value());
            }
        }

        connect(obj, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
                this, [this, obj](const QString &interface, const QVariantMap &properties, const QStringList &invalidated) {
#warning kinda code dupe from initial setting above
            qDebug() << interface << properties;
            for (auto it = properties.cbegin(); it != properties.cend(); ++it) {
                obj->setProperty(qPrintable(it.key()), it.value());

                // Technically we need an event filter to monitor dynamic prop changes,
                // but since this is the only place they can change we'll take a shortcut
                // and notify the views immediately.
                const int role = m_objectPoperties.key(it.key().toLatin1(), -1);
                Q_ASSERT(role != -1);
                const int index = m_objects.indexOf(obj);
                Q_ASSERT(index != -1);
                emit dataChanged(createIndex(index, 0), createIndex(index, 0), {role});
            }
        });

        if (m_roles.isEmpty()) {
            initRoleNames(obj);
        }

        endResetModel();
    };

    auto iface = new OrgFreedesktopDBusObjectManagerInterface(
                "org.kde.kded5",
                "/modules/smart/devices",
                QDBusConnection::sessionBus());
    connect(iface, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
            this, addObj);
    connect(iface, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this, [this](const QDBusObjectPath &dbusPath /* don't care about interface, there's only one */) {
        const QString path = dbusPath.path();
        auto it = std::find_if(m_objects.begin(), m_objects.end(), [path](const QObject *o) {
            return o->objectName() == path;
        });
        if (it == m_objects.end()) {
            return; // not tracked
        }
        auto index = std::distance(m_objects.begin(), it);
        beginRemoveRows(QModelIndex(), index, index);
        (*it)->deleteLater();
        m_objects.erase(it);
        endRemoveRows();
    });

    // Load existing objects.
    auto watcher = new QDBusPendingCallWatcher(iface->GetManagedObjects(), this);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, [watcher, addObj] {
        QDBusPendingReply<KDBusObjectManagerObjectPathInterfacePropertiesMap> call = *watcher;
        QList<QObject*> objects;
        auto map = call.value();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            addObj(it.key(), it.value());
        }
    });
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    qDebug() << "returning roles" << m_roles << QAbstractItemModel::roleNames();
    return m_roles;
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    qDebug() << "rowcount" << parent.isValid() << m_objects.count();
    if (parent.isValid()) {
        return 0;
    }
    return m_objects.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    qDebug() << role;
    if (!hasIndex(index.row(), index.column())) {
        qDebug() << "noindex";
        return QVariant();
    }
    QObject *obj = m_objects.at(index.row());
    if (role == ObjectRole) {
        return QVariant::fromValue(obj);
    }
    const QByteArray prop = m_objectPoperties.value(role);
    if (prop.isEmpty()) {
        qDebug() << "  no prop mapped" << role;
        return QVariant();
    }
    qDebug() << role << prop << obj->property(prop);
    return obj->property(prop);
}

int DeviceModel::role(const QByteArray &roleName) const
{
    qDebug() << roleName << m_roles.key(roleName, -1);
    return m_roles.key(roleName, -1);
}

void DeviceModel::propertyChanged()
{
    qDebug() << "prop changed";
    Q_UNREACHABLE();
    if (!sender() || senderSignalIndex() == -1) {
        return;
    }
    int role = m_signalIndexToProperties.value(senderSignalIndex(), -1);
    int index = m_objects.indexOf(sender());
    qDebug() << "PROPERTY CHANGED (" << index << ") :: " << role << roleNames().value(role);
    Q_EMIT dataChanged(createIndex(index, 0), createIndex(index, 0), {role});
}

void DeviceModel::initRoleNames(QObject *object)
{
    m_roles[ObjectRole] = QByteArrayLiteral("object");

    int maxEnumValue = ObjectRole;
    Q_ASSERT(maxEnumValue != -1);
    auto mo = *object->metaObject();
    for (int i = 0; i < mo.propertyCount(); ++i) {
        QMetaProperty property = mo.property(i);
        QString name(property.name());
        m_roles[++maxEnumValue] = name.toLatin1();
        m_objectPoperties.insert(maxEnumValue, property.name());
        if (!property.hasNotifySignal()) {
            continue;
        }
        m_signalIndexToProperties.insert(property.notifySignalIndex(), maxEnumValue);
    }
    for (const auto &dynProperty : object->dynamicPropertyNames()) {
        m_roles[++maxEnumValue] = dynProperty;
        m_objectPoperties.insert(maxEnumValue, dynProperty);
    }

    qDebug() << m_roles;
}

QMetaMethod DeviceModel::propertyChangedMetaMethod() const
{
    auto mo = metaObject();
    int methodIndex = mo->indexOfMethod("propertyChanged()");
    if (methodIndex == -1) {
        qFatal("Cannot find propertyChanged method. The model '%s' seems broken", mo->className());
        return QMetaMethod();
    }
    return mo->method(methodIndex);
}

#include "devicemodel.moc"
