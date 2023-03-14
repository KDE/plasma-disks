# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

cmake_minimum_required(VERSION 3.16)

project(plasma-disks)
set(PROJECT_VERSION "5.27.3")

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.102.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CMakeDependentOption)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(KDEClangFormat)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui Core)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    ConfigWidgets
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

cmake_dependent_option(WITH_SIMULATION
    "Build with simulation tech allowing easy testing via PLASMA_DISKS_SIMULATION=1"
    ON
    "CMAKE_BUILD_TYPE MATCHES [Dd]ebug"
    OFF
)
ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

ki18n_install(po)

install(FILES org.kde.plasma.disks.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
