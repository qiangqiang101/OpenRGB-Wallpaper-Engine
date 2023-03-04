# OpenRGB Wallpaper Engine
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

 ![image](https://user-images.githubusercontent.com/11488961/201601205-465ca003-1300-4caa-a7e5-1897fb00119f.png)
 
# Screenshots 
![IMG_6915](https://user-images.githubusercontent.com/11488961/202174752-3ecf4780-be04-40de-9382-d5ad14732104.JPG)
![IMG_6916](https://user-images.githubusercontent.com/11488961/202174762-a31ea030-35ec-47d6-a1b7-d8cee2229893.JPG)
![IMG_6917](https://user-images.githubusercontent.com/11488961/202174765-5b2bbdfc-581e-4bf2-ab65-979c0533dd4b.JPG)
![IMG_6919](https://user-images.githubusercontent.com/11488961/202174769-6164a88d-039d-4922-a501-51649a4a2da6.JPG)

# Download
https://steamcommunity.com/sharedfiles/filedetails/?id=2942091593

# Prerequisite
- [OpenRGB by CalcProgrammer1](https://gitlab.com/CalcProgrammer1/OpenRGB)
- [.NET 6.0 Runtime](https://dotnet.microsoft.com/en-us/download/dotnet/6.0)

# About Virus
From the latest release [test result](https://www.virustotal.com/gui/file/34a2b0d40e1ae37f1184a6c3e52a1a207f5456cbdc6ecfa9f0539e864461043b/detection) shows that 2 out of 70 vendors detected as Malicious, it's false positive, you're safe, if you have doubts, you can build the project by yourself, build instructions can be found below.

# Build yourself
You need Visual Studio 2022 or newer, run OpenRGBWallpaperEngine.sln to begin. Click the Build on menu, select Build Solution.

# How To
1. Run OpenRGB, go to Settings tab, Click E1.31 Devices.
2. Click Add button at bottom, Enter:
```
Name: Wallpaper1 (Must identical with Device Name on Wallpaper Engine)
IP: Put a bogus IP (Example: 0.0.0.1)
Start Universe: 1
Start Channel: 1
Number of LEDs: 2304
Type: Matrix
Matrix Width: 64
Matrix Height: 36
Matrix Order: Horizontal Top Left
RGB Order: GRB
Universe Size: 510
Keepalive Time: 1000
```
![image](https://user-images.githubusercontent.com/11488961/201520080-4f8fc71e-c041-4509-87f4-c31f5819d11f.png)

3. Save Settings, Click Rescan Devices or Restart OpenRGB.
4. On OpenRGB, go to SDK Server, click Start Server.
5. OpenRGB Wallpaper should now automatically connects to OpenRGB.

Note: 
1. The Device name on Wallpaper Engine properties must Identical with the name on E1.31.
2. You are allow to change the Matrix Size, but make sure your LED count is matching the size, IE: 
```
Width  Height  LEDs
  16  x  9   = 144
  32  x  18  = 576
  64  x  36  = 2304
  256 x  144 = 36864
```
* The higher the LED count, the higher the CPU usage used.

# Special Thanks
- [OpenRGB by CalcProgrammer1](https://gitlab.com/CalcProgrammer1/OpenRGB)
- [OpenRGB.NET SDK by diogotr7](https://github.com/diogotr7/OpenRGB.NET)
