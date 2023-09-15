
#include <QTranslator>
#include <qboxlayout.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QDesktopServices>
#include <QUrl>
#include <QScreen>
#include <vapor/ResourcePath.h>
#include <vapor/VAssert.h>
#include "BannerGUI.h"

BannerGUI::BannerGUI(QWidget *parent, std::string imagefile, int maxwait, bool center, QString text, QString url)
{
    _parent = parent;
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
        QImage image(Wasp::GetSharePath("images/" + imagefile).c_str());
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

            if (center) {
                QPoint mpos = parent->pos();
                move(mpos.x() + (this->width() / 2) - (image.width() / 2), mpos.y() + (this->height() / 2) - (image.height() / 2));
            } else {
                QScreen *screen = QGuiApplication::primaryScreen();
                QRect    screenGeometry = screen->geometry();

                int x = (screenGeometry.width() - image.size().width()) / 2;
                int y = (screenGeometry.height() - image.size().height()) / 2;
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
