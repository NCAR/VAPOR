#include "TFHistogramWidget.h"
#include <QPaintEvent>
#include <vapor/DataMgr.h>
#include <QPainter>
#include <glm/glm.hpp>
#include <Histo.h>

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
}

QSize TFHistogramWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

void TFHistogramWidget::paintEvent(QPaintEvent* event)
{
    printf("PAINT\n");
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    QMatrix m;
    m.translate(0, height());
    m.scale(width()/100.f, -1);
    p.setMatrix(m);
    
//    p.setViewport(0, height(), width(), 0);
    p.fillRect(rect(), Qt::gray);
    
    Histo h(100);
    h.Populate(_dataMgr, _renderParams);
    for (int i = 0; i < 100; i++) {
        p.fillRect(i, 0, 1, h.getBinSizeNormalized(i)*height(), Qt::black);
    }
}

//void TFFunctionEditor::mousePressEvent(QMouseEvent *event)
//void TFFunctionEditor::mouseReleaseEvent(QMouseEvent *event)
//void TFFunctionEditor::mouseMoveEvent(QMouseEvent *event)
//void TFFunctionEditor::mouseDoubleClickEvent(QMouseEvent *event)

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
