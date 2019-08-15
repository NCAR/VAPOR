#include <QWidget>
#include <QFrame>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

class TFColorWidget : public QFrame {
    Q_OBJECT
    
public:
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    QSize minimumSizeHint() const;
    
protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    
private:
    VAPoR::ParamsMgr *_paramsMgr = nullptr;
    VAPoR::RenderParams *_renderParams = nullptr;
    bool _isDraggingControl = false;
    int _draggingControlID;
    glm::vec2 _dragOffset;
    glm::vec2 m;
    
    void opacityChanged();
    
    bool controlPointContainsPixel(const glm::vec2 &cp, const glm::vec2 &pixel) const;
    
    glm::vec2 NDCToPixel(const glm::vec2 &v) const;
    QPointF   QNDCToPixel(const glm::vec2 &v) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
    glm::vec2 PixelToNDC(const glm::vec2 &p) const;
    
    VAPoR::ColorMap *getColormap() const;
    int findSelectedControlPoint(const glm::vec2 &mouse) const;
    bool controlPointContainsPixel(float cp, const glm::vec2 &pixel) const;
    QPointF controlQPositionForValue(float value) const;
    glm::vec2 controlPositionForValue(float value) const;
    float controlXForValue(float value) const;
    float valueForControlX(float position) const;
};
