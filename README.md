# OpenRGB Wallpaper Engine 2
![Github All Releases](https://img.shields.io/github/downloads/qiangqiang101/OpenRGB-Wallpaper-Engine/total.svg)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/qiangqiang101/OpenRGB-Wallpaper-Engine)
![GitHub](https://img.shields.io/github/license/qiangqiang101/OpenRGB-Wallpaper-Engine)
![GitHub branch checks state](https://img.shields.io/github/checks-status/qiangqiang101/OpenRGB-Wallpaper-Engine/master)
![GitHub issues](https://img.shields.io/github/issues/qiangqiang101/OpenRGB-Wallpaper-Engine)
![GitHub forks](https://img.shields.io/github/forks/qiangqiang101/OpenRGB-Wallpaper-Engine?style=social)
![GitHub Repo stars](https://img.shields.io/github/stars/qiangqiang101/OpenRGB-Wallpaper-Engine?style=social)
![YouTube Channel Subscribers](https://img.shields.io/youtube/channel/subscribers/UCAZlasvEy1euunP1M7nwj5Q?style=social)
[![Donate via PayPal](https://img.shields.io/badge/Donate-Paypal-brightgreen)](https://paypal.me/imnotmental)
[![Follow on Patreon](https://img.shields.io/badge/Donate-Patreon-orange)](https://www.patreon.com/imnotmental)

Turn your wallpaper into an OpenRGB device. You can now control the RGB lighting on your wallpaper and synchronize wallpaper with other OpenRGB compatible devices by [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB).

<img width="904" height="711" alt="QQ_1783458311061" src="https://github.com/user-attachments/assets/b04a8d31-04e3-4a0a-ac78-678c364eedd2" />
<img width="904" height="711" alt="QQ_1783458318865" src="https://github.com/user-attachments/assets/a008beb0-08aa-4f8b-a4da-b28a5c00792c" />

# 📸 Showcase
![IMG_6915](https://user-images.githubusercontent.com/11488961/202174752-3ecf4780-be04-40de-9382-d5ad14732104.JPG)
![IMG_6916](https://user-images.githubusercontent.com/11488961/202174762-a31ea030-35ec-47d6-a1b7-d8cee2229893.JPG)
![IMG_6917](https://user-images.githubusercontent.com/11488961/202174765-5b2bbdfc-581e-4bf2-ab65-979c0533dd4b.JPG)
![IMG_6919](https://user-images.githubusercontent.com/11488961/202174769-6164a88d-039d-4922-a501-51649a4a2da6.JPG)

# 🛠️ Download
## Wallpaper
[Wallpaper Engine](wpe.md) | [Lively Wallpaper](lively.md)

## Plugin
### OpenRGB 0.9+ Pipeline/Next (Plugin API Version 5)
- [Download Github](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.1.0.1/OpenRGBWallpaperPlugin_api5_win64.zip)
- [Download GitLab](https://gitlab.com/-/project/84234272/uploads/b885f23a6b4574c39a60a6f01bde33e0/OpenRGBWallpaperPlugin_api5_win64.zip)

### OpenRGB 1.0 RC - 1.0 RC3 (Plugin API Version 4)
- [Download Github](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.1.0.1/OpenRGBWallpaperPlugin_api4_win64.zip)
- [Download GitLab](https://gitlab.com/-/project/84234272/uploads/57079f30de92122f1e62a03c0fa07811/OpenRGBWallpaperPlugin_win64.zip)

# 🚧 Prerequisite
- [OpenRGB by CalcProgrammer1](https://gitlab.com/CalcProgrammer1/OpenRGB)
- [.NET 10.0 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/10.0)
- [Wallpaper Engine](https://store.steampowered.com/app/431960/Wallpaper_Engine/) or [Lively Wallpaper](https://github.com/rocksdanister/lively/releases/latest)
- Wallpaper Engine requires Windows 10/11 version 24H2 and above, Lively Wallpaper requires Windows 10/11 version 23H2 and below

# 🔨 Build yourself
## Wallpaper
1. Open the OpenRGBWallpaperEngine2.slnx file with Visual Studio 2026
2. Click the Build on menu, select Build Solution.

## Plugin
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

# 🚀 Similar Projects
- [OpenRGB Wallpaper](https://github.com/qiangqiang101/OpenRGB-Wallpaper)
- [SignalRGB Wallpaper Engine](https://github.com/qiangqiang101/SignalRGB-Wallpaper-Engine) 
