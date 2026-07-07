/****************************************************************************
** Meta object code from reading C++ file 'OpenRGBWallpaperEngineWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../OpenRGBWallpaperEngineWidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OpenRGBWallpaperEngineWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_OpenRGBWallpaperEngineWidget_t {
    QByteArrayData data[10];
    char stringdata0[145];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_OpenRGBWallpaperEngineWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_OpenRGBWallpaperEngineWidget_t qt_meta_stringdata_OpenRGBWallpaperEngineWidget = {
    {
QT_MOC_LITERAL(0, 0, 28), // "OpenRGBWallpaperEngineWidget"
QT_MOC_LITERAL(1, 29, 11), // "OnAddDevice"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 14), // "OnRemoveDevice"
QT_MOC_LITERAL(4, 57, 16), // "OnDeviceSelected"
QT_MOC_LITERAL(5, 74, 5), // "index"
QT_MOC_LITERAL(6, 80, 15), // "OnSelectBgColor"
QT_MOC_LITERAL(7, 96, 15), // "OnSelectSdColor"
QT_MOC_LITERAL(8, 112, 18), // "OnBrowseCoverImage"
QT_MOC_LITERAL(9, 131, 13) // "OnSaveChanges"

    },
    "OpenRGBWallpaperEngineWidget\0OnAddDevice\0"
    "\0OnRemoveDevice\0OnDeviceSelected\0index\0"
    "OnSelectBgColor\0OnSelectSdColor\0"
    "OnBrowseCoverImage\0OnSaveChanges"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_OpenRGBWallpaperEngineWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    1,   51,    2, 0x08 /* Private */,
       6,    0,   54,    2, 0x08 /* Private */,
       7,    0,   55,    2, 0x08 /* Private */,
       8,    0,   56,    2, 0x08 /* Private */,
       9,    0,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void OpenRGBWallpaperEngineWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<OpenRGBWallpaperEngineWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->OnAddDevice(); break;
        case 1: _t->OnRemoveDevice(); break;
        case 2: _t->OnDeviceSelected((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->OnSelectBgColor(); break;
        case 4: _t->OnSelectSdColor(); break;
        case 5: _t->OnBrowseCoverImage(); break;
        case 6: _t->OnSaveChanges(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject OpenRGBWallpaperEngineWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_OpenRGBWallpaperEngineWidget.data,
    qt_meta_data_OpenRGBWallpaperEngineWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *OpenRGBWallpaperEngineWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OpenRGBWallpaperEngineWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OpenRGBWallpaperEngineWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int OpenRGBWallpaperEngineWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
