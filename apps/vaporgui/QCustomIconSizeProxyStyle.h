#pragma once

#include <QProxyStyle>

class QCustomIconSizeProxyStyle : public QProxyStyle {
    const int _size;

public:
    QCustomIconSizeProxyStyle(int size) : _size(size) {}
    virtual int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override
    {
        if (metric == PM_SmallIconSize)
            return _size;
        else
            return QCommonStyle::pixelMetric(metric, option, widget);
    }
};
