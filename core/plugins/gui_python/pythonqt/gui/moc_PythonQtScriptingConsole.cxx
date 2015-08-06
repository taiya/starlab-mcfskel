/****************************************************************************
** Meta object code from reading C++ file 'PythonQtScriptingConsole.h'
**
** Created: Wed Nov 21 16:32:31 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "PythonQtScriptingConsole.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PythonQtScriptingConsole.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_PythonQtScriptingConsole[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      36,   26,   25,   25, 0x0a,
      56,   54,   25,   25, 0x0a,
      90,   82,   25,   25, 0x0a,
     126,   25,  114,   25, 0x0a,
     138,  136,   25,   25, 0x0a,
     162,   25,   25,   25, 0x0a,
     170,   25,   25,   25, 0x0a,
     178,  176,   25,   25, 0x0a,
     194,  176,   25,   25, 0x0a,
     210,   25,   25,   25, 0x0a,
     236,   26,   25,   25, 0x0a,
     262,   25,   25,   25, 0x2a,

       0        // eod
};

static const char qt_meta_stringdata_PythonQtScriptingConsole[] = {
    "PythonQtScriptingConsole\0\0storeOnly\0"
    "executeLine(bool)\0e\0keyPressEvent(QKeyEvent*)\0"
    "message\0consoleMessage(QString)\0"
    "QStringList\0history()\0h\0setHistory(QStringList)\0"
    "clear()\0cut()\0s\0stdOut(QString)\0"
    "stdErr(QString)\0insertCompletion(QString)\0"
    "appendCommandPrompt(bool)\0"
    "appendCommandPrompt()\0"
};

void PythonQtScriptingConsole::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        PythonQtScriptingConsole *_t = static_cast<PythonQtScriptingConsole *>(_o);
        switch (_id) {
        case 0: _t->executeLine((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->keyPressEvent((*reinterpret_cast< QKeyEvent*(*)>(_a[1]))); break;
        case 2: _t->consoleMessage((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: { QStringList _r = _t->history();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = _r; }  break;
        case 4: _t->setHistory((*reinterpret_cast< const QStringList(*)>(_a[1]))); break;
        case 5: _t->clear(); break;
        case 6: _t->cut(); break;
        case 7: _t->stdOut((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->stdErr((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->insertCompletion((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 10: _t->appendCommandPrompt((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->appendCommandPrompt(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData PythonQtScriptingConsole::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject PythonQtScriptingConsole::staticMetaObject = {
    { &QTextEdit::staticMetaObject, qt_meta_stringdata_PythonQtScriptingConsole,
      qt_meta_data_PythonQtScriptingConsole, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &PythonQtScriptingConsole::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *PythonQtScriptingConsole::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *PythonQtScriptingConsole::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_PythonQtScriptingConsole))
        return static_cast<void*>(const_cast< PythonQtScriptingConsole*>(this));
    return QTextEdit::qt_metacast(_clname);
}

int PythonQtScriptingConsole::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
