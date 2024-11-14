#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>


class ProgressStatusBar : public QWidget {
    QLabel *      _titleLabel = new QLabel;
    QProgressBar *_progressBar = new QProgressBar;
    QToolButton * _cancelButton = new QToolButton;

    bool _canceled = false;

public:
    ProgressStatusBar()
    {
        QHBoxLayout *layout = new QHBoxLayout;
        layout->setMargin(4);
        setLayout(layout);

        _cancelButton->setIcon(_cancelButton->style()->standardIcon(QStyle::StandardPixmap::SP_DialogCancelButton));
        QObject::connect(_cancelButton, &QAbstractButton::clicked, this, [this]() {
            _canceled = true;
            Finish();
            SetTitle("Cancelled.");
        });

        QSizePolicy sp = _cancelButton->sizePolicy();
        sp.setRetainSizeWhenHidden(true);
        _cancelButton->setSizePolicy(sp);
        _cancelButton->setIconSize(_cancelButton->iconSize() * 0.7);
        _cancelButton->setToolTip("Cancel");

        layout->addWidget(_titleLabel);
        layout->addWidget(_progressBar);
        layout->addWidget(_cancelButton);

        Finish();
    }
    void SetTitle(const string &title) { _titleLabel->setText(QString::fromStdString(title)); }
    void SetTotal(long total) { _progressBar->setRange(0, total); }
    void SetCancelable(bool b) { _cancelButton->setEnabled(b); }
    void SetDone(long done) { _progressBar->setValue(done); }
    bool Cancelled() { return _canceled; }
    void StartTask(const string &title, long total, bool cancelable)
    {
        Reset();
        SetTitle(title);
        SetTotal(total);
        SetCancelable(cancelable);
        _progressBar->show();
        _cancelButton->show();
    }
    void Finish()
    {
        _progressBar->hide();
        _cancelButton->hide();
        SetTitle("");
    }
    void Reset()
    {
        _canceled = false;
        _progressBar->reset();
    }
    const QObject *GetCancelButtonObject() const { return _cancelButton; }
};
