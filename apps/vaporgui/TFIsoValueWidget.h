#pragma once

#include "TFMapWidget.h"
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

// class TFIsoValueInfoWidget;

class TFIsoValueMap : public TFMap {
    Q_OBJECT

public:
    bool BottomPadding = false;

    TFIsoValueMap(TFMapWidget *parent = nullptr);
    void PopulateContextMenu(QMenu *menu, const glm::vec2 &p) override;

    QSize minimumSizeHint() const override;
    void  LostFocus() override;

    void SetEquidistantIsoValues(bool b) { _equidistantIsoValues = b; }

protected:
    void          paramsUpdate() override;
    TFInfoWidget *createInfoWidget() override;
    void          paintEvent(QPainter &p) override;
    void          drawControl(QPainter &p, const QPointF &pos, bool selected = false, bool invalid = false) const;
    float         GetControlPointTriangleHeight() const;
    float         GetControlPointSquareHeight() const;
    QRect         GetControlPointArea(const QPoint &p) const;
    void          mousePressEvent(QMouseEvent *event) override;
    void          mouseReleaseEvent(QMouseEvent *event) override;
    void          mouseMoveEvent(QMouseEvent *event) override;
    void          mouseDoubleClickEvent(QMouseEvent *event) override;
    QMargins      GetPadding() const override;

private:
    bool               _isDraggingControl = false;
    int                _draggingControlID;
    int                _selectedId = -1;
    glm::vec2          _dragOffset;
    glm::vec2          m;
    std::vector<float> _isoValues;
    std::vector<bool>  _isoValuesInBounds;
    bool               _equidistantIsoValues = true;

    bool controlPointContainsPixel(const glm::vec2 &cp, const glm::vec2 &pixel) const;

    void      saveToParams(VAPoR::RenderParams *rp) const;
    void      loadFromParams(VAPoR::RenderParams *rp);
    void      clampIsoValuesToMappingRange();
    int       addControlPoint(float value);
    void      deleteControlPoint(int i);
    void      moveControlPoint(int *index, float value);
    void      selectControlPoint(int index);
    int       findSelectedControlPoint(const glm::vec2 &mouse) const;
    bool      controlPointContainsPixel(float cp, const glm::vec2 &pixel) const;
    QPoint    controlQPositionForValue(float value) const;
    glm::vec2 controlPositionForValue(float value) const;
    float     controlXForValue(float value) const;
    float     valueForControlX(float position) const;
    float     getMapRangeMin() const;
    float     getMapRangeMax() const;

signals:
    void ControlPointDeselected();
    void UpdateInfo(float value);

public slots:
    void DeselectControlPoint();
    void UpdateFromInfo(float value);

private slots:
    void menuDeleteControlPoint();
    void menuAddControlPoint();
};

class TFIsoValueWidget : public TFMapWidget {
public:
    TFIsoValueWidget() : TFMapWidget(new TFIsoValueMap(this)) {}
};
