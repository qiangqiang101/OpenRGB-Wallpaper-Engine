/********************************************************************************
** Form generated from reading UI file 'OpenRGBWallpaperEngineWidget.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPENRGBWALLPAPERENGINEWIDGET_H
#define UI_OPENRGBWALLPAPERENGINEWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OpenRGBWallpaperEngineWidget
{
public:
    QHBoxLayout *mainLayout;
    QFrame *frame;
    QGridLayout *gridLayout;
    QGroupBox *detail_box;
    QFormLayout *formLayout;
    QLabel *nameLabel;
    QLineEdit *name_edit;
    QLabel *hostLabel;
    QLineEdit *host_edit;
    QLabel *portLabel;
    QSpinBox *port_spin;
    QLabel *aspectLabel;
    QComboBox *size_combo;
    QLabel *densityLabel;
    QComboBox *tier_combo;
    QLabel *blurLabel;
    QSpinBox *blur_spin;
    QLabel *shapeLabel;
    QComboBox *shape_combo;
    QLabel *radiusLabel;
    QSpinBox *radius_spin;
    QLabel *paddingLabel;
    QSpinBox *padding_spin;
    QLabel *bgColorLabel;
    QPushButton *bg_color_btn;
    QLabel *sdColorLabel;
    QPushButton *sd_color_btn;
    QLabel *sdEffectLabel;
    QComboBox *shutdown_combo;
    QLabel *showFpsLabel;
    QCheckBox *fps_checkbox;
    QLabel *stretchLabel;
    QComboBox *stretch_combo;
    QLabel *coverLabel;
    QHBoxLayout *coverLayout;
    QLineEdit *cover_edit;
    QPushButton *browse_btn;
    QPushButton *save_btn;
    QFrame *frame1;
    QVBoxLayout *leftLayout;
    QListWidget *device_list;
    QHBoxLayout *listButtonsLayout;
    QPushButton *add_btn;
    QPushButton *remove_btn;

    void setupUi(QWidget *OpenRGBWallpaperEngineWidget)
    {
        if (OpenRGBWallpaperEngineWidget->objectName().isEmpty())
            OpenRGBWallpaperEngineWidget->setObjectName("OpenRGBWallpaperEngineWidget");
        OpenRGBWallpaperEngineWidget->resize(618, 692);
        mainLayout = new QHBoxLayout(OpenRGBWallpaperEngineWidget);
        mainLayout->setSpacing(15);
        mainLayout->setObjectName("mainLayout");
        mainLayout->setContentsMargins(10, 10, 10, 10);
        frame = new QFrame(OpenRGBWallpaperEngineWidget);
        frame->setObjectName("frame");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(frame->sizePolicy().hasHeightForWidth());
        frame->setSizePolicy(sizePolicy);
        frame->setMinimumSize(QSize(0, 0));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Sunken);
        gridLayout = new QGridLayout(frame);
        gridLayout->setObjectName("gridLayout");
        detail_box = new QGroupBox(frame);
        detail_box->setObjectName("detail_box");
        detail_box->setMinimumSize(QSize(400, 0));
        detail_box->setStyleSheet(QString::fromUtf8("QGroupBox::title { font-weight: bold; font-size: 11pt; }"));
        formLayout = new QFormLayout(detail_box);
        formLayout->setObjectName("formLayout");
        formLayout->setHorizontalSpacing(10);
        formLayout->setVerticalSpacing(10);
        formLayout->setContentsMargins(15, 20, 15, 15);
        nameLabel = new QLabel(detail_box);
        nameLabel->setObjectName("nameLabel");

        formLayout->setWidget(0, QFormLayout::LabelRole, nameLabel);

        name_edit = new QLineEdit(detail_box);
        name_edit->setObjectName("name_edit");

        formLayout->setWidget(0, QFormLayout::FieldRole, name_edit);

        hostLabel = new QLabel(detail_box);
        hostLabel->setObjectName("hostLabel");

        formLayout->setWidget(1, QFormLayout::LabelRole, hostLabel);

        host_edit = new QLineEdit(detail_box);
        host_edit->setObjectName("host_edit");

        formLayout->setWidget(1, QFormLayout::FieldRole, host_edit);

        portLabel = new QLabel(detail_box);
        portLabel->setObjectName("portLabel");

        formLayout->setWidget(2, QFormLayout::LabelRole, portLabel);

        port_spin = new QSpinBox(detail_box);
        port_spin->setObjectName("port_spin");
        port_spin->setMinimum(1);
        port_spin->setMaximum(65535);

        formLayout->setWidget(2, QFormLayout::FieldRole, port_spin);

        aspectLabel = new QLabel(detail_box);
        aspectLabel->setObjectName("aspectLabel");

        formLayout->setWidget(3, QFormLayout::LabelRole, aspectLabel);

        size_combo = new QComboBox(detail_box);
        size_combo->setObjectName("size_combo");

        formLayout->setWidget(3, QFormLayout::FieldRole, size_combo);

        densityLabel = new QLabel(detail_box);
        densityLabel->setObjectName("densityLabel");

        formLayout->setWidget(4, QFormLayout::LabelRole, densityLabel);

        tier_combo = new QComboBox(detail_box);
        tier_combo->setObjectName("tier_combo");

        formLayout->setWidget(4, QFormLayout::FieldRole, tier_combo);

        blurLabel = new QLabel(detail_box);
        blurLabel->setObjectName("blurLabel");

        formLayout->setWidget(5, QFormLayout::LabelRole, blurLabel);

        blur_spin = new QSpinBox(detail_box);
        blur_spin->setObjectName("blur_spin");
        blur_spin->setMaximum(100);

        formLayout->setWidget(5, QFormLayout::FieldRole, blur_spin);

        shapeLabel = new QLabel(detail_box);
        shapeLabel->setObjectName("shapeLabel");

        formLayout->setWidget(6, QFormLayout::LabelRole, shapeLabel);

        shape_combo = new QComboBox(detail_box);
        shape_combo->setObjectName("shape_combo");

        formLayout->setWidget(6, QFormLayout::FieldRole, shape_combo);

        radiusLabel = new QLabel(detail_box);
        radiusLabel->setObjectName("radiusLabel");

        formLayout->setWidget(7, QFormLayout::LabelRole, radiusLabel);

        radius_spin = new QSpinBox(detail_box);
        radius_spin->setObjectName("radius_spin");
        radius_spin->setMaximum(20);

        formLayout->setWidget(7, QFormLayout::FieldRole, radius_spin);

        paddingLabel = new QLabel(detail_box);
        paddingLabel->setObjectName("paddingLabel");

        formLayout->setWidget(8, QFormLayout::LabelRole, paddingLabel);

        padding_spin = new QSpinBox(detail_box);
        padding_spin->setObjectName("padding_spin");
        padding_spin->setMaximum(250);

        formLayout->setWidget(8, QFormLayout::FieldRole, padding_spin);

        bgColorLabel = new QLabel(detail_box);
        bgColorLabel->setObjectName("bgColorLabel");

        formLayout->setWidget(9, QFormLayout::LabelRole, bgColorLabel);

        bg_color_btn = new QPushButton(detail_box);
        bg_color_btn->setObjectName("bg_color_btn");

        formLayout->setWidget(9, QFormLayout::FieldRole, bg_color_btn);

        sdColorLabel = new QLabel(detail_box);
        sdColorLabel->setObjectName("sdColorLabel");

        formLayout->setWidget(10, QFormLayout::LabelRole, sdColorLabel);

        sd_color_btn = new QPushButton(detail_box);
        sd_color_btn->setObjectName("sd_color_btn");

        formLayout->setWidget(10, QFormLayout::FieldRole, sd_color_btn);

        sdEffectLabel = new QLabel(detail_box);
        sdEffectLabel->setObjectName("sdEffectLabel");

        formLayout->setWidget(11, QFormLayout::LabelRole, sdEffectLabel);

        shutdown_combo = new QComboBox(detail_box);
        shutdown_combo->setObjectName("shutdown_combo");

        formLayout->setWidget(11, QFormLayout::FieldRole, shutdown_combo);

        showFpsLabel = new QLabel(detail_box);
        showFpsLabel->setObjectName("showFpsLabel");

        formLayout->setWidget(12, QFormLayout::LabelRole, showFpsLabel);

        fps_checkbox = new QCheckBox(detail_box);
        fps_checkbox->setObjectName("fps_checkbox");

        formLayout->setWidget(12, QFormLayout::FieldRole, fps_checkbox);

        stretchLabel = new QLabel(detail_box);
        stretchLabel->setObjectName("stretchLabel");

        formLayout->setWidget(13, QFormLayout::LabelRole, stretchLabel);

        stretch_combo = new QComboBox(detail_box);
        stretch_combo->setObjectName("stretch_combo");

        formLayout->setWidget(13, QFormLayout::FieldRole, stretch_combo);

        coverLabel = new QLabel(detail_box);
        coverLabel->setObjectName("coverLabel");

        formLayout->setWidget(14, QFormLayout::LabelRole, coverLabel);

        coverLayout = new QHBoxLayout();
        coverLayout->setObjectName("coverLayout");
        cover_edit = new QLineEdit(detail_box);
        cover_edit->setObjectName("cover_edit");

        coverLayout->addWidget(cover_edit);

        browse_btn = new QPushButton(detail_box);
        browse_btn->setObjectName("browse_btn");

        coverLayout->addWidget(browse_btn);


        formLayout->setLayout(14, QFormLayout::FieldRole, coverLayout);

        save_btn = new QPushButton(detail_box);
        save_btn->setObjectName("save_btn");
        save_btn->setStyleSheet(QString::fromUtf8(""));
        save_btn->setAutoDefault(false);

        formLayout->setWidget(15, QFormLayout::FieldRole, save_btn);


        gridLayout->addWidget(detail_box, 0, 1, 1, 1);

        frame1 = new QFrame(frame);
        frame1->setObjectName("frame1");
        frame1->setMaximumSize(QSize(280, 16777215));
        frame1->setFrameShape(QFrame::StyledPanel);
        frame1->setFrameShadow(QFrame::Sunken);
        leftLayout = new QVBoxLayout(frame1);
        leftLayout->setSpacing(8);
        leftLayout->setObjectName("leftLayout");
        leftLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        leftLayout->setContentsMargins(-1, -1, 1, -1);
        device_list = new QListWidget(frame1);
        device_list->setObjectName("device_list");
        device_list->setMinimumSize(QSize(0, 0));
        device_list->setMaximumSize(QSize(16777215, 16777215));

        leftLayout->addWidget(device_list);

        listButtonsLayout = new QHBoxLayout();
        listButtonsLayout->setSpacing(6);
        listButtonsLayout->setObjectName("listButtonsLayout");
        listButtonsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        add_btn = new QPushButton(frame1);
        add_btn->setObjectName("add_btn");
        add_btn->setMaximumSize(QSize(16777215, 16777215));

        listButtonsLayout->addWidget(add_btn);

        remove_btn = new QPushButton(frame1);
        remove_btn->setObjectName("remove_btn");
        remove_btn->setMaximumSize(QSize(16777215, 16777215));

        listButtonsLayout->addWidget(remove_btn);


        leftLayout->addLayout(listButtonsLayout);


        gridLayout->addWidget(frame1, 0, 0, 1, 1);


        mainLayout->addWidget(frame);


        retranslateUi(OpenRGBWallpaperEngineWidget);

        QMetaObject::connectSlotsByName(OpenRGBWallpaperEngineWidget);
    } // setupUi

    void retranslateUi(QWidget *OpenRGBWallpaperEngineWidget)
    {
        OpenRGBWallpaperEngineWidget->setWindowTitle(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Wallpaper Engine Settings", nullptr));
        detail_box->setTitle(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Device Configuration", nullptr));
        nameLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Device Name:", nullptr));
        hostLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "WPE Host IP:", nullptr));
        portLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "UDP Port:", nullptr));
        aspectLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Aspect Ratio:", nullptr));
        densityLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Grid Density:", nullptr));
        blurLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Blur Intensity:", nullptr));
        shapeLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "LED Shape:", nullptr));
        radiusLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Corner Radius:", nullptr));
        paddingLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "LED Padding:", nullptr));
        bgColorLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Background Color:", nullptr));
        sdColorLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Shutdown Color:", nullptr));
        sdEffectLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Shutdown Effect:", nullptr));
        showFpsLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Show FPS:", nullptr));
        fps_checkbox->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Show FPS on Wallpaper", nullptr));
        stretchLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Cover Stretch:", nullptr));
        coverLabel->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Cover Image:", nullptr));
        browse_btn->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Browse...", nullptr));
        save_btn->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Save Changes", nullptr));
        add_btn->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Add Device", nullptr));
        remove_btn->setText(QCoreApplication::translate("OpenRGBWallpaperEngineWidget", "Remove", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OpenRGBWallpaperEngineWidget: public Ui_OpenRGBWallpaperEngineWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPENRGBWALLPAPERENGINEWIDGET_H
