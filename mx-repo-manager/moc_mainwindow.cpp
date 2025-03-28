/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "mainwindow.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MainWindow_t {
    QByteArrayData data[21];
    char stringdata0[310];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MainWindow"
QT_MOC_LITERAL(1, 11, 15), // "cancelOperation"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 10), // "closeEvent"
QT_MOC_LITERAL(4, 39, 12), // "QCloseEvent*"
QT_MOC_LITERAL(5, 52, 8), // "procDone"
QT_MOC_LITERAL(6, 61, 8), // "procTime"
QT_MOC_LITERAL(7, 70, 9), // "procStart"
QT_MOC_LITERAL(8, 80, 22), // "lineSearch_textChanged"
QT_MOC_LITERAL(9, 103, 4), // "arg1"
QT_MOC_LITERAL(10, 108, 26), // "pushRestoreSources_clicked"
QT_MOC_LITERAL(11, 135, 17), // "pushAbout_clicked"
QT_MOC_LITERAL(12, 153, 25), // "pushFastestDebian_clicked"
QT_MOC_LITERAL(13, 179, 21), // "pushFastestMX_clicked"
QT_MOC_LITERAL(14, 201, 16), // "pushHelp_clicked"
QT_MOC_LITERAL(15, 218, 14), // "pushOk_clicked"
QT_MOC_LITERAL(16, 233, 24), // "tabWidget_currentChanged"
QT_MOC_LITERAL(17, 258, 22), // "treeWidget_itemChanged"
QT_MOC_LITERAL(18, 281, 16), // "QTreeWidgetItem*"
QT_MOC_LITERAL(19, 298, 4), // "item"
QT_MOC_LITERAL(20, 303, 6) // "column"

    },
    "MainWindow\0cancelOperation\0\0closeEvent\0"
    "QCloseEvent*\0procDone\0procTime\0procStart\0"
    "lineSearch_textChanged\0arg1\0"
    "pushRestoreSources_clicked\0pushAbout_clicked\0"
    "pushFastestDebian_clicked\0"
    "pushFastestMX_clicked\0pushHelp_clicked\0"
    "pushOk_clicked\0tabWidget_currentChanged\0"
    "treeWidget_itemChanged\0QTreeWidgetItem*\0"
    "item\0column"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MainWindow[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   84,    2, 0x08 /* Private */,
       3,    1,   85,    2, 0x08 /* Private */,
       5,    0,   88,    2, 0x08 /* Private */,
       6,    0,   89,    2, 0x08 /* Private */,
       7,    0,   90,    2, 0x08 /* Private */,
       8,    1,   91,    2, 0x08 /* Private */,
      10,    0,   94,    2, 0x08 /* Private */,
      11,    0,   95,    2, 0x08 /* Private */,
      12,    0,   96,    2, 0x08 /* Private */,
      13,    0,   97,    2, 0x08 /* Private */,
      14,    0,   98,    2, 0x08 /* Private */,
      15,    0,   99,    2, 0x08 /* Private */,
      16,    0,  100,    2, 0x08 /* Private */,
      17,    2,  101,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 18, QMetaType::Int,   19,   20,

       0        // eod
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->cancelOperation(); break;
        case 1: _t->closeEvent((*reinterpret_cast< QCloseEvent*(*)>(_a[1]))); break;
        case 2: _t->procDone(); break;
        case 3: _t->procTime(); break;
        case 4: _t->procStart(); break;
        case 5: _t->lineSearch_textChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: _t->pushRestoreSources_clicked(); break;
        case 7: _t->pushAbout_clicked(); break;
        case 8: _t->pushFastestDebian_clicked(); break;
        case 9: _t->pushFastestMX_clicked(); break;
        case 10: _t->pushHelp_clicked(); break;
        case 11: _t->pushOk_clicked(); break;
        case 12: _t->tabWidget_currentChanged(); break;
        case 13: _t->treeWidget_itemChanged((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.data,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
