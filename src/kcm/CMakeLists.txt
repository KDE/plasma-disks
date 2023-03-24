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

kcoreaddons_add_plugin(plasma_disks_kcm SOURCES ${kcm_SRCS} INSTALL_NAMESPACE "plasma/kcms/kinfocenter")
set_target_properties(plasma_disks_kcm PROPERTIES OUTPUT_NAME smart)

target_link_libraries(plasma_disks_kcm
    statickdedsmart
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::QuickAddons
    Qt6::DBus
    KF6::Solid)

kcoreaddons_desktop_to_json(plasma_disks_kcm smart.desktop)

kpackage_install_package(package plasma_disks kcms)
