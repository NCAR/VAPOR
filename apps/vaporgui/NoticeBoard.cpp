#include "NoticeBoard.h"
#include "CheckForNotices.h"
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QLabel>
#include <QCheckBox>
#include <cassert>
#include "VPushButton.h"
#include "VGroup.h"

NoticeBoard::NoticeBoard(const std::vector<Notice> &notices) : _notices(notices)
{
    setWindowTitle("Notice Board");
    setBaseSize(500, 500);
    auto *layout = new QVBoxLayout;
    setLayout(layout);

    _browser = new QTextBrowser;
    layout->addWidget(_browser);
    _browser->setOpenExternalLinks(true);

    auto *check = new QCheckBox("Don't check for notices on startup");
    layout->addWidget(check);
    connect(check, &QCheckBox::stateChanged, this, [this](int b) { this->_wasDisableCheckingRequested = b; });

    // Qt provides a "QDialogButtonBox" but it is buggy.
    // For example the following:
    // auto *disableButton = box->addButton("Check", QDialogButtonBox::RejectRole);
    // box->addButton(QDialogButtonBox::StandardButton::Close);
    // box->addButton("<", QDialogButtonBox::YesRole);
    // box->addButton(">", QDialogButtonBox::YesRole);
    // Results in:
    // [*Close*] [Check] [>] [<]
    // While if you swap the first 2 lines you get:
    // [*Check*] [Close] [>] [<]
    // While in itself, the above behaviour is unintuative and confusing,
    // the kicker is that the behavior is not consistent between systems.

    auto *buttonsWidget = new QWidget;
    auto *buttons = new QHBoxLayout;
    buttons->setMargin(0);
    buttonsWidget->setLayout(buttons);
    layout->addWidget(buttonsWidget);

    auto *prev = new QPushButton("<");
    auto *disp = new QLabel("1/2");
    auto *next = new QPushButton(">");
    auto *clos = new QPushButton("Close");
    clos->setDefault(true);
    _numberDisplay = disp;

    buttons->addWidget(prev);
    buttons->addWidget(disp);
    buttons->addWidget(next);
    buttons->addStretch();
    buttons->addWidget(clos);

    connect(clos, &QPushButton::clicked, this, &QWidget::close);
    connect(prev, &QPushButton::clicked, this, [this]() { showNotice(_displayedNotice - 1); });
    connect(next, &QPushButton::clicked, this, [this]() { showNotice(_displayedNotice + 1); });

    if (notices.size() == 1) {
        prev->hide();
        disp->hide();
        next->hide();
    }

    showNotice(0);
}


void NoticeBoard::showNotice(int i)
{
    if (i < 0) i = _notices.size() - 1;
    if (i >= _notices.size()) i = 0;

    _browser->setText(QString::fromStdString(_notices[i].content));
    _numberDisplay->setText(QString("%1/%2").arg(i + 1).arg(_notices.size()));
    _displayedNotice = i;
}


#include <vapor/ControlExecutive.h>
#include <vapor/SettingsParams.h>


void NoticeBoard::CheckForAndShowNotices(ControlExec *ce)
{
#ifndef NDEBUG
    return;    // Don't check for notices in debug builds
#endif

    CheckForGHNotices([ce](const std::vector<Notice> &notices) {
        if (notices.empty()) return;

        NoticeBoard board(notices);
        board.exec();

        if (board.WasDisableCheckingRequested()) {
            ce->GetParams<SettingsParams>()->SetAutoCheckForNotices(false);
            ce->GetParams<SettingsParams>()->SaveSettings();
        }
    });
}
