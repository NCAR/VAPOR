#include <iostream>
//#include <vapor/MyBase.h>

#include <QApplication>
#include <QLabel>

int main(int argc, char **argv)
{
    std::cout << "testApp" << std::endl;
    //std::cout << Wasp::MyBase::GetErrMsg() << std::endl;

    QApplication app(argc, argv);

    QLabel label("testApp Label");
    label.show();

    return app.exec();

    return (0);
}
