cmake_minimum_required(VERSION 3.16)

project(AnOpenRGBWallpaperPlugin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt moc, uic, and rcc automatic compilation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt5 or Qt6. OpenRGB can be compiled with either.
# Note: We omit Network module to avoid dynamic DLL loader version mismatch crashes.
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets REQUIRED)

# Source files
set(SOURCES
    OpenRGBWallpaperEnginePlugin.cpp
    OpenRGBWallpaperEngineWidget.cpp
    RGBController.cpp
    StringUtils.cpp
    OpenRGBWallpaperEnginePlugin.qrc
    OpenRGBWallpaperEngineWidget.ui
)

# Header files
set(HEADERS
    OpenRGBWallpaperEnginePlugin.h
    OpenRGBWallpaperEngineWidget.h
    OpenRGBPluginInterface.h
    ResourceManagerInterface.h
    RGBController.h
    RGBControllerInterface.h
    LogManager.h
    StringUtils.h
    filesystem.h
    i2c_smbus.h
)

# Build the plugin as a shared module (.dll or .so)
add_library(AnOpenRGBWallpaperPlugin MODULE
    ${SOURCES}
    ${HEADERS}
)

# Include directories
target_include_directories(AnOpenRGBWallpaperPlugin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link against Qt libraries
target_link_libraries(AnOpenRGBWallpaperPlugin PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Link against Winsock library on Windows
if(WIN32)
    target_link_libraries(AnOpenRGBWallpaperPlugin PRIVATE ws2_32)
endif()

# Target options
target_compile_definitions(AnOpenRGBWallpaperPlugin PRIVATE
    QT_NO_keywords
)

# Configure build output directory matching OpenRGB's setup
set_target_properties(AnOpenRGBWallpaperPlugin PROPERTIES
    PREFIX ""
    SUFFIX ${CMAKE_SHARED_MODULE_SUFFIX}
)
