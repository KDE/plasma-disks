cmake_minimum_required(VERSION 3.6)

project(plasma-smart VERSION 0.0)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt5 REQUIRED COMPONENTS Gui Core Test)
find_package(KF5CoreAddons REQUIRED)
find_package(KF5DBusAddons REQUIRED)
find_package(KF5Notifications REQUIRED)
find_package(KF5I18n REQUIRED)
find_package(KF5Solid REQUIRED)
find_package(KF5Service REQUIRED)
find_package(KF5KIO REQUIRED)
find_package(KF5Auth REQUIRED)

find_package(smartctl)
set_package_properties(smartctl PROPERTIES TYPE RUNTIME)

add_subdirectory(src)

if(BUILD_TESTING)
   add_subdirectory(autotests)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
