#pragma once

#include <QWidget>

class VContainer : public QWidget {
public:
    VContainer(QWidget *w);
    void AddBottomStretch();
    void SetPadding(int left, int top, int right, int bottom);

    QLayout *layout() const = delete;
    void     setLayout(QLayout *) = delete;
};
