# OpenRGB Waywallen plugin

This directory contains a Linux C++ port of the OpenRGB wallpaper receiver logic.

## What it does
- listens for OpenRGB UDP packets on port 8133 by default
- parses RGB frame payloads into a simple matrix of color cells
- renders the frame to an SDL window with optional background image support

## Build
```bash
cmake -S waywallen-plugin -B build/waywallen-plugin
cmake --build build/waywallen-plugin -j
```

## Run
```bash
./build/waywallen-plugin/openrgb-waywallen-plugin --udp-port 8133
```
