/*----------------------------------------------------------*\
| OpenRGBWallpaperEnginePlugin.cpp                           |
|                                                            |
|   OpenRGB Wallpaper Plugin                                 |
|                                                            |
|   This file is part of the OpenRGB Wallpaper Plugin project|
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*----------------------------------------------------------*/

#include "OpenRGBWallpaperEnginePlugin.h"
#include "OpenRGBWallpaperEngineWidget.h"
#include "RGBController.h"
#include <cmath>
#include <algorithm>
#include <cstring>
#include <QFile>
#include <QDir>
#include <QFileInfo>

static QString GetSettingsFilePath()
{
    QString appdata = qgetenv("APPDATA");
    if (appdata.isEmpty())
    {
        appdata = QDir::homePath() + "/AppData/Roaming";
    }
    return appdata + "/OpenRGB/plugins/settings/wallpaper.json";
}

static const int MAX_LEDS_IN_PACKET = 480;

// Matrix Sizes mapping helper (exactly same as wallpaper2.js)
static const std::map<QString, int> MATRIX_SIZE_MAP = {
    {"4:3 Landscape", 0}, {"4:3 Portrait", 1}, {"5:4 Landscape", 2}, {"5:4 Portrait", 3},
    {"16:9 Landscape", 4}, {"16:9 Portrait", 5}, {"16:10 Landscape", 6}, {"16:10 Portrait", 7},
    {"21:9 Landscape", 8}, {"21:9 Portrait", 9}, {"32:9 Landscape", 10}, {"32:9 Portrait", 11},
    {"4:1 Landscape", 12}, {"4:1 Portrait", 13}
};

static const std::map<QString, int> MATRIX_TIER_MAP = {
    {"Small", 0}, {"Normal", 1}, {"Large", 2}, {"X Large", 3}
};

struct SizeGrid {
    int w;
    int h;
};

static SizeGrid GetGridSize(const QString& size_str, const QString& tier_str) {
    int w = 48, h = 27; // Default 16:9 Normal
    
    if (size_str == "16:9 Landscape") {
        if (tier_str == "Small") { w = 32; h = 18; }
        else if (tier_str == "Normal") { w = 48; h = 27; }
        else if (tier_str == "Large") { w = 64; h = 36; }
        else if (tier_str == "X Large") { w = 128; h = 72; }
    }
    else if (size_str == "16:9 Portrait") {
        if (tier_str == "Small") { w = 18; h = 32; }
        else if (tier_str == "Normal") { w = 27; h = 48; }
        else if (tier_str == "Large") { w = 36; h = 64; }
        else if (tier_str == "X Large") { w = 72; h = 128; }
    }
    else if (size_str == "4:3 Landscape") {
        if (tier_str == "Small") { w = 8; h = 6; }
        else if (tier_str == "Normal") { w = 16; h = 12; }
        else if (tier_str == "Large") { w = 32; h = 24; }
        else if (tier_str == "X Large") { w = 64; h = 48; }
    }
    else if (size_str == "4:3 Portrait") {
        if (tier_str == "Small") { w = 6; h = 8; }
        else if (tier_str == "Normal") { w = 12; h = 16; }
        else if (tier_str == "Large") { w = 24; h = 32; }
        else if (tier_str == "X Large") { w = 48; h = 64; }
    }
    else if (size_str == "5:4 Landscape") {
        if (tier_str == "Small") { w = 10; h = 8; }
        else if (tier_str == "Normal") { w = 20; h = 16; }
        else if (tier_str == "Large") { w = 40; h = 32; }
        else if (tier_str == "X Large") { w = 80; h = 64; }
    }
    else if (size_str == "5:4 Portrait") {
        if (tier_str == "Small") { w = 8; h = 10; }
        else if (tier_str == "Normal") { w = 16; h = 20; }
        else if (tier_str == "Large") { w = 32; h = 40; }
        else if (tier_str == "X Large") { w = 64; h = 80; }
    }
    else if (size_str == "16:10 Landscape") {
        if (tier_str == "Small") { w = 32; h = 20; }
        else if (tier_str == "Normal") { w = 48; h = 30; }
        else if (tier_str == "Large") { w = 64; h = 40; }
        else if (tier_str == "X Large") { w = 128; h = 80; }
    }
    else if (size_str == "16:10 Portrait") {
        if (tier_str == "Small") { w = 20; h = 32; }
        else if (tier_str == "Normal") { w = 30; h = 48; }
        else if (tier_str == "Large") { w = 40; h = 64; }
        else if (tier_str == "X Large") { w = 80; h = 128; }
    }
    else if (size_str == "21:9 Landscape") {
        if (tier_str == "Small") { w = 42; h = 18; }
        else if (tier_str == "Normal") { w = 63; h = 27; }
        else if (tier_str == "Large") { w = 84; h = 36; }
        else if (tier_str == "X Large") { w = 168; h = 72; }
    }
    else if (size_str == "21:9 Portrait") {
        if (tier_str == "Small") { w = 18; h = 42; }
        else if (tier_str == "Normal") { w = 27; h = 63; }
        else if (tier_str == "Large") { w = 36; h = 84; }
        else if (tier_str == "X Large") { w = 72; h = 168; }
    }
    else if (size_str == "32:9 Landscape") {
        if (tier_str == "Small") { w = 64; h = 18; }
        else if (tier_str == "Normal") { w = 96; h = 27; }
        else if (tier_str == "Large") { w = 128; h = 36; }
        else if (tier_str == "X Large") { w = 256; h = 72; }
    }
    else if (size_str == "32:9 Portrait") {
        if (tier_str == "Small") { w = 18; h = 64; }
        else if (tier_str == "Normal") { w = 27; h = 96; }
        else if (tier_str == "Large") { w = 36; h = 128; }
        else if (tier_str == "X Large") { w = 72; h = 256; }
    }
    else if (size_str == "4:1 Landscape") {
        if (tier_str == "Small") { w = 8; h = 2; }
        else if (tier_str == "Normal") { w = 16; h = 4; }
        else if (tier_str == "Large") { w = 32; h = 8; }
        else if (tier_str == "X Large") { w = 64; h = 16; }
    }
    else if (size_str == "4:1 Portrait") {
        if (tier_str == "Small") { w = 2; h = 8; }
        else if (tier_str == "Normal") { w = 4; h = 16; }
        else if (tier_str == "Large") { w = 8; h = 32; }
        else if (tier_str == "X Large") { w = 16; h = 64; }
    }
    
    return {w, h};
}

OpenRGBWallpaperEnginePlugin::OpenRGBWallpaperEnginePlugin()
{
    plugin_api = nullptr;
    widget     = nullptr;
    udp_socket = INVALID_SOCKET;
}

OpenRGBWallpaperEnginePlugin::~OpenRGBWallpaperEnginePlugin()
{
    Unload();
}

OpenRGBPluginInfo OpenRGBWallpaperEnginePlugin::GetPluginInfo()
{
    OpenRGBPluginInfo info;
    info.Name        = "OpenRGB Wallpaper Engine & Lively Wallpaper Plugin";
    info.Description = "Turn your wallpaper into an OpenRGB device. You can now control the RGB lighting on your wallpaper and synchronize wallpaper with other OpenRGB compatible devices by OpenRGB.";
    info.Version     = "2.0.0";
    info.Commit      = "master";
    info.URL         = "https://github.com/qiangqiang101/OpenRGB-Wallpaper-Engine";
    info.Location    = OPENRGB_PLUGIN_LOCATION_SETTINGS;
    info.Label       = "Wallpaper Devices";
	info.Icon.load(":/OpenRGBWallpaperEnginePlugin.png");
    return info;
}

unsigned int OpenRGBWallpaperEnginePlugin::GetPluginAPIVersion()
{
    return OPENRGB_PLUGIN_API_VERSION;
}

void OpenRGBWallpaperEnginePlugin::Load(OpenRGBPluginAPIInterface* plugin_api_ptr)
{
    plugin_api = plugin_api_ptr;
    
    LoadConfig();
    InitSocket();
    
    // Register controllers
    for (const auto& dev : devices)
    {
        WallpaperRGBController* controller = new WallpaperRGBController(dev);
        controller->update_callback = [this, dev](const std::vector<RGBColor>& colors) {
            SendRGBData(dev, colors);
        };
        controllers.push_back(controller);
        plugin_api->RegisterVirtualRGBController(controller);
        
        // Push initial/black frame to trigger connection
        SendRGBData(dev, controller->GetColors());
    }
}

void OpenRGBWallpaperEnginePlugin::Unload()
{
    // Unregister and delete virtual controllers
    if (plugin_api)
    {
        for (auto* controller : controllers)
        {
            plugin_api->UnregisterVirtualRGBController(controller);
            delete controller;
        }
    }
    controllers.clear();
    
    CloseSocket();
    
    if (widget)
    {
        delete widget;
        widget = nullptr;
    }
}

QWidget* OpenRGBWallpaperEnginePlugin::GetWidget()
{
    if (!widget)
    {
        widget = new OpenRGBWallpaperEngineWidget(this);
    }
    widget->RefreshDevicesList();
    return widget;
}

QMenu* OpenRGBWallpaperEnginePlugin::GetTrayMenu()
{
    return nullptr;
}

void OpenRGBWallpaperEnginePlugin::LoadConfig()
{
    devices.clear();
    QString path = GetSettingsFilePath();
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
    {
        // Add a default device
        WallpaperDeviceConfig def;
        def.name = "Primary Display";
        def.host = "127.0.0.1";
        def.port = 8133;
        def.size = "16:9 Landscape";
        def.tier = "Normal";
        def.blur = 20;
        def.shape = 0;
        def.radius = 2;
        def.padding = 0;
        def.bg_color = "#000000";
        def.shutdown_color = "#009bde";
        def.shutdown_effect = 0;
        def.show_fps = false;
        def.cover_stretch = 2;
        devices.push_back(def);
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray arr = doc.array();
    for (int i = 0; i < arr.size(); ++i)
    {
        QJsonObject obj = arr[i].toObject();
        WallpaperDeviceConfig dev;
        dev.name = obj["name"].toString();
        dev.host = obj["host"].toString();
        dev.port = obj["port"].toInt();
        dev.size = obj["size"].toString();
        dev.tier = obj["tier"].toString();
        dev.blur = obj["blur"].toInt();
        dev.shape = obj["shape"].toInt();
        dev.radius = obj["radius"].toInt();
        dev.padding = obj["padding"].toInt();
        dev.bg_color = obj["bg_color"].toString();
        dev.shutdown_color = obj["shutdown_color"].toString();
        dev.cover_image = obj["cover_image"].toString();
        dev.shutdown_effect = obj["shutdown_effect"].toInt(0);
        dev.show_fps = obj["show_fps"].toBool(false);
        dev.cover_stretch = obj["cover_stretch"].toInt(2);
        devices.push_back(dev);
    }
}

void OpenRGBWallpaperEnginePlugin::SaveConfig()
{
    QString path = GetSettingsFilePath();
    QFileInfo file_info(path);
    QDir dir = file_info.dir();
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }
    
    QJsonArray arr;
    for (const auto& dev : devices)
    {
        QJsonObject obj;
        obj["name"] = dev.name;
        obj["host"] = dev.host;
        obj["port"] = dev.port;
        obj["size"] = dev.size;
        obj["tier"] = dev.tier;
        obj["blur"] = dev.blur;
        obj["shape"] = dev.shape;
        obj["radius"] = dev.radius;
        obj["padding"] = dev.padding;
        obj["bg_color"] = dev.bg_color;
        obj["shutdown_color"] = dev.shutdown_color;
        obj["cover_image"] = dev.cover_image;
        obj["shutdown_effect"] = dev.shutdown_effect;
        obj["show_fps"] = dev.show_fps;
        obj["cover_stretch"] = dev.cover_stretch;
        arr.append(obj);
    }
    QJsonDocument doc(arr);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void OpenRGBWallpaperEnginePlugin::RecreateControllers()
{
    if (!plugin_api) return;
    
    // Unregister and delete old ones
    for (auto* controller : controllers)
    {
        plugin_api->UnregisterVirtualRGBController(controller);
        delete controller;
    }
    controllers.clear();
    last_settings_map.clear();
    
    // Register new ones
    for (const auto& dev : devices)
    {
        WallpaperRGBController* controller = new WallpaperRGBController(dev);
        controller->update_callback = [this, dev](const std::vector<RGBColor>& colors) {
            SendRGBData(dev, colors);
        };
        controllers.push_back(controller);
        plugin_api->RegisterVirtualRGBController(controller);
        
        // Push initial config settings
        SendRGBData(dev, controller->GetColors());
    }
}

void OpenRGBWallpaperEnginePlugin::InitSocket()
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

void OpenRGBWallpaperEnginePlugin::CloseSocket()
{
    if (udp_socket != INVALID_SOCKET)
    {
        closesocket(udp_socket);
        udp_socket = INVALID_SOCKET;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

void OpenRGBWallpaperEnginePlugin::SendDatagram(const WallpaperDeviceConfig& dev, const QByteArray& data)
{
    if (udp_socket == INVALID_SOCKET || dev.host.isEmpty()) return;
    
    sockaddr_in dest;
    std::memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dev.port);
    
#ifdef _WIN32
    inet_pton(AF_INET, dev.host.toStdString().c_str(), &dest.sin_addr);
#else
    dest.sin_addr.s_addr = inet_addr(dev.host.toStdString().c_str());
#endif
    
    sendto(udp_socket, data.constData(), data.size(), 0, (struct sockaddr*)&dest, sizeof(dest));
}

void OpenRGBWallpaperEnginePlugin::SendRGBData(const WallpaperDeviceConfig& dev, const std::vector<RGBColor>& colors)
{
    if (udp_socket == INVALID_SOCKET) return;
    
    // Send Settings packet if config changed
    QByteArray settings_pkt = CreateSettingsPacket(dev);
    if (last_settings_map[dev.port] != settings_pkt)
    {
        SendDatagram(dev, settings_pkt);
        last_settings_map[dev.port] = settings_pkt;
    }
    
    // Resolve target grid sizes
    SizeGrid grid = GetGridSize(dev.size, dev.tier);
    int target_leds = grid.w * grid.h;
    
    // Convert current RGBColor buffer to R, G, B bytes
    std::vector<unsigned char> source_colors;
    source_colors.reserve(colors.size() * 3);
    for (RGBColor col : colors)
    {
        source_colors.push_back(RGBGetRValue(col));
        source_colors.push_back(RGBGetGValue(col));
        source_colors.push_back(RGBGetBValue(col));
    }
    
    // Interpolate the colors to fit the grid matrix
    std::vector<unsigned char> interpolated = InterpolateColors(source_colors, target_leds);
    
    // Create and stream color packets
    std::vector<QByteArray> pkts = CreateColorPackets(interpolated);
    for (const QByteArray& pkt : pkts)
    {
        SendDatagram(dev, pkt);
    }
}

QByteArray OpenRGBWallpaperEnginePlugin::CreateSettingsPacket(const WallpaperDeviceConfig& dev)
{
    QByteArray packet;
    packet.append((char)0x01); // SettingPacketHeader = 0x01
    
    int size_idx = 4;
    auto size_it = MATRIX_SIZE_MAP.find(dev.size);
    if (size_it != MATRIX_SIZE_MAP.end()) size_idx = size_it->second;
    
    int tier_idx = 1;
    auto tier_it = MATRIX_TIER_MAP.find(dev.tier);
    if (tier_it != MATRIX_TIER_MAP.end()) tier_idx = tier_it->second;
    
    packet.append((char)size_idx);
    packet.append((char)tier_idx);
    packet.append((char)dev.shutdown_effect);
    packet.append((char)(dev.show_fps ? 1 : 0));
    packet.append((char)dev.blur);
    packet.append((char)dev.shape);
    packet.append((char)dev.radius);
    packet.append((char)dev.padding);
    packet.append((char)60); // Default FPS inside packet format
    packet.append((char)dev.cover_stretch);
    packet.append((char)0);  // CpuUsagePauseValue = 0
    
    // Background Color
    QColor bg(dev.bg_color);
    if (!bg.isValid()) bg = QColor(0, 0, 0);
    packet.append((char)bg.red());
    packet.append((char)bg.green());
    packet.append((char)bg.blue());
    
    // Shutdown Color
    QColor sd(dev.shutdown_color);
    if (!sd.isValid()) sd = QColor(0, 155, 222);
    packet.append((char)sd.red());
    packet.append((char)sd.green());
    packet.append((char)sd.blue());
    
    // Cover Image URL Length Prefix & Bytes
    QString cover_url = dev.cover_image;
    if (cover_url.isEmpty())
    {
        cover_url = "https://github.com/qiangqiang101/OpenRGB-Wallpaper/raw/master/Wallpaper-Wallpaper/razer5.png?raw=true";
    }
    
    QByteArray cover_bytes = cover_url.toUtf8();
    packet.append((char)cover_bytes.length());
    packet.append(cover_bytes);
    
    return packet;
}

std::vector<QByteArray> OpenRGBWallpaperEnginePlugin::CreateColorPackets(const std::vector<unsigned char>& rgb_data)
{
    int led_count = rgb_data.size() / 3;
    int num_packets = (int)std::ceil((double)led_count / MAX_LEDS_IN_PACKET);
    std::vector<QByteArray> packets;
    
    for (int curr_packet = 0; curr_packet < num_packets; ++curr_packet)
    {
        QByteArray packet;
        packet.append((char)0x00); // ColorPacketHeader = 0x00
        packet.append((char)curr_packet);
        packet.append((char)num_packets);
        
        int start = curr_packet * MAX_LEDS_IN_PACKET * 3;
        int end = std::min((curr_packet + 1) * MAX_LEDS_IN_PACKET * 3, (int)rgb_data.size());
        
        packet.append((const char*)&rgb_data[start], end - start);
        packets.push_back(packet);
    }
    return packets;
}

std::vector<unsigned char> OpenRGBWallpaperEnginePlugin::InterpolateColors(const std::vector<unsigned char>& source_colors, int target_count)
{
    if (target_count <= 1)
    {
        std::vector<unsigned char> output(target_count * 3, 0);
        if (!source_colors.empty() && target_count == 1)
        {
            output[0] = source_colors[0];
            output[1] = source_colors[1];
            output[2] = source_colors[2];
        }
        return output;
    }

    if (source_colors.empty())
    {
        return std::vector<unsigned char>(target_count * 3, 0);
    }
    
    int num_source = source_colors.size() / 3;
    std::vector<unsigned char> output(target_count * 3, 0);
    
    if (num_source == 1)
    {
        unsigned char r = source_colors[0];
        unsigned char g = source_colors[1];
        unsigned char b = source_colors[2];
        for (int j = 0; j < target_count; ++j)
        {
            output[j * 3] = r;
            output[j * 3 + 1] = g;
            output[j * 3 + 2] = b;
        }
        return output;
    }
    
    for (int j = 0; j < target_count; ++j)
    {
        double idx = (double)j * (num_source - 1) / (target_count - 1);
        int idx_low = (int)std::floor(idx);
        int idx_high = std::min(idx_low + 1, num_source - 1);
        double weight = idx - idx_low;
        
        int src_low_idx = idx_low * 3;
        int src_high_idx = idx_high * 3;
        
        unsigned char r_low = source_colors[src_low_idx];
        unsigned char g_low = source_colors[src_low_idx + 1];
        unsigned char b_low = source_colors[src_low_idx + 2];
        
        unsigned char r_high = source_colors[src_high_idx];
        unsigned char g_high = source_colors[src_high_idx + 1];
        unsigned char b_high = source_colors[src_high_idx + 2];
        
        output[j * 3]     = (unsigned char)(r_low + (r_high - r_low) * weight);
        output[j * 3 + 1] = (unsigned char)(g_low + (g_high - g_low) * weight);
        output[j * 3 + 2] = (unsigned char)(b_low + (b_high - b_low) * weight);
    }
    
    return output;
}

// WallpaperRGBController Implementation
WallpaperRGBController::WallpaperRGBController(const WallpaperDeviceConfig& config)
{
    name        = config.name.toStdString();
    vendor      = "SignalRGB-WPE";
    description = "Virtual Wallpaper Engine Device";
    version     = "2.0.0";
    type        = DEVICE_TYPE_VIRTUAL;
    
    SizeGrid grid = GetGridSize(config.size, config.tier);
    width  = grid.w;
    height = grid.h;
    
    // Add Direct Mode
    mode direct_mode;
    direct_mode.name = "Direct";
    direct_mode.value = 0;
    direct_mode.flags = MODE_FLAG_HAS_PER_LED_COLOR;
    direct_mode.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(direct_mode);
    active_mode = 0;
    
    SetupZones();
    SetupColors();
}

WallpaperRGBController::~WallpaperRGBController()
{
}

void WallpaperRGBController::SetupZones()
{
    zones.clear();
    leds.clear();
    colors.clear();

    zone wpe_zone;
    wpe_zone.name = "Wallpaper Matrix";
    wpe_zone.type = ZONE_TYPE_MATRIX;
    wpe_zone.leds_count = width * height;
    wpe_zone.leds_min = wpe_zone.leds_count;
    wpe_zone.leds_max = wpe_zone.leds_count;
    wpe_zone.start_idx = 0;
    
    wpe_zone.matrix_map.Set(height, width, nullptr);
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            wpe_zone.matrix_map.map[y * width + x] = y * width + x;
        }
    }
    
    zones.push_back(wpe_zone);
    
    for (int i = 0; i < width * height; ++i)
    {
        led new_led;
        new_led.name = "LED " + std::to_string(i + 1);
        new_led.value = i;
        leds.push_back(new_led);
    }
    
    colors.resize(width * height, 0);
}

void WallpaperRGBController::DeviceUpdateLEDs()
{
    if (update_callback)
    {
        update_callback(colors);
    }
}

void WallpaperRGBController::DeviceUpdateZoneLEDs(int /*zone_idx*/)
{
    DeviceUpdateLEDs();
}

void WallpaperRGBController::DeviceUpdateSingleLED(int /*led_idx*/)
{
    DeviceUpdateLEDs();
}

void WallpaperRGBController::DeviceUpdateMode()
{
    // Direct mode by default
}
