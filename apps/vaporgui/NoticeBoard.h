#pragma once

#include <QDialog>

struct Notice;
class QTextBrowser;
class QLabel;

class NoticeBoard : public QDialog {
    bool                      _wasDisableCheckingRequested = false;
    const std::vector<Notice> _notices;
    int                       _displayedNotice;

    QTextBrowser *_browser;
    QLabel *      _numberDisplay;

    void showNotice(int i);

public:
    NoticeBoard(const std::vector<Notice> &notices);
    bool WasDisableCheckingRequested() const { return _wasDisableCheckingRequested; }
};
