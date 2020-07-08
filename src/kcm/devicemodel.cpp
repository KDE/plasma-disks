// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "devicemodel.h"

#include "org.freedesktop.DBus.Properties.h"

#warning I feel like the dbus shebang needs putting into a seprate class it has little to do with the devicemodelling in general
DeviceModel::DeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
    auto watcher = new QDBusServiceWatcher("org.kde.kded5", QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForOwnerChange,
                                           this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged,
            this, [this](const QString &/*service*/, const QString &/*oldOwner*/, const QString &newOwner){
        if (!newOwner.isEmpty()) { // this is a registration even not a loss event
            reload();
        } else {
            reset();
        }
    });

    reload();
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    qDebug() << "returning roles" << m_roles << QAbstractItemModel::roleNames();
    return m_roles;
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent); // this is a flat list we decidedly don't care about the parent
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

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << "!!!!!" << role;
    if (!hasIndex(index.row(), index.column())) {
        qDebug() << "noindex";
        return false;
    }
    QObject *obj = m_objects.at(index.row());
    if (role == ObjectRole) {
        return false; // cannot set object!
    }
    const QByteArray prop = m_objectPoperties.value(role);
    if (prop.isEmpty()) {
        qDebug() << "  no prop mapped" << role;
        return false;
    }
    qDebug() << role << prop << obj->property(prop);
    return obj->setProperty(prop, value);
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

class RuntimePropertyChangeFilter : public QObject
{
    Q_OBJECT
public:
    RuntimePropertyChangeFilter(OrgFreedesktopDBusPropertiesInterface *parent)
        : QObject(parent)
        , m_dbusObject(parent)
    {
        qDebug() << "::::: WATCH";
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        qDebug() << "::::: FILTER!";
        if (event->type() == QEvent::DynamicPropertyChange) {
            auto change = static_cast<QDynamicPropertyChangeEvent *>(event);
            const auto name = change->propertyName();
            const auto value = m_dbusObject->property(name);
            qDebug() << "org.kde.kded.smart.Device" << name << value;
#warning we should kinda support interfaces properly we presently ignore them in addobject
            m_dbusObject->Set("org.kde.kded.smart.Device", name, QDBusVariant(value));
        }
        return QObject::eventFilter(obj, event);
    }

private:
    OrgFreedesktopDBusPropertiesInterface *m_dbusObject;
};

void DeviceModel::addObject(const QDBusObjectPath &dbusPath, const KDBusObjectManagerInterfacePropertiesMap &interfacePropertyMap)
{
    const QString path = dbusPath.path();

    int newIndex = 0;
    for (auto it = m_objects.cbegin(); it != m_objects.cend(); ++it) {
        if ((*it)->objectName() == path) {
            qDebug() << path << "already tracked";
            return; // already tracked
        }
        ++newIndex;
    }

    beginInsertRows(QModelIndex(), newIndex, newIndex);

    // QDBus doesn't manage to map notfiable properties for its generated interface classes
    // so it brings literally nothing to the table for our Device class.
    // Use QObjects with dynamic properties instead to model the remote objects.
    // Property changes are abstracted via the the ListModel anyway.
    auto obj = new OrgFreedesktopDBusPropertiesInterface(
                "org.kde.kded5",
                path,
                QDBusConnection::sessionBus(),
                this);
    m_objects << obj;
    obj->setObjectName(path);
    // Don't care about interfaces, iterate the values i.e. propertymap
    for (const auto &propertyMap : interfacePropertyMap) {
        for (auto propertyIt = propertyMap.cbegin(); propertyIt != propertyMap.cend(); ++propertyIt) {
            obj->setProperty(qPrintable(propertyIt.key()), propertyIt.value());
        }
    }
    obj->installEventFilter(new RuntimePropertyChangeFilter(obj));

    connect(obj, &OrgFreedesktopDBusPropertiesInterface::PropertiesChanged,
            this, [this, obj](const QString &/*interface*/,
                              const QVariantMap &properties,
                              const QStringList &/*invalidated*/) {
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

    endInsertRows();
}

void DeviceModel::removeObject(const QDBusObjectPath &dbusPath)
{
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

void DeviceModel::reset()
{
    qDebug() << "reset";
    beginResetModel();
    qDeleteAll(m_objects);
    m_objects.clear();
    if (m_iface) {
        m_iface->disconnect(this);
        m_iface->deleteLater();
        m_iface = nullptr;
    }
    qDebug() << "objects in" << m_objects.size();
    endResetModel();
}

void DeviceModel::reload()
{
    reset();

    m_iface = new OrgFreedesktopDBusObjectManagerInterface(
                "org.kde.kded5",
                "/modules/smart/devices",
                QDBusConnection::sessionBus(),
                this);
    connect(m_iface, &OrgFreedesktopDBusObjectManagerInterface::InterfacesAdded,
            this, &DeviceModel::addObject);
    connect(m_iface, &OrgFreedesktopDBusObjectManagerInterface::InterfacesRemoved,
            this, &DeviceModel::removeObject);

    // Load existing objects.
    if (m_getManagedObjectsWatcher) {
        // Last reload didn't finish before this one, so throw away the last watcher
        m_getManagedObjectsWatcher->deleteLater();
    }
    m_getManagedObjectsWatcher = new QDBusPendingCallWatcher(m_iface->GetManagedObjects(), this);
    connect(m_getManagedObjectsWatcher, &QDBusPendingCallWatcher::finished,
            this, [this] {
        QDBusPendingReply<KDBusObjectManagerObjectPathInterfacePropertiesMap> call = *m_getManagedObjectsWatcher;
        QList<QObject*> objects;
        auto map = call.value();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            addObject(it.key(), it.value());
        }
        m_getManagedObjectsWatcher->deleteLater();
        m_getManagedObjectsWatcher = nullptr;
    });
}

#include "devicemodel.moc"
