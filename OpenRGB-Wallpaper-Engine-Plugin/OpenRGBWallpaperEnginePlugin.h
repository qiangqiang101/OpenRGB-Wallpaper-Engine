/*----------------------------------------------------------*\
| OpenRGBWallpaperEnginePlugin.h                             |
|                                                            |
|   OpenRGB Wallpaper Plugin                                 |
|                                                            |
|   This file is part of the OpenRGB Wallpaper Plugin project|
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*----------------------------------------------------------*/

#pragma once

#define NOMINMAX
#include <QObject>
#include <QTimer>
#include <QSettings>
#include <QColor>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <map>
#include <vector>
#include <functional>
#include "OpenRGBPluginInterface.h"
#include "RGBController.h"

// Define platform-specific socket types
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define closesocket close
#endif

// Configuration structure for a single wallpaper device
struct WallpaperDeviceConfig
{
    QString name;
    QString host;
    int port;
    QString size;
    QString tier;
    int blur;
    int shape;
    int radius;
    int padding;
    QString bg_color;
    QString shutdown_color;
    QString cover_image;
    int shutdown_effect;
    bool show_fps;
    int cover_stretch;
};

class OpenRGBWallpaperEngineWidget;
class WallpaperRGBController;

class OpenRGBWallpaperEnginePlugin : public QObject, public OpenRGBPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(OpenRGBPluginInterface)
    Q_PLUGIN_METADATA(IID "org.openrgb.OpenRGBPluginInterface" FILE "OpenRGBWallpaperEnginePlugin.json")
    
public:
    OpenRGBWallpaperEnginePlugin();
    virtual ~OpenRGBWallpaperEnginePlugin();

    // Required methods from OpenRGBPluginInterface
    virtual OpenRGBPluginInfo   GetPluginInfo() override;
    virtual unsigned int        GetPluginAPIVersion() override;

    virtual void                Load(OpenRGBPluginAPIInterface* plugin_api_ptr) override;
    virtual QWidget*            GetWidget() override;
    virtual QMenu*              GetTrayMenu() override;
    virtual void                Unload() override;

    // OpenRGBPluginInterface overrides for API Version 5
    virtual void                OnProfileAboutToLoad() override {}
    virtual void                OnProfileLoad(nlohmann::json profile_data) override {}
    virtual nlohmann::json      OnProfileSave() override { return nlohmann::json(); }
    virtual unsigned char*      OnSDKCommand(unsigned int pkt_id, unsigned char * pkt_data, unsigned int *pkt_size) override { return nullptr; }

    virtual void                ProfileManagerUpdated(unsigned int update_reason) override {}
    virtual void                ResourceManagerUpdated(unsigned int update_reason) override {}
    virtual void                SettingsManagerUpdated(unsigned int update_reason) override {}

    // Helper functions for settings and config
    void                        LoadConfig();
    void                        SaveConfig();
    void                        RecreateControllers();

private:
    OpenRGBPluginAPIInterface*  plugin_api;
    OpenRGBWallpaperEngineWidget* widget;
    
    // Native UDP socket
    SOCKET                      udp_socket;
    
    // Configured devices and virtual controllers
    std::vector<WallpaperDeviceConfig> devices;
    std::vector<WallpaperRGBController*> controllers;
    
    std::map<int, QByteArray>   last_settings_map;
    
    friend class OpenRGBWallpaperEngineWidget;
    
    void                        InitSocket();
    void                        CloseSocket();
    void                        SendDatagram(const WallpaperDeviceConfig& dev, const QByteArray& data);
    void                        SendRGBData(const WallpaperDeviceConfig& dev, const std::vector<RGBColor>& colors);
    
    QByteArray                  CreateSettingsPacket(const WallpaperDeviceConfig& dev);
    std::vector<QByteArray>     CreateColorPackets(const std::vector<unsigned char>& rgb_data);
    std::vector<unsigned char>  InterpolateColors(const std::vector<unsigned char>& source_colors, int target_count);
};

// Custom virtual device class in OpenRGB
class WallpaperRGBController : public RGBController
{
public:
    WallpaperRGBController(const WallpaperDeviceConfig& config);
    virtual ~WallpaperRGBController();

    std::vector<RGBColor> GetColors() const { return colors; }

    void SetupZones();
    virtual void DeviceUpdateLEDs() override;
    virtual void DeviceUpdateZoneLEDs(int zone) override;
    virtual void DeviceUpdateSingleLED(int led) override;
    virtual void DeviceUpdateMode() override;

    std::function<void(const std::vector<RGBColor>&)> update_callback;

private:
    int width;
    int height;
};
