#pragma once

#include <QWidget>
#include <QColor>
#include <QPushButton>

class OpenRGBWallpaperEnginePlugin;

namespace Ui {
class OpenRGBWallpaperEngineWidget;
}

class OpenRGBWallpaperEngineWidget : public QWidget
{
    Q_OBJECT
    
public:
    OpenRGBWallpaperEngineWidget(OpenRGBWallpaperEnginePlugin* plugin_ptr, QWidget* parent = nullptr);
    virtual ~OpenRGBWallpaperEngineWidget();

    void RefreshDevicesList();

private slots:
    void OnAddDevice();
    void OnRemoveDevice();
    void OnDeviceSelected(int index);
    
    void OnSelectBgColor();
    void OnSelectSdColor();
    void OnBrowseCoverImage();
    void OnSaveChanges();

private:
    Ui::OpenRGBWallpaperEngineWidget* ui;
    OpenRGBWallpaperEnginePlugin*     plugin;
    
    // Current editing colors
    QColor              bg_color;
    QColor              sd_color;
    
    int                 current_device_idx;
    bool                updating_ui;
    
    void                UpdateColorButtonText(QPushButton* btn, const QColor& color);
    void                LoadDeviceToForm(int idx);
};
