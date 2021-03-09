# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"plasma_disks\")

set(kded_SRCS
    smartdata.cpp
    smartmonitor.cpp
    smartctl.cpp
    smartnotifier.cpp
    dbusobjectmanagerserver.cpp
    device.cpp
    devicenotifier.cpp
    soliddevicenotifier.cpp
    instabilities.cpp
    smartfailure.cpp
)

if(WITH_SIMULATION)
    list(APPEND kded_SRCS
        simulationdevicenotifier.cpp
        simulationctl.cpp
        ../autotests/fixtures/simulation.qrc)
    add_definitions(-DWITH_SIMULATION)
endif()

ecm_qt_declare_logging_category(
    kded_SRCS
    HEADER "kded_debug.h"
    IDENTIFIER "KDED"
    CATEGORY_NAME "org.kde.plasma.disks"
    DESCRIPTION "Plasma Disks"
    EXPORT PlasmaDisks
)

add_library(statickdedsmart STATIC ${kded_SRCS})
target_include_directories(statickdedsmart
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_link_libraries(statickdedsmart
    PUBLIC
    Qt::Core
    Qt::DBus
    KF5::Solid
    KF5::I18n
    KF5::Notifications
    KF5::CoreAddons
    KF5::DBusAddons
    KF5::Service
    KF5::KIOGui
    KF5::AuthCore
)

add_library(kded-smart MODULE main.cpp)
kcoreaddons_desktop_to_json(kded-smart smart.desktop)
set_target_properties(kded-smart PROPERTIES OUTPUT_NAME smart)
target_link_libraries(kded-smart statickdedsmart)
install(TARGETS kded-smart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)

kauth_install_actions(org.kde.kded.smart org.kde.kded.smart.actions)
add_executable(kded-smart-helper helper.cpp)
target_link_libraries(kded-smart-helper KF5::AuthCore KF5::ConfigCore KF5::I18n)

kauth_install_helper_files(kded-smart-helper org.kde.kded.smart root)
install(TARGETS kded-smart-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

install(FILES org.kde.kded.smart.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

add_subdirectory(kcm)
