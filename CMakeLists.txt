# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

cmake_minimum_required(VERSION 3.16)

project(plasma-disks)
set(PROJECT_VERSION "5.22.4")

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.82")

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
if(ECM_VERSION VERSION_GREATER_EQUAL 5.79)
    include(KDEGitCommitHooks)
endif()

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

cmake_dependent_option(WITH_SIMULATION
    "Build with simulation tech allowing easy testing via PLASMA_DISKS_SIMULATION=1"
    ON
    "CMAKE_BUILD_TYPE MATCHES [Dd]ebug"
    OFF
)

add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ki18n_install(po)
endif()

install(FILES org.kde.plasma.disks.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
if(ECM_VERSION VERSION_GREATER_EQUAL 5.79)
    kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
