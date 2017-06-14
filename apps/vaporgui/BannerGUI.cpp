
#include <QTranslator>
#include <qboxlayout.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include "MainForm.h"
#include <QDesktopServices>
#include <QUrl>
#include <vapor/GetAppPath.h>
#include "BannerGUI.h"

BannerGUI::BannerGUI(std::string imagefile, int maxwait, bool center, QString text, QString url)
{
    this->url = url;

    if (maxwait >= 0) this->setWindowFlags(Qt::FramelessWindowHint);
    if (maxwait > 0) QTimer::singleShot(maxwait, this, SLOT(on_timer_end()));

    central = new QWidget();
    setCentralWidget(central);

    mainLayout = new QVBoxLayout();
    central->setLayout(mainLayout);
    buttonLayout = new QHBoxLayout();

    textLabel = new QLabel(text, this);
    textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    imageLabel = new QLabel("", this);
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setScaledContents(true);

    infoButton = new QPushButton("More Info");
    connect(infoButton, SIGNAL(released()), this, SLOT(infoButton_action()));

    closeButton = new QPushButton("Close");
    connect(closeButton, SIGNAL(released()), this, SLOT(closeButton_action()));

    if (!imagefile.empty()) {
        std::vector<std::string> vec = std::vector<std::string>();
        vec.push_back("images");
        vec.push_back(imagefile);
        QImage image(GetAppPath("VAPOR", "share", vec).c_str());
        // QImage image(imagefile.c_str());
        if (image.isNull()) {
            QMessageBox::information(this, tr("VAPoR Banner"), tr("Could not load banner image.\n"));
        } else {
            imageLabel->setPixmap(QPixmap::fromImage(image));
            // setCentralWidget(imageLabel);
            if (text.compare("") != 0) mainLayout->addWidget(textLabel);
            mainLayout->addWidget(imageLabel);
            if (maxwait < 0 || url.compare("") != 0) mainLayout->addLayout(buttonLayout);
            if (url.compare("") != 0) buttonLayout->addWidget(infoButton);
            if (maxwait < 0) buttonLayout->addWidget(closeButton);
            // imageLabel->resize(image.size());
            /*
            QRect position = frameGeometry();
            position.moveCenter(MainForm::getInstance()->pos());
            move(position.center() - image.size());
            */
            // move(MainForm::getInstance()->rect().center().x() - (image.width() / 2), MainForm::getInstance()->rect().center().y() - (image.width() / 2));
            // I will assume the problem is my high-res monitor or something

            if (center) {
                MainForm *mf = MainForm::getInstance();
                QPoint    mpos = mf->pos();
                move(mpos.x() + (mf->width() / 2) - (image.width() / 2), mpos.y() + (mf->height() / 2) - (image.height() / 2));
            } else {
                QRect screenGeometry = QApplication::desktop()->screenGeometry();
                int   x = (screenGeometry.width() - image.size().width()) / 2;
                int   y = (screenGeometry.height() - image.size().height()) / 2;
                move(x, y);
            }
            mainLayout->setSizeConstraint(QLayout::SetFixedSize);
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            show();
        }
    }
}

void BannerGUI::request_close() { close(); }

void BannerGUI::on_timer_end() { close(); }

void BannerGUI::infoButton_action()
{
    // TODO: THIS APPEARS NOT TO WORK. FIND OUT HOW OTHERS DO IT.
    QDesktopServices::openUrl(QUrl(url));
}

void BannerGUI::closeButton_action() { close(); }
