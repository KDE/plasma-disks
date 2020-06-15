# FIXME currently inherits the kded i18n domain; maybe split

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/version.h)

file(GLOB QML_SRCS package/contents/*.qml)
add_custom_target(QmlFiles ALL echo SOURCES ${QML_SRCS})

set(kcm_SRCS
    main.cpp
    module.cpp
)

add_library(kcm MODULE ${kcm_SRCS})
set_target_properties(kcm PROPERTIES OUTPUT_NAME smart)

target_link_libraries(kcm
    kded_smart
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::QuickAddons
    KF5::Solid)

kcoreaddons_desktop_to_json(kcm smart.desktop)
install(FILES smart.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(TARGETS kcm DESTINATION ${PLUGIN_INSTALL_DIR}/kcms)
kpackage_install_package(package smart kcms)
