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

using namespace VAPoR;
using std::vector;
using glm::vec2;
using glm::clamp;

static vec2 qvec2(const QPoint &qp)  { return vec2(qp.x(), qp.y()); }
static vec2 qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }


TFHistogramWidget::TFHistogramWidget()
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setFrameStyle(QFrame::Box);
}

void TFHistogramWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    _dataMgr = dataMgr;
    
    if (_histo.getNumBins() != width())
        _histo.reset(width());
    if (_histo.PopulateIfNeeded(dataMgr, rp) < 0)
        MSG_ERR("Failed to populate histogram");
}

QSize TFHistogramWidget::minimumSizeHint() const
{
    return QSize(100, 30);
}

TFInfoWidget *TFHistogramWidget::createInfoWidget()
{
    TFHistogramInfoWidget *info = new TFHistogramInfoWidget;
    
    return info;
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

void TFHistogramWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QMatrix m;
    m.translate(0, height());
    m.scale(width()/(float)_histo.getNumBins(), -1);
    p.setMatrix(m);
    
    p.fillRect(rect(), Qt::gray);
    
    QPolygonF graph;
    graph.push_back(QPointF(0,0));
    
    for (int i = 0; i < _histo.getNumBins(); i++) {
        float bin = _histo.getBinSizeNormalized(i)*height();
        graph.push_back(QPointF(i, bin));
        graph.push_back(QPointF(i, bin));
    }
    
    graph.push_back(QPointF(_histo.getNumBins()-1,0));
    
    p.setBrush(QBrush(Qt::black));
    p.drawPolygon(graph);
}

void TFHistogramWidget::mousePressEvent(QMouseEvent *event)
{
    emit Activated(this);
}

//void TFHistogramWidget::mouseReleaseEvent(QMouseEvent *event)
//void TFHistogramWidget::mouseMoveEvent(QMouseEvent *event)
//void TFHistogramWidget::mouseDoubleClickEvent(QMouseEvent *event)

glm::vec2 TFHistogramWidget::NDCToPixel(const glm::vec2 &v) const
{
    return vec2(PADDING + v.x * (width()-2*PADDING), PADDING + (1.0f - v.y) * (height()-2*PADDING));
}

QPointF TFHistogramWidget::QNDCToPixel(const glm::vec2 &v) const
{
    const vec2 p = NDCToPixel(v);
    return QPointF(p.x, p.y);
}

glm::vec2 TFHistogramWidget::PixelToNDC(const glm::vec2 &p) const
{
    float width = QWidget::width();
    float height = QWidget::height();
    VAssert(width != 0 && height != 0);
    
    return vec2((p.x - PADDING) / (width-2*PADDING), 1.0f - (p.y - PADDING) / (height-2*PADDING));
}

glm::vec2 TFHistogramWidget::PixelToNDC(const QPointF &p) const
{
    return PixelToNDC(vec2(p.x(), p.y()));
}
