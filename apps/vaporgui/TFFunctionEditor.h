#include <QGLWidget>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

class ControlPointList {
    
    class PointIterator {
        ControlPointList *list;
        int i;
        
        PointIterator(){}
        PointIterator(ControlPointList *list, int i) : list(list), i(i) {}
        
    public:
        PointIterator &operator ++() { ++i; return *this; }
        PointIterator &operator --() { --i; return *this; }
        glm::vec2 &operator *() { return (*list)[i]; }
        bool operator !=(const PointIterator &other) { return !(*this == other); }
        bool operator ==(const PointIterator &other) {
            return other.list == this->list
            &&     other.i == this->i;
        }
        
        friend class ControlPointList;
    };
    
    class LineIterator : public PointIterator {
        using PointIterator::PointIterator;
        
    public:
        glm::vec2 &operator *() = delete;
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
    
    void Add(const glm::vec2 &v) {
        _points.push_back(v);
    }
    
    void Add(const glm::vec2 &v, const int i) {
        _points.insert(_points.begin()+i, v);
    }
    
    int Size() const { return _points.size(); }
    int SizeLines() const { return Size() + 1; }
    
    PointIterator BeginPoints() { return PointIterator(this, 0); }
    PointIterator EndPoints()   { return PointIterator(this, Size()); }
    LineIterator BeginLines() { return LineIterator(this, 0); }
    LineIterator EndLines()   { return LineIterator(this, SizeLines()); }
};

class TFFunctionEditor : public QWidget {
    Q_OBJECT
    
public:
    
    TFFunctionEditor();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    QSize minimumSizeHint() const;
    
protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    
private:
    ControlPointList _controlPoints;
    int _draggedID = -1;
    glm::vec2 _dragOffset;
    glm::vec2 m;
    
    QPointF NDCToPixel(const glm::vec2 &v) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
};
