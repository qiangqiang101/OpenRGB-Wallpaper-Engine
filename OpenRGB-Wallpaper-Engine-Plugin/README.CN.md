# OpenRGB Wallpaper Engine 同步 C++ 插件

这是一个用于 **OpenRGB** 的 C++ 插件，可将设备的 RGB 灯效直接与 Wallpaper Engine 伴侣应用程序进行同步。

与外部脚本不同，该插件会编译为原生动态链接库（Windows 上为 `.dll`，Linux 上为 `.so`），并**直接在 OpenRGB 进程内部运行**。它会在 OpenRGB 用户界面中添加一个自定义的“壁纸设备（Wallpaper Device）”设置选项卡。

---

## 🛠️ 构建与编译

由于 OpenRGB 插件必须与你运行的 OpenRGB 版本的 Qt 版本和编译器架构相匹配，因此你必须从源码进行编译。

### 前置需求
* **CMake** (v3.16+)
* **Qt 5** 或 **Qt 6** 开发库（须与你的 OpenRGB 安装版本匹配）
* 支持 C++17 的 C++ 编译器（Windows 上推荐 MSVC，Linux 上推荐 GCC/Clang）

### 编译步骤

1. 在当前目录下打开终端或开发者命令提示符：
   ```bash
   cd OpenRGB-Wallpaper-Engine-Plugin
   ```
2. 创建并进入构建目录：
   ```bash
   mkdir build
   cd build
   ```
3. 生成构建配置：
   * **Qt6 版本：**
     ```bash
     cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2019_64" ..
     ```
   * **Qt5 版本：**
     ```bash
     cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.15.x/msvc2019_64" ..
     ```
4. 构建插件：
   ```bash
   cmake --build . --config Release
   ```

构建完成后，你将在 `build` 文件夹中找到 `AnOpenRGBWallpaperPlugin.dll`（Windows）或 `AnOpenRGBWallpaperPlugin.so`（Linux）。

---

## 🚀 安装与使用指南

1. 将编译好的动态库（`AnOpenRGBWallpaperPlugin.dll`/`.so`）复制到本地 OpenRGB 插件目录：
   * **Windows:** `%APPDATA%/OpenRGB/plugins/`
   * **Linux:** `~/.config/OpenRGB/plugins/`
2. 启动 **OpenRGB** 应用程序。
3. 打开 **设置 (Settings)** 选项卡并转到 **插件 (Plugins)**。你应该会看到列出的 **OpenRGB Wallpaper Engine & Lively Wallpaper** 插件。点击 **启用 (Enable)**。
4. 设置中将出现一个新的 **壁纸设备 (Wallpaper Devices)** 子选项卡。
5. 在此选项卡中：
   * 配置 Wallpaper Engine/Lively Wallpaper 的主机地址与 UDP 端口（默认为 `127.0.0.1:8133`）。
   * 配置矩阵设置（宽高比、网格密度、形状、模糊程度），使其与 Wallpaper Engine/Lively Wallpaper 的配置一致。
6. 点击 **保存设置 (Save Settings)** 保持配置，即可开始实时同步色彩。
