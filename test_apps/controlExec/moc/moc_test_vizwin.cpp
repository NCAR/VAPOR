/****************************************************************************
** Meta object code from reading C++ file 'test_vizwin.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../test_vizwin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
    #error "The header file 'test_vizwin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
    #error "This file was generated using the moc from 4.8.6. It"
    #error "cannot be used with the include files from this version of Qt."
    #error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_VAPoR__Test_VizWin[] = {

    // content:
    6,       // revision
    0,       // classname
    0, 0,    // classinfo
    0, 0,    // methods
    0, 0,    // properties
    0, 0,    // enums/sets
    0, 0,    // constructors
    0,       // flags
    0,       // signalCount

    0    // eod
};

static const char qt_meta_stringdata_VAPoR__Test_VizWin[] = {"VAPoR::Test_VizWin\0"};

void VAPoR::Test_VizWin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData VAPoR::Test_VizWin::staticMetaObjectExtraData = {0, qt_static_metacall};

const QMetaObject VAPoR::Test_VizWin::staticMetaObject = {{&QGLWidget::staticMetaObject, qt_meta_stringdata_VAPoR__Test_VizWin, qt_meta_data_VAPoR__Test_VizWin, &staticMetaObjectExtraData}};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &VAPoR::Test_VizWin::getStaticMetaObject() { return staticMetaObject; }
#endif    // Q_NO_DATA_RELOCATION

const QMetaObject *VAPoR::Test_VizWin::metaObject() const { return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject; }

void *VAPoR::Test_VizWin::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_VAPoR__Test_VizWin)) return static_cast<void *>(const_cast<Test_VizWin *>(this));
    return QGLWidget::qt_metacast(_clname);
}

int VAPoR::Test_VizWin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0) return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
