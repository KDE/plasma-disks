# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

file(GLOB QML_SRCS package/contents/*.qml)
add_custom_target(QmlFiles ALL echo SOURCES ${QML_SRCS})

set(kcm_SRCS
    main.cpp
    module.cpp
    devicemodel.cpp
    servicerunner.cpp
)

# Something wonky is going on with the dbus macros. When the commands are set in the parent scope
# we'll not have access to them here as cmake claims
#   No rule to make target 'src/org.kde.smart.Device.xml'
# Instead generate the xmls (again) here...
qt5_generate_dbus_interface(../device.h org.kde.kded.smart.Device.xml)
qt5_add_dbus_interface(kcm_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kded.smart.Device.xml org.kde.kded.smart.Device)

qt5_add_dbus_interface(kcm_SRCS org.freedesktop.DBus.Properties.xml org.freedesktop.DBus.Properties)
set_source_files_properties(org.freedesktop.DBus.ObjectManager.xml
    PROPERTIES
        INCLUDE dbusobjectmanagerserver.h
)
qt5_add_dbus_interface(kcm_SRCS org.freedesktop.DBus.ObjectManager.xml org.freedesktop.DBus.ObjectManager)

add_library(kcm MODULE ${kcm_SRCS})
set_target_properties(kcm PROPERTIES
    OUTPUT_NAME smart
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/kcms
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/kcms
)

target_link_libraries(kcm
    statickdedsmart
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::QuickAddons
    Qt::DBus
    KF5::Solid)

kcoreaddons_desktop_to_json(kcm smart.desktop)

install(FILES smart.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm DESTINATION ${PLUGIN_INSTALL_DIR}/kcms)
kpackage_install_package(package plasma_disks kcms)
