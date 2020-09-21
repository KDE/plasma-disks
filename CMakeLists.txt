# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

cmake_minimum_required(VERSION 3.16)

project(plasma-disks)
set(PROJECT_VERSION "5.20.80")

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.74")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui Core)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DBusAddons
    Notifications
    I18n
    Solid
    Service
    KIO
    Auth
    Declarative
)

find_package(smartctl)
set_package_properties(smartctl PROPERTIES TYPE RUNTIME)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()

install(FILES org.kde.plasma.disks.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
