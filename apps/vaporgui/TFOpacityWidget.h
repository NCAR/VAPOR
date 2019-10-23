#pragma once

#include "TFMapWidget.h"
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

class ControlPointList {
public:
    class PointIterator {
        ControlPointList *list;
        int               i;

        PointIterator(ControlPointList *list, int i) : list(list), i(i) {}

    public:
        PointIterator() {}
        PointIterator &operator++()
        {
            ++i;
            return *this;
        }
        PointIterator &operator--()
        {
            --i;
            return *this;
        }
        PointIterator operator+(int x) const { return PointIterator(list, i + x); }
        PointIterator operator-(int x) const { return PointIterator(list, i - x); }
        glm::vec2 &   operator*() { return (*list)[i]; }
        bool          operator!=(const PointIterator &other) { return !(*this == other); }
        bool          operator==(const PointIterator &other) { return other.list == this->list && other.i == this->i; }

        bool IsFirst() const { return i == 0; }
        bool IsLast() const { return i == list->Size() - 1; }
        int  Index() const { return i; }

        friend class ControlPointList;
    };

    class LineIterator : public PointIterator {
        using PointIterator::PointIterator;

    public:
        glm::vec2 &operator*() = delete;
        //        bool IsFirst() const = delete;
        //        bool IsLast() const = delete;
        bool         IsLast() const { return i == list->Size(); }
        LineIterator operator+(int x) const { return LineIterator(list, i + x); }
        LineIterator operator-(int x) const { return LineIterator(list, i - x); }

        const glm::vec2 a()
        {
            if (i == 0) return glm::vec2(0, (*list)[0].y);
            return (*list)[i - 1];
        }
        const glm::vec2 b()
        {
            if (i == list->SizeLines() - 1) return glm::vec2(1, (*list)[list->Size() - 1].y);
            return (*list)[i];
        }
        void setA(const glm::vec2 &v)
        {
            if (i != 0) (*list)[i - 1] = v;
        }
        void setB(const glm::vec2 &v)
        {
            if (i != list->SizeLines() - 1) (*list)[i] = v;
        }

        friend class ControlPointList;
    };

    std::vector<glm::vec2> _points;

public:
    glm::vec2 &operator[](const int i)
    {
        VAssert(i >= 0 && i < _points.size());
        return _points[i];
    }

    int Add(const glm::vec2 &v)
    {
        for (int i = 0; i < _points.size(); i++)
            if (_points[i].x > v.x) return Add(v, i);

        return Add(v, Size());
    }

    int Add(const glm::vec2 &v, const int i)
    {
        VAssert(i >= 0 && i <= _points.size());
        glm::vec2 vClamped = glm::clamp(v, glm::vec2(0, 0), glm::vec2(1, 1));
        _points.insert(_points.begin() + i, vClamped);
        return i;
    }

    int Add(const glm::vec2 &v, const LineIterator &line)
    {
        VAssert(line.i >= 0 && line.i <= _points.size());
        _points.insert(_points.begin() + line.i, v);
        return line.i;
    }

    void Remove(const PointIterator &point)
    {
        VAssert(point.i >= 0 && point.i < _points.size());
        if (Size() > 1) _points.erase(_points.begin() + point.i);
    }

    int  Size() const { return _points.size(); }
    int  SizeLines() const { return Size() + 1; }
    void Resize(int n) { _points.resize(n); }

    PointIterator BeginPoints() { return PointIterator(this, 0); }
    PointIterator EndPoints() { return PointIterator(this, Size()); }
    LineIterator  BeginLines() { return LineIterator(this, 0); }
    LineIterator  EndLines() { return LineIterator(this, SizeLines()); }
};

class TFOpacityMap : public TFMap {
    Q_OBJECT

public:
    TFOpacityMap(TFMapWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    void  LostFocus() override;
    void  PopulateContextMenu(QMenu *menu, const glm::vec2 &p) override;
    void  PopulateSettingsMenu(QMenu *menu) const override;

protected:
    void          paramsUpdate() override;
    TFInfoWidget *createInfoWidget() override;
    void          paintEvent(QPainter &p) override;
    void          mousePressEvent(QMouseEvent *event) override;
    void          mouseReleaseEvent(QMouseEvent *event) override;
    void          mouseMoveEvent(QMouseEvent *event) override;
    void          mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    ControlPointList                _controlPoints;
    bool                            _isDraggingControl = false;
    bool                            _isDraggingLine = false;
    ControlPointList::PointIterator _draggedControl;
    ControlPointList::LineIterator  _draggedLine;
    glm::vec2                       _dragOffset;
    glm::vec2                       _dragOffsetB;
    glm::vec2                       m;
    int                             _selectedControl = -1;

    void opacityChanged();

    bool                            controlPointContainsPixel(const glm::vec2 &cp, const glm::vec2 &pixel) const;
    ControlPointList::PointIterator findSelectedControlPoint(const glm::vec2 &mouse);
    ControlPointList::LineIterator  findSelectedControlLine(const glm::vec2 &mouse);

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
    void menuLoad();
    void menuSave();
};

class TFOpacityWidget : public TFMapWidget {
public:
    TFOpacityWidget();
};
