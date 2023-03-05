# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

file(GLOB QML_SRCS package/contents/*.qml)
add_custom_target(QmlFiles ALL echo SOURCES ${QML_SRCS})

set(kcm_SRCS
    module.cpp module.h
    devicemodel.cpp devicemodel.h
    servicerunner.cpp servicerunner.h
)

# Something wonky is going on with the dbus macros. When the commands are set in the parent scope
# we'll not have access to them here as cmake claims
#   No rule to make target 'src/org.kde.smart.Device.xml'
# Instead generate the xmls (again) here...
qt_generate_dbus_interface(../device.h org.kde.kded.smart.Device.xml)
qt_add_dbus_interface(kcm_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded.smart.Device.xml org.kde.kded.smart.Device)

qt_add_dbus_interface(kcm_SRCS org.freedesktop.DBus.Properties.xml org.freedesktop.DBus.Properties)
set_source_files_properties(org.freedesktop.DBus.ObjectManager.xml
    PROPERTIES
        INCLUDE dbusobjectmanagerserver.h
)
qt_add_dbus_interface(kcm_SRCS org.freedesktop.DBus.ObjectManager.xml org.freedesktop.DBus.ObjectManager)

kcoreaddons_add_plugin(kcm_disks SOURCES ${kcm_SRCS} INSTALL_NAMESPACE "plasma/kcms/kinfocenter")
kcmutils_generate_desktop_file(kcm_disks)

target_link_libraries(kcm_disks
    statickdedsmart
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::QuickAddons
    Qt${QT_MAJOR_VERSION}::DBus
    KF5::Solid)

kpackage_install_package(package kcm_disks kcms)
