#ifndef FOO_H
#define FOO_H

#include <QWidget>

namespace Ui {
class foo;
}

class foo : public QWidget {
    Q_OBJECT

public:
    explicit foo(QWidget *parent = 0);
    ~foo();

private:
    Ui::foo *ui;
};

#endif    // FOO_H
