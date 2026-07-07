# OpenRGB Wallpaper Engine Sync C++ Plugin

This is a C++ plugin for **OpenRGB** that synchronizes your device lighting directly with the Wallpaper Engine companion application.

Unlike external scripts, this compiles into a native dynamic library (`.dll` on Windows or `.so` on Linux) and runs **directly inside the OpenRGB process**. It adds a custom "Wallpaper Device" settings tab to your OpenRGB user interface.

---

## 🛠️ Build and Compilation

Since OpenRGB plugins must match the Qt version and compiler architecture of the OpenRGB build you are running, you must compile the plugin from source.

### Prerequisites
* **CMake** (v3.16+)
* **Qt 5** or **Qt 6** Developer Libraries (matching your OpenRGB installation version)
* A C++ compiler supporting C++17 (MSVC on Windows, GCC/Clang on Linux)

### Compilation Steps

1. Open your terminal or developer command prompt in this directory:
   ```bash
   cd OpenRGB-Wallpaper-Engine-Plugin
   ```
2. Create and enter a build directory:
   ```bash
   mkdir build
   cd build
   ```
3. Generate the build configuration:
   * **For Qt6:**
     ```bash
     cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2019_64" ..
     ```
   * **For Qt5:**
     ```bash
     cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.15.x/msvc2019_64" ..
     ```
4. Build the plugin:
   ```bash
   cmake --build . --config Release
   ```

Upon completion, you will find `AnOpenRGBWallpaperPlugin.dll` (on Windows) or `AnOpenRGBWallpaperPlugin.so` (on Linux) in your build folder.

---

## 🚀 How to Install and Use

1. Copy the compiled library (`AnOpenRGBWallpaperPlugin.dll`/`.so`) into your local OpenRGB plugins directory:
   * **Windows:** `%APPDATA%/OpenRGB/plugins/`
   * **Linux:** `~/.config/OpenRGB/plugins/`
2. Start the **OpenRGB** application.
3. Open the **Settings** tab and go to **Plugins**. You should see the **OpenRGB Wallpaper Engine & Lively Wallpaper** plugin listed. Click **Enable**.
4. A new **Wallpaper Devices** sub-tab will appear in your settings.
5. In this tab:
   * Configure the host and UDP port of the Wallpaper Engine/Lively Wallpaper (defaults to `127.0.0.1:8133`).
   * Configure Matrix settings (Aspect Ratio, Grid Density, Shapes, Blur) to align with your Wallpaper Engine/Lively Wallpaper's configuration.
6. Click **Save Settings** to persist the configuration and start sync colors in real-time.
