#pragma once

#include "VContainer.h"
#include "AbstractWidgetGroup.h"
#include <QStackedWidget>

class VRouter : public VContainer, public AbstractWidgetGroup<VRouter, QWidget> {
    Q_OBJECT

    QStackedWidget *_stack;
    QWidget *_emptyWidget = nullptr;

public:
	VRouter();
	VRouter(List children);
    VRouter *Add(QWidget *w) override;

    void Show(QWidget *w);

private:
    QWidget *emptyWidget();
};
