#ifndef BANNERGUI_H
#define BANNERGUI_H

#include <qmainwindow.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qpushbutton.h>

class BannerGUI : QMainWindow {
    Q_OBJECT
public:
    BannerGUI(std::string imagefile, int maxwait = 0, bool center = false, QString text = "", QString url = "");

    void request_close();

private:
    QWidget *    central;
    QVBoxLayout *mainLayout;
    QLabel *     textLabel;
    QLabel *     imageLabel;
    QHBoxLayout *buttonLayout;
    QPushButton *closeButton;
    QPushButton *infoButton;
    QString      url;

private slots:
    void on_timer_end();
    void infoButton_action();
    void closeButton_action();
};

#endif    // BANNERGUI_H
