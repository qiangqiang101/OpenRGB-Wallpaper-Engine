/****************************************************************************
** Meta object code from reading C++ file 'OpenRGBWallpaperEnginePlugin.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../OpenRGBWallpaperEnginePlugin.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OpenRGBWallpaperEnginePlugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN28OpenRGBWallpaperEnginePluginE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN28OpenRGBWallpaperEnginePluginE = QtMocHelpers::stringData(
    "OpenRGBWallpaperEnginePlugin"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN28OpenRGBWallpaperEnginePluginE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject OpenRGBWallpaperEnginePlugin::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN28OpenRGBWallpaperEnginePluginE.offsetsAndSizes,
    qt_meta_data_ZN28OpenRGBWallpaperEnginePluginE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN28OpenRGBWallpaperEnginePluginE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<OpenRGBWallpaperEnginePlugin, std::true_type>
    >,
    nullptr
} };

void OpenRGBWallpaperEnginePlugin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<OpenRGBWallpaperEnginePlugin *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *OpenRGBWallpaperEnginePlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpenRGBWallpaperEnginePlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN28OpenRGBWallpaperEnginePluginE.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "OpenRGBPluginInterface"))
        return static_cast< OpenRGBPluginInterface*>(this);
    if (!strcmp(_clname, "org.openrgb.OpenRGBPluginInterface"))
        return static_cast< OpenRGBPluginInterface*>(this);
    return QObject::qt_metacast(_clname);
}

int OpenRGBWallpaperEnginePlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}

#ifdef QT_MOC_EXPORT_PLUGIN_V2
static constexpr unsigned char qt_pluginMetaDataV2_OpenRGBWallpaperEnginePlugin[] = {
    0xbf, 
    // "IID"
    0x02,  0x78,  0x22,  'o',  'r',  'g',  '.',  'o', 
    'p',  'e',  'n',  'r',  'g',  'b',  '.',  'O', 
    'p',  'e',  'n',  'R',  'G',  'B',  'P',  'l', 
    'u',  'g',  'i',  'n',  'I',  'n',  't',  'e', 
    'r',  'f',  'a',  'c',  'e', 
    // "className"
    0x03,  0x78,  0x1c,  'O',  'p',  'e',  'n',  'R', 
    'G',  'B',  'W',  'a',  'l',  'l',  'p',  'a', 
    'p',  'e',  'r',  'E',  'n',  'g',  'i',  'n', 
    'e',  'P',  'l',  'u',  'g',  'i',  'n', 
    // "MetaData"
    0x04,  0xa9,  0x66,  'C',  'o',  'm',  'm',  'i', 
    't',  0x67,  'u',  'n',  'k',  'n',  'o',  'w', 
    'n',  0x6b,  'D',  'e',  's',  'c',  'r',  'i', 
    'p',  't',  'i',  'o',  'n',  0x78,  0x25,  'C', 
    'o',  'n',  'n',  'e',  'c',  't',  ' ',  'W', 
    'a',  'l',  'l',  'p',  'a',  'p',  'e',  'r', 
    ' ',  'E',  'n',  'g',  'i',  'n',  'e',  ' ', 
    'w',  'i',  't',  'h',  ' ',  'O',  'p',  'e', 
    'n',  'R',  'G',  'B',  0x62,  'I',  'd',  0x78, 
    0x28,  'o',  'r',  'g',  '.',  'o',  'p',  'e', 
    'n',  'r',  'g',  'b',  '.',  'o',  'p',  'e', 
    'n',  'r',  'g',  'b',  'w',  'a',  'l',  'l', 
    'p',  'a',  'p',  'e',  'r',  'e',  'n',  'g', 
    'i',  'n',  'e',  'p',  'l',  'u',  'g',  'i', 
    'n',  0x64,  'N',  'a',  'm',  'e',  0x78,  0x1f, 
    'O',  'p',  'e',  'n',  'R',  'G',  'B',  ' ', 
    'W',  'a',  'l',  'l',  'p',  'a',  'p',  'e', 
    'r',  ' ',  'E',  'n',  'g',  'i',  'n',  'e', 
    ' ',  'P',  'l',  'u',  'g',  'i',  'n',  0x77, 
    'O',  'p',  'e',  'n',  'R',  'G',  'B',  'P', 
    'l',  'u',  'g',  'i',  'n',  'A',  'P',  'I', 
    'V',  'e',  'r',  's',  'i',  'o',  'n',  0x05, 
    0x63,  'U',  'r',  'l',  0x78,  0x31,  'h',  't', 
    't',  'p',  's',  ':',  '/',  '/',  'g',  'i', 
    't',  'h',  'u',  'b',  '.',  'c',  'o',  'm', 
    '/',  'W',  'e',  'P',  'r',  'o',  '/',  'O', 
    'p',  'e',  'n',  'R',  'G',  'B',  '-',  'W', 
    'a',  'l',  'l',  'p',  'a',  'p',  'e',  'r', 
    '-',  'E',  'n',  'g',  'i',  'n',  'e',  0x68, 
    'V',  'e',  'n',  'd',  'o',  'r',  'I',  'd', 
    0x67,  'o',  'p',  'e',  'n',  'r',  'g',  'b', 
    0x67,  'V',  'e',  'r',  's',  'i',  'o',  'n', 
    0x01,  0x6a,  'V',  'e',  'r',  's',  'i',  'o', 
    'n',  'S',  't',  'r',  0x65,  '1',  '.',  '0', 
    '.',  '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN_V2(OpenRGBWallpaperEnginePlugin, OpenRGBWallpaperEnginePlugin, qt_pluginMetaDataV2_OpenRGBWallpaperEnginePlugin)
#else
QT_PLUGIN_METADATA_SECTION
Q_CONSTINIT static constexpr unsigned char qt_pluginMetaData_OpenRGBWallpaperEnginePlugin[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x22,  'o',  'r',  'g',  '.',  'o', 
    'p',  'e',  'n',  'r',  'g',  'b',  '.',  'O', 
    'p',  'e',  'n',  'R',  'G',  'B',  'P',  'l', 
    'u',  'g',  'i',  'n',  'I',  'n',  't',  'e', 
    'r',  'f',  'a',  'c',  'e', 
    // "className"
    0x03,  0x78,  0x1c,  'O',  'p',  'e',  'n',  'R', 
    'G',  'B',  'W',  'a',  'l',  'l',  'p',  'a', 
    'p',  'e',  'r',  'E',  'n',  'g',  'i',  'n', 
    'e',  'P',  'l',  'u',  'g',  'i',  'n', 
    // "MetaData"
    0x04,  0xa9,  0x66,  'C',  'o',  'm',  'm',  'i', 
    't',  0x67,  'u',  'n',  'k',  'n',  'o',  'w', 
    'n',  0x6b,  'D',  'e',  's',  'c',  'r',  'i', 
    'p',  't',  'i',  'o',  'n',  0x78,  0x25,  'C', 
    'o',  'n',  'n',  'e',  'c',  't',  ' ',  'W', 
    'a',  'l',  'l',  'p',  'a',  'p',  'e',  'r', 
    ' ',  'E',  'n',  'g',  'i',  'n',  'e',  ' ', 
    'w',  'i',  't',  'h',  ' ',  'O',  'p',  'e', 
    'n',  'R',  'G',  'B',  0x62,  'I',  'd',  0x78, 
    0x28,  'o',  'r',  'g',  '.',  'o',  'p',  'e', 
    'n',  'r',  'g',  'b',  '.',  'o',  'p',  'e', 
    'n',  'r',  'g',  'b',  'w',  'a',  'l',  'l', 
    'p',  'a',  'p',  'e',  'r',  'e',  'n',  'g', 
    'i',  'n',  'e',  'p',  'l',  'u',  'g',  'i', 
    'n',  0x64,  'N',  'a',  'm',  'e',  0x78,  0x1f, 
    'O',  'p',  'e',  'n',  'R',  'G',  'B',  ' ', 
    'W',  'a',  'l',  'l',  'p',  'a',  'p',  'e', 
    'r',  ' ',  'E',  'n',  'g',  'i',  'n',  'e', 
    ' ',  'P',  'l',  'u',  'g',  'i',  'n',  0x77, 
    'O',  'p',  'e',  'n',  'R',  'G',  'B',  'P', 
    'l',  'u',  'g',  'i',  'n',  'A',  'P',  'I', 
    'V',  'e',  'r',  's',  'i',  'o',  'n',  0x05, 
    0x63,  'U',  'r',  'l',  0x78,  0x31,  'h',  't', 
    't',  'p',  's',  ':',  '/',  '/',  'g',  'i', 
    't',  'h',  'u',  'b',  '.',  'c',  'o',  'm', 
    '/',  'W',  'e',  'P',  'r',  'o',  '/',  'O', 
    'p',  'e',  'n',  'R',  'G',  'B',  '-',  'W', 
    'a',  'l',  'l',  'p',  'a',  'p',  'e',  'r', 
    '-',  'E',  'n',  'g',  'i',  'n',  'e',  0x68, 
    'V',  'e',  'n',  'd',  'o',  'r',  'I',  'd', 
    0x67,  'o',  'p',  'e',  'n',  'r',  'g',  'b', 
    0x67,  'V',  'e',  'r',  's',  'i',  'o',  'n', 
    0x01,  0x6a,  'V',  'e',  'r',  's',  'i',  'o', 
    'n',  'S',  't',  'r',  0x65,  '1',  '.',  '0', 
    '.',  '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(OpenRGBWallpaperEnginePlugin, OpenRGBWallpaperEnginePlugin)
#endif  // QT_MOC_EXPORT_PLUGIN_V2

QT_WARNING_POP
