# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"kded_smart\")

set(kded_SRCS
    smartdata.cpp
    smartmonitor.cpp
    smartctl.cpp
    smartnotifier.cpp
    dbusobjectmanagerserver.cpp
    device.cpp
)

add_library(statickdedsmart STATIC ${kded_SRCS})
target_include_directories(statickdedsmart
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
target_link_libraries(statickdedsmart
    PUBLIC
    Qt5::Core
    Qt5::DBus
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
set_target_properties(kded-smart PROPERTIES OUTPUT_NAME smart)
kcoreaddons_desktop_to_json(kded-smart smart.desktop)
target_link_libraries(kded-smart
    statickdedsmart
)

install(TARGETS kded-smart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf5/kded)

kauth_install_actions(org.kde.kded.smart org.kde.kded.smart.actions)
add_executable(kded-smart-helper helper.cpp)
target_link_libraries(kded-smart-helper KF5::AuthCore KF5::ConfigCore KF5::I18n)

kauth_install_helper_files(kded-smart-helper org.kde.kded.smart root)
install(TARGETS kded-smart-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

install(FILES org.kde.kded.smart.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

add_subdirectory(kcm)
