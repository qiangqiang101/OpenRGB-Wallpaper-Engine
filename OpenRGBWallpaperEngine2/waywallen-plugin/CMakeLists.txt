cmake_minimum_required(VERSION 3.20)
project(openrgb_waywallen_plugin LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(PNG REQUIRED libpng)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GBM REQUIRED gbm)
pkg_check_modules(GLESV2 REQUIRED glesv2)

add_executable(openrgb-waywallen-plugin
    src/main.cpp
    /tmp/waywallen/bridge/src/bridge.c
    /tmp/waywallen/bridge/src/drm_fourcc.c
    /tmp/waywallen/bridge/src/ipc_v1.c
    /tmp/waywallen/bridge/src/probe_egl.c
    /tmp/waywallen/bridge/src/probe_vk.c
    /tmp/waywallen/bridge/src/sync_release.c
    /tmp/waywallen/bridge/src/pool.c
    /tmp/waywallen/bridge/src/pool_egl_gbm.c
    /tmp/waywallen/bridge/src/pool_vulkan.c
)

target_include_directories(openrgb-waywallen-plugin PRIVATE ${SDL2_INCLUDE_DIRS} ${CAIRO_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} /tmp/waywallen/bridge/include)
target_compile_options(openrgb-waywallen-plugin PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(openrgb-waywallen-plugin PRIVATE ${SDL2_LIBRARIES} ${CAIRO_LIBRARIES} ${PNG_LIBRARIES} ${EGL_LIBRARIES} ${GBM_LIBRARIES} ${GLESV2_LIBRARIES} pthread)

install(TARGETS openrgb-waywallen-plugin RUNTIME DESTINATION bin)

set(PLUGIN_PACKAGE_DIR "${CMAKE_BINARY_DIR}/plugin-package")
set(PLUGIN_ZIP_PATH "${CMAKE_BINARY_DIR}/org.openrgb.waywallen.zip")
set(PLUGIN_PAYLOAD_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin.toml"
    "${CMAKE_CURRENT_SOURCE_DIR}/main.lua"
    "${CMAKE_CURRENT_SOURCE_DIR}/files.txt"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/package-plugin.cmake"
)

add_custom_command(
    OUTPUT "${PLUGIN_ZIP_PATH}"
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${PLUGIN_PACKAGE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PLUGIN_PACKAGE_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:openrgb-waywallen-plugin> "${PLUGIN_PACKAGE_DIR}/bin/openrgb-waywallen-plugin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/plugin.toml" "${PLUGIN_PACKAGE_DIR}/plugin.toml"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/main.lua" "${PLUGIN_PACKAGE_DIR}/main.lua"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/files.txt" "${PLUGIN_PACKAGE_DIR}/files.txt"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/README.md" "${PLUGIN_PACKAGE_DIR}/README.md"
    COMMAND ${CMAKE_COMMAND} -DPLUGIN_PACKAGE_DIR=${PLUGIN_PACKAGE_DIR} -DPLUGIN_ZIP_PATH=${PLUGIN_ZIP_PATH} -P "${CMAKE_CURRENT_SOURCE_DIR}/package-plugin.cmake"
    DEPENDS openrgb-waywallen-plugin ${PLUGIN_PAYLOAD_FILES}
    COMMENT "Packaging OpenRGB waywallen plugin into zip"
)

add_custom_target(package-plugin ALL DEPENDS "${PLUGIN_ZIP_PATH}")
