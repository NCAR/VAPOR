#include "TFHistogramWidget.h"
#include "TFHistogramInfoWidget.h"
#include <QPaintEvent>
#include <vapor/DataMgr.h>
#include <QPainter>
#include <QPicture>
#include <glm/glm.hpp>
#include <Histo.h>
#include "ErrorReporter.h"
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <math.h>
#include "QPaintUtils.h"
#include "ParamsMenuItems.h"

using namespace VAPoR;
using glm::clamp;
using glm::vec2;
using std::vector;

#define SCALING_TAG "HistogramScalingTag"

TFHistogramMap::TFHistogramMap(TFMapWidget *parent) : TFMap(parent) { _scalingMenu = new ParamsDropdownMenuItem(this, SCALING_TAG, {"Linear", "Logarithmic", "Boolean"}, {}, "Histogram Scaling"); }

QSize TFHistogramMap::minimumSizeHint() const
{
    //    return QSize(100, 40);
    return QSize(100, 75);
}

void TFHistogramMap::PopulateSettingsMenu(QMenu *menu) const
{
    menu->addAction(_scalingMenu);

    QAction *histogramDynamicScalingAction = menu->addAction("Histogram Dynamic Scaling", this, SLOT(_menuDynamicScalingToggled(bool)));
    histogramDynamicScalingAction->setCheckable(true);
    histogramDynamicScalingAction->setChecked(_dynamicScaling);
}

void TFHistogramMap::paramsUpdate()
{
    if (_histo.getNumBins() != width() || _histo.getNumBins() == 0) _histo.reset(width() ? width() : 256);    // This can be called before it is resized
    if (_histo.PopulateIfNeeded(getVariableName(), getDataMgr(), getRenderParams()) < 0) MSG_ERR("Failed to populate histogram");

    _scalingMenu->Update(getRenderParams());
    update();
}

TFInfoWidget *TFHistogramMap::createInfoWidget()
{
    TFHistogramInfoWidget *info = new TFHistogramInfoWidget;
    info->UsingColormapVariable = this->UsingColormapVariable;

    connect(this, SIGNAL(UpdateInfo(float)), info, SLOT(SetControlPoint(float)));
    //    connect(this, SIGNAL(InfoDeselected()), info, SLOT(Deselect()));

    return info;
}

void TFHistogramMap::paintEvent(QPainter &p)
{
    if (width() == 0) return;

    p.fillRect(rect(), Qt::lightGray);

    QPolygonF graph;
    graph.push_back(NDCToQPixel(0, 0));

    vector<double> mapRange = getRenderParams()->GetMapperFunc(getVariableName())->getMinMaxMapValue();

    int      startBin = _histo.getBinIndexForValue(mapRange[0]);
    int      endBin = _histo.getBinIndexForValue(mapRange[1]);
    int      stride = 1;
    QMargins padding = GetPadding();
    while ((endBin - startBin) / stride >= 2 * (width() - (padding.left() + padding.right()))) stride *= 2;
    startBin -= startBin % stride;

    float maxBin;
    if (_dynamicScaling)
        maxBin = _histo.getMaxBinSizeBetweenIndices(startBin, endBin);
    else
        maxBin = _histo.getMaxBinSize();

    const float logMaxBin = maxBin == 1 ? 1 : logf(maxBin);
    ScalingType scaling = _getScalingType();

    for (int i = startBin; i < endBin; i += stride) {
        float bin = _histo.getBinSize(i, stride);

        switch (scaling) {
        default:
        case ScalingType::Linear: bin /= maxBin; break;
        case ScalingType::Logarithmic: bin = bin == 0 ? 0 : logf(bin) / logMaxBin; break;
        case ScalingType::Boolean: bin = bin > 0 ? 1 : 0; break;
        }

        graph.push_back(NDCToQPixel((i - startBin) / (float)(endBin - startBin), bin));
    }

    graph.push_back(NDCToQPixel(1, 0));

    QPicture picture;
    QPainter pp(&picture);
    pp.setRenderHint(QPainter::Antialiasing);

    pp.setPen(Qt::NoPen);
    pp.setBrush(QBrush(QColor(56, 128, 255)));
    pp.drawPolygon(graph);
    pp.end();
    QPaintUtils::DropShadow(p, picture, 10, QColor(56, 128, 255, 150));
    p.drawPicture(0, 0, picture);
    QPaintUtils::InnerShadow(p, picture, 10, QColor(0, 0, 0, 100));
}

void TFHistogramMap::mousePressEvent(QMouseEvent *event)
{
    emit Activated(this);
    emit UpdateInfo(PixelToNDC(event->pos()).x);
}

void TFHistogramMap::mouseReleaseEvent(QMouseEvent *event)
{
    //    emit InfoDeselected();
}

void TFHistogramMap::mouseMoveEvent(QMouseEvent *event) { emit UpdateInfo(PixelToNDC(event->pos()).x); }

// void TFHistogramWidget::mouseDoubleClickEvent(QMouseEvent *event)

TFHistogramMap::ScalingType TFHistogramMap::_getScalingType() const
{
    if (!getRenderParams()) return ScalingType::Linear;

    ScalingType type = (ScalingType)getRenderParams()->GetValueLong(SCALING_TAG, (int)ScalingType::Linear);
    if (type >= ScalingTypeCount || type < 0) type = Linear;

    return type;
}

void TFHistogramMap::_menuDynamicScalingToggled(bool on) { _dynamicScaling = on; }
