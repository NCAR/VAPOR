#pragma once

#include <QFrame>

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class TFInfoWidget;

class TFMapWidget : public QFrame {
    Q_OBJECT
    
public:
    TFInfoWidget *GetInfoWidget();
    virtual void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) = 0;
    virtual void Deactivate() = 0;
    
protected:
    void drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
    virtual TFInfoWidget *createInfoWidget() = 0;
    
private:
    TFInfoWidget *_infoWidget = nullptr;
    
signals:
    void Activated(TFMapWidget *who);
};
