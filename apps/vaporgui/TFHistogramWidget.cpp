#include "TFHistogramWidget.h"
#include "TFHistogramInfoWidget.h"
#include <QPaintEvent>
#include <vapor/DataMgr.h>
#include <QPainter>
#include <glm/glm.hpp>
#include <Histo.h>
#include "ErrorReporter.h"
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <math.h>
#include "QPaintUtils.h"

using namespace VAPoR;
using std::vector;
using glm::vec2;
using glm::clamp;

static vec2 qvec2(const QPoint &qp)  { return vec2(qp.x(), qp.y()); }
static vec2 qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }


TFHistogramMap::TFHistogramMap(TFMapWidget *parent)
: TFMap(parent) {}

void TFHistogramMap::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    _dataMgr = dataMgr;
    
    if (_histo.getNumBins() != width())
        _histo.reset(width());
    if (_histo.PopulateIfNeeded(dataMgr, rp) < 0)
        MSG_ERR("Failed to populate histogram");
    
    update();
}

QSize TFHistogramMap::minimumSizeHint() const
{
    return QSize(100, 40);
}

TFInfoWidget *TFHistogramMap::createInfoWidget()
{
    TFHistogramInfoWidget *info = new TFHistogramInfoWidget;
    
    connect(this, SIGNAL(UpdateInfo(float)), info, SLOT(SetControlPoint(float)));
//    connect(this, SIGNAL(InfoDeselected()), info, SLOT(Deselect()));
    
    return info;
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

#include <QPicture>

void TFHistogramMap::paintEvent(QPainter &p)
{
    p.fillRect(rect(), Qt::lightGray);
    
//    QMatrix m;
//    m.translate(PADDING, height());
//    m.scale((width()-PADDING*2)/(float)_histo.getNumBins(), -1);
//    p.setMatrix(m);
    
    QPolygonF graph;
    graph.push_back(NDCToQPixel(0,0));
    
    vector<double> mapRange = _renderParams->GetMapperFunc(_renderParams->GetVariableName())->getMinMaxMapValue();
    
    int startBin = _histo.getBinIndexForValue(mapRange[0]);
    int endBin = _histo.getBinIndexForValue(mapRange[1]);
    int stride = 1;
    QMargins padding = GetPadding();
    while ((endBin - startBin)/stride >= 2 * (width() - (padding.left()+padding.right())))
        stride *= 2;
    
    float maxBin;
    if (DynamicScaling)
        maxBin = _histo.getMaxBinSizeBetweenIndices(startBin, endBin);
    else
        maxBin = _histo.getMaxBinSize();
    
    for (int i = startBin; i < endBin; i += stride) {
        float bin = _histo.getBinSize(i) / maxBin;
        
        graph.push_back(NDCToQPixel((i-startBin)/(float)(endBin-startBin), bin));
    }
    
    graph.push_back(NDCToQPixel(1,0));
    
    
    QPicture picture;
    QPainter pp(&picture);
    pp.setRenderHint(QPainter::Antialiasing);
    
    pp.setPen(Qt::NoPen);
    pp.setBrush(QBrush(QColor(56, 128, 255)));
    pp.drawPolygon(graph);
    pp.end();
    QPaintUtils::DropShadow(p, picture, 10, QColor(56,128,255, 150));
    p.drawPicture(0, 0, picture);
    QPaintUtils::InnerShadow(p, picture, 10, QColor(0,0,0, 100));
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

void TFHistogramMap::mouseMoveEvent(QMouseEvent *event)
{
    emit UpdateInfo(PixelToNDC(event->pos()).x);
}

//void TFHistogramWidget::mouseDoubleClickEvent(QMouseEvent *event)
