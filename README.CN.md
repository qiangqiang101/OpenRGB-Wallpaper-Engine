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

[English README](README.md)

将你的壁纸变成一个 OpenRGB 设备。现在你可以通过 [OpenRGB](https://gitlab.com/CalcProgrammer1/OpenRGB) 控制壁纸上的 RGB 灯效，并将其与其它支持 OpenRGB 的设备同步。

<img width="904" height="711" alt="QQ_1783458311061" src="https://github.com/user-attachments/assets/b04a8d31-04e3-4a0a-ac78-678c364eedd2" />
<img width="904" height="711" alt="QQ_1783458318865" src="https://github.com/user-attachments/assets/a008beb0-08aa-4f8b-a4da-b28a5c00792c" />

# 📸 展示
<table>
   <tr>
      <td><img src="https://user-images.githubusercontent.com/11488961/202174752-3ecf4780-be04-40de-9382-d5ad14732104.JPG" /></td>
      <td><img src="https://user-images.githubusercontent.com/11488961/202174762-a31ea030-35ec-47d6-a1b7-d8cee2229893.JPG" /></td>
   </tr>
   <tr>
      <td><img src="https://user-images.githubusercontent.com/11488961/202174765-5b2bbdfc-581e-4bf2-ab65-979c0533dd4b.JPG" /></td>
      <td><img src="https://user-images.githubusercontent.com/11488961/202174769-6164a88d-039d-4922-a501-51649a4a2da6.JPG" /></td>
   </tr>
</table>

# 🛠️ 下载
### 壁纸插件
## OpenRGB 0.9+ Pipeline/Next (插件 API 版本 5)
- [Windows 64](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/API5-qt6-windows-x64.zip)
- [Linux amd64](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/API5-qt6-linux-amd64.zip)
- [Linux arm64](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/API5-qt6-linux-arm64.zip)
- [Linux armhf](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/API5-qt6-linux-armhf.zip)
- [Linux i386](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/API5-qt6-linux-i386.zip)

## OpenRGB 1.0 RC - 1.0 RC3 (插件 API 版本 4)
- [Windows 86](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/2.2.0b-API4-qt5-windows-x86.zip)
- [Windows 64](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/2.2.0b-API4-qt5-windows-x64.zip)
- [Linux amd64](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/2.2.0b-API4-qt5-linux-amd64.zip)
- [Linux arm64](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/2.2.0b-API4-qt5-linux-arm64.zip)
- [Linux armhf](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/2.2.0b-API4-qt5-linux-armhf.zip)
- [Linux i386](https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine/releases/download/v2.2.0/2.2.0b-API4-qt5-linux-i386.zip)

### 壁纸客户端
[GitHub](https://github.com/qiangqiang101/RGB-Wallpaper/releases) | [GitLab](https://gitlab.com/nollie/RGB-Wallpaper/-/releases)

## 封面图片
[封面图片](coverimages.md)

# 🚧 前置需求
- [OpenRGB (by CalcProgrammer1)](https://gitlab.com/CalcProgrammer1/OpenRGB)
- [.NET 10.0](https://dotnet.microsoft.com/en-us/download/dotnet/10.0)
- [小红车](https://store.steampowered.com/app/431960/Wallpaper_Engine/) 或 [Lively Wallpaper](https://github.com/rocksdanister/lively/releases/latest)
- [壁纸客户端 (GitHub)](https://github.com/qiangqiang101/RGB-Wallpaper/releases) 或 [壁纸客户端 (GitLab)](https://gitlab.com/nollie/RGB-Wallpaper/-/releases)

# 🔨 自行构建
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
   * **Qt6 版本:**
     ```bash
     cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2019_64" ..
     ```
   * **Qt5 版本:**
     ```bash
     cmake -DCMAKE_PREFIX_PATH="C:/Qt/5.15.x/msvc2019_64" ..
     ```
4. 构建插件：
   ```bash
   cmake --build . --config Release
   ```

构建完成后，你将在 `build` 文件夹中找到 `AnOpenRGBWallpaperPlugin.dll`（Windows）或 `AnOpenRGBWallpaperPlugin.so`（Linux）。

# 🚀 类似项目
- [SignalRGB Wallpaper Engine](https://github.com/qiangqiang101/SignalRGB-Wallpaper-Engine)
