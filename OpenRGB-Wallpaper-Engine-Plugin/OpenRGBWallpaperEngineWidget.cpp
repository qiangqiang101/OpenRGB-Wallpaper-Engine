#include "OpenRGBWallpaperEngineWidget.h"
#include "OpenRGBWallpaperEnginePlugin.h"
#include "ui_OpenRGBWallpaperEngineWidget.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>

OpenRGBWallpaperEngineWidget::OpenRGBWallpaperEngineWidget(OpenRGBWallpaperEnginePlugin* plugin_ptr, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::OpenRGBWallpaperEngineWidget)
    , plugin(plugin_ptr)
    , current_device_idx(-1)
    , updating_ui(false)
{
    ui->setupUi(this);
    
    // Populate dropdown items
    ui->size_combo->addItems({
        "4:3 Landscape", "4:3 Portrait", "5:4 Landscape", "5:4 Portrait",
        "16:9 Landscape", "16:9 Portrait", "16:10 Landscape", "16:10 Portrait",
        "21:9 Landscape", "21:9 Portrait", "32:9 Landscape", "32:9 Portrait",
        "4:1 Landscape", "4:1 Portrait"
    });
    
    ui->tier_combo->addItems({"Small", "Normal", "Large", "X Large"});
    
    ui->shape_combo->addItems({"Rectangle", "Circle", "Rounded Rectangle"});
    
    ui->shutdown_combo->addItems({
        "Solid Color", "Aurora", "Breathing", "Rainbow Wave (Left)", "Rainbow Wave (Right)",
        "Neon Wave (Left)", "Neon Wave (Right)", "Sunset Wave (Left)", "Sunset Wave (Right)",
        "Audio Party", "Rainbow Cycle", "Rainbow Pinwheel", "Fire"
    });
    
    ui->stretch_combo->addItems({"None", "Fill", "Uniform", "Uniform to Fill"});
    
    // Connections
    connect(ui->device_list, &QListWidget::currentRowChanged, this, &OpenRGBWallpaperEngineWidget::OnDeviceSelected);
    connect(ui->add_btn, &QPushButton::clicked, this, &OpenRGBWallpaperEngineWidget::OnAddDevice);
    connect(ui->remove_btn, &QPushButton::clicked, this, &OpenRGBWallpaperEngineWidget::OnRemoveDevice);
    connect(ui->bg_color_btn, &QPushButton::clicked, this, &OpenRGBWallpaperEngineWidget::OnSelectBgColor);
    connect(ui->sd_color_btn, &QPushButton::clicked, this, &OpenRGBWallpaperEngineWidget::OnSelectSdColor);
    connect(ui->browse_btn, &QPushButton::clicked, this, &OpenRGBWallpaperEngineWidget::OnBrowseCoverImage);
    connect(ui->save_btn, &QPushButton::clicked, this, &OpenRGBWallpaperEngineWidget::OnSaveChanges);
}

OpenRGBWallpaperEngineWidget::~OpenRGBWallpaperEngineWidget()
{
    delete ui;
}

void OpenRGBWallpaperEngineWidget::RefreshDevicesList()
{
    updating_ui = true;
    int prev_row = ui->device_list->currentRow();
    
    ui->device_list->clear();
    for (const auto& dev : plugin->devices)
    {
        ui->device_list->addItem(dev.name + " (" + dev.host + ":" + QString::number(dev.port) + ")");
    }
    
    if (prev_row >= 0 && prev_row < ui->device_list->count())
    {
        ui->device_list->setCurrentRow(prev_row);
    }
    else if (ui->device_list->count() > 0)
    {
        ui->device_list->setCurrentRow(0);
    }
    else
    {
        ui->detail_box->setEnabled(false);
        current_device_idx = -1;
    }
    updating_ui = false;
    
    // Explicitly load the active item to the form now that updating_ui guard is released
    LoadDeviceToForm(ui->device_list->currentRow());
}

void OpenRGBWallpaperEngineWidget::OnAddDevice()
{
    WallpaperDeviceConfig dev;
    dev.name = "New Display";
    dev.host = "127.0.0.1";
    dev.port = 8133 + plugin->devices.size();
    dev.size = "16:9 Landscape";
    dev.tier = "Normal";
    dev.blur = 20;
    dev.shape = 0;
    dev.radius = 2;
    dev.padding = 0;
    dev.bg_color = "#000000";
    dev.shutdown_color = "#009bde";
    dev.shutdown_effect = 0;
    dev.show_fps = false;
    dev.cover_stretch = 2;
    
    plugin->devices.push_back(dev);
    RefreshDevicesList();
    ui->device_list->setCurrentRow(ui->device_list->count() - 1);
}

void OpenRGBWallpaperEngineWidget::OnRemoveDevice()
{
    int row = ui->device_list->currentRow();
    if (row < 0 || row >= (int)plugin->devices.size()) return;
    
    if (plugin->devices.size() <= 1)
    {
        QMessageBox::warning(this, "Cannot Remove", "You must keep at least one wallpaper device configuration.");
        return;
    }
    
    plugin->devices.erase(plugin->devices.begin() + row);
    RefreshDevicesList();
}

void OpenRGBWallpaperEngineWidget::OnDeviceSelected(int index)
{
    if (updating_ui) return;
    LoadDeviceToForm(index);
}

void OpenRGBWallpaperEngineWidget::LoadDeviceToForm(int idx)
{
    if (idx < 0 || idx >= (int)plugin->devices.size())
    {
        ui->detail_box->setEnabled(false);
        current_device_idx = -1;
        return;
    }
    
    ui->detail_box->setEnabled(true);
    current_device_idx = idx;
    
    const auto& dev = plugin->devices[idx];
    
    updating_ui = true;
    ui->name_edit->setText(dev.name);
    ui->host_edit->setText(dev.host);
    ui->port_spin->setValue(dev.port);
    ui->size_combo->setCurrentText(dev.size);
    ui->tier_combo->setCurrentText(dev.tier);
    ui->blur_spin->setValue(dev.blur);
    ui->shape_combo->setCurrentIndex(dev.shape);
    ui->radius_spin->setValue(dev.radius);
    ui->padding_spin->setValue(dev.padding);
    ui->cover_edit->setText(dev.cover_image);
    ui->shutdown_combo->setCurrentIndex(dev.shutdown_effect >= 0 && dev.shutdown_effect <= 12 ? dev.shutdown_effect : 0);
    ui->fps_checkbox->setChecked(dev.show_fps);
    ui->stretch_combo->setCurrentIndex(dev.cover_stretch >= 0 && dev.cover_stretch <= 3 ? dev.cover_stretch : 2);
    
    bg_color = QColor(dev.bg_color);
    sd_color = QColor(dev.shutdown_color);
    UpdateColorButtonText(ui->bg_color_btn, bg_color);
    UpdateColorButtonText(ui->sd_color_btn, sd_color);
    
    updating_ui = false;
}

void OpenRGBWallpaperEngineWidget::OnSelectBgColor()
{
    QColor chosen = QColorDialog::getColor(bg_color, this, "Choose Background Color");
    if (chosen.isValid())
    {
        bg_color = chosen;
        UpdateColorButtonText(ui->bg_color_btn, bg_color);
    }
}

void OpenRGBWallpaperEngineWidget::OnSelectSdColor()
{
    QColor chosen = QColorDialog::getColor(sd_color, this, "Choose Shutdown Color");
    if (chosen.isValid())
    {
        sd_color = chosen;
        UpdateColorButtonText(ui->sd_color_btn, sd_color);
    }
}

void OpenRGBWallpaperEngineWidget::OnBrowseCoverImage()
{
    QString file_path = QFileDialog::getOpenFileName(
        this,
        "Select Cover Image File",
        ui->cover_edit->text(),
        "Image Files (*.png *.jpg *.jpeg *.bmp);;All Files (*)"
    );
    if (!file_path.isEmpty())
    {
        ui->cover_edit->setText(file_path);
    }
}

void OpenRGBWallpaperEngineWidget::UpdateColorButtonText(QPushButton* btn, const QColor& color)
{
    btn->setText(color.name().toUpper());
    btn->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold; border: 1px solid #777;")
        .arg(color.name())
        .arg(color.lightness() > 128 ? "black" : "white"));
}

void OpenRGBWallpaperEngineWidget::OnSaveChanges()
{
    if (current_device_idx < 0 || current_device_idx >= (int)plugin->devices.size()) return;
    
    auto& dev = plugin->devices[current_device_idx];
    dev.name            = ui->name_edit->text().trimmed();
    dev.host            = ui->host_edit->text().trimmed();
    dev.port            = ui->port_spin->value();
    dev.size            = ui->size_combo->currentText();
    dev.tier            = ui->tier_combo->currentText();
    dev.blur            = ui->blur_spin->value();
    dev.shape           = ui->shape_combo->currentIndex();
    dev.radius          = ui->radius_spin->value();
    dev.padding         = ui->padding_spin->value();
    dev.bg_color        = bg_color.name();
    dev.shutdown_color  = sd_color.name();
    dev.cover_image     = ui->cover_edit->text().trimmed();
    dev.shutdown_effect = ui->shutdown_combo->currentIndex();
    dev.show_fps        = ui->fps_checkbox->isChecked();
    dev.cover_stretch   = ui->stretch_combo->currentIndex();
    
    plugin->SaveConfig();
    plugin->RecreateControllers();
    
    // Refresh item label in list
    updating_ui = true;
    ui->device_list->item(current_device_idx)->setText(dev.name + " (" + dev.host + ":" + QString::number(dev.port) + ")");
    updating_ui = false;
}
