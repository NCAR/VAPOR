#pragma once

#include "TFMapWidget.h"
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

class ControlPointList {
public:
    class PointIterator {
        ControlPointList *list;
        int i;
        
        PointIterator(ControlPointList *list, int i) : list(list), i(i) {}
        
    public:
        PointIterator(){}
        PointIterator &operator ++() { ++i; return *this; }
        PointIterator &operator --() { --i; return *this; }
        PointIterator operator +(int x) const { return PointIterator(list, i+x); }
        PointIterator operator -(int x) const { return PointIterator(list, i-x); }
        glm::vec2 &operator *() { return (*list)[i]; }
        bool operator !=(const PointIterator &other) { return !(*this == other); }
        bool operator ==(const PointIterator &other) {
            return other.list == this->list
            &&     other.i == this->i;
        }
        
        bool IsFirst() const { return i == 0; }
        bool IsLast() const { return i == list->Size()-1; }
        int Index() const { return i; }
        
        friend class ControlPointList;
    };
    
    class LineIterator : public PointIterator {
        using PointIterator::PointIterator;
        
    public:
        glm::vec2 &operator *() = delete;
        bool IsFirst() const = delete;
        bool IsLast() const = delete;
        
        const glm::vec2 a() {
            if (i == 0) return glm::vec2(0, (*list)[0].y);
            return (*list)[i-1];
        }
        const glm::vec2 b() {
            if (i == list->SizeLines()-1) return glm::vec2(1, (*list)[list->Size()-1].y);
            return (*list)[i];
        }
        
        friend class ControlPointList;
    };
    
    std::vector<glm::vec2> _points;
    
public:
    glm::vec2 &operator [](const int i) {
        VAssert(i >= 0 && i < _points.size());
        return _points[i];
    }
    
    int Add(const glm::vec2 &v)
    {
        for (int i = 0; i < _points.size(); i++)
            if (_points[i].x > v.x)
                return Add(v, i);
        
        return Add(v, Size());
    }
    
    int Add(const glm::vec2 &v, const int i) {
        VAssert(i >= 0 && i <= _points.size());
        _points.insert(_points.begin()+i, v);
        return i;
    }
    
    int Add(const glm::vec2 &v, const LineIterator &line) {
        VAssert(line.i >= 0 && line.i <= _points.size());
        _points.insert(_points.begin() + line.i, v);
        return line.i;
    }
    
    void Remove(const PointIterator &point) {
        VAssert(point.i >= 0 && point.i < _points.size());
        if (Size() > 2) // VAPoR::MapperFunc cannot handle less than 2
            _points.erase(_points.begin() + point.i);
    }
    
    int Size() const { return _points.size(); }
    int SizeLines() const { return Size() + 1; }
    void Resize(int n) { _points.resize(n); }
    
    PointIterator BeginPoints() { return PointIterator(this, 0); }
    PointIterator EndPoints()   { return PointIterator(this, Size()); }
    LineIterator BeginLines() { return LineIterator(this, 0); }
    LineIterator EndLines()   { return LineIterator(this, SizeLines()); }
};

class TFOpacityMap : public TFMap {
    Q_OBJECT
    
public:
    TFOpacityMap(TFMapWidget *parent);
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) override;
    
    QSize minimumSizeHint() const override;
    void Deactivate() override;
    void PopulateContextMenu(QMenu *menu, const glm::vec2 &p) override;
    
protected:
    TFInfoWidget *createInfoWidget() override;
    void paintEvent(QPainter &p) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    
private:
    VAPoR::ParamsMgr *_paramsMgr = nullptr;
    VAPoR::RenderParams *_renderParams = nullptr;
    ControlPointList _controlPoints;
    bool _isDraggingControl = false;
    glm::vec2 _controlStartValue;
    ControlPointList::PointIterator _draggedControl;
    glm::vec2 _dragOffset;
    glm::vec2 m;
    int _selectedControl = -1;
    
    void opacityChanged();
    
    bool controlPointContainsPixel(const glm::vec2 &cp, const glm::vec2 &pixel) const;
    ControlPointList::PointIterator findSelectedControlPoint(const glm::vec2 &mouse);
    
    void selectControlPoint(ControlPointList::PointIterator it);
    void deleteControlPoint(ControlPointList::PointIterator it);
    void addControlPoint(const glm::vec2 &ndc);
    
signals:
    void ControlPointDeselected();
    void UpdateInfo(float value, float opacity);
    
public slots:
    void DeselectControlPoint();
    void UpdateFromInfo(float value, float opacity);
    
private slots:
    void menuDeleteSelectedControlPoint();
    void menuAddControlPoint();
};

class TFOpacityWidget : public TFMapWidget {
public:
    TFOpacityWidget();
};
