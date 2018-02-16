#include "foo.h"
#include "ui_foo.h"

foo::foo(QWidget *parent) : QWidget(parent), ui(new Ui::foo) { ui->setupUi(this); }

foo::~foo() { delete ui; }
