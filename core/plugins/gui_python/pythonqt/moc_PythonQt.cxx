/****************************************************************************
** Meta object code from reading C++ file 'PythonQt.h'
**
** Created: Wed Nov 21 16:32:31 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "PythonQt.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PythonQt.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PythonQt[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   10,    9,    9, 0x05,
      36,   10,    9,    9, 0x05,
      71,   58,    9,    9, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_PythonQt[] = {
    "PythonQt\0\0str\0pythonStdOut(QString)\0"
    "pythonStdErr(QString)\0cppClassName\0"
    "pythonHelpRequest(QByteArray)\0"
};

void PythonQt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PythonQt *_t = static_cast<PythonQt *>(_o);
        switch (_id) {
        case 0: _t->pythonStdOut((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->pythonStdErr((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 2: _t->pythonHelpRequest((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PythonQt::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PythonQt::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PythonQt,
      qt_meta_data_PythonQt, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PythonQt::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PythonQt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PythonQt::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PythonQt))
        return static_cast<void*>(const_cast< PythonQt*>(this));
    return QObject::qt_metacast(_clname);
}

int PythonQt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void PythonQt::pythonStdOut(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PythonQt::pythonStdErr(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PythonQt::pythonHelpRequest(const QByteArray & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_PythonQtPrivate[] = {

 // content:
       6,       // revision
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

static const char qt_meta_stringdata_PythonQtPrivate[] = {
    "PythonQtPrivate\0"
};

void PythonQtPrivate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData PythonQtPrivate::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PythonQtPrivate::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_PythonQtPrivate,
      qt_meta_data_PythonQtPrivate, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PythonQtPrivate::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PythonQtPrivate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PythonQtPrivate::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PythonQtPrivate))
        return static_cast<void*>(const_cast< PythonQtPrivate*>(this));
    return QObject::qt_metacast(_clname);
}

int PythonQtPrivate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
