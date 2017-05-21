/****************************************************************************
** Meta object code from reading C++ file 'CQTextFileCanvas.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../include/CQTextFileCanvas.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CQTextFileCanvas.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CQTextFileCanvas_t {
    QByteArrayData data[5];
    char stringdata0[40];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CQTextFileCanvas_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CQTextFileCanvas_t qt_meta_stringdata_CQTextFileCanvas = {
    {
QT_MOC_LITERAL(0, 0, 16), // "CQTextFileCanvas"
QT_MOC_LITERAL(1, 17, 11), // "sizeChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 4), // "rows"
QT_MOC_LITERAL(4, 35, 4) // "cols"

    },
    "CQTextFileCanvas\0sizeChanged\0\0rows\0"
    "cols"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CQTextFileCanvas[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,

       0        // eod
};

void CQTextFileCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CQTextFileCanvas *_t = static_cast<CQTextFileCanvas *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sizeChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CQTextFileCanvas::*_t)(int , int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CQTextFileCanvas::sizeChanged)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject CQTextFileCanvas::staticMetaObject = {
    { &CQWindow::staticMetaObject, qt_meta_stringdata_CQTextFileCanvas.data,
      qt_meta_data_CQTextFileCanvas,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CQTextFileCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CQTextFileCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CQTextFileCanvas.stringdata0))
        return static_cast<void*>(const_cast< CQTextFileCanvas*>(this));
    if (!strcmp(_clname, "CTextFileNotifier"))
        return static_cast< CTextFileNotifier*>(const_cast< CQTextFileCanvas*>(this));
    return CQWindow::qt_metacast(_clname);
}

int CQTextFileCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CQWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void CQTextFileCanvas::sizeChanged(int _t1, int _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
