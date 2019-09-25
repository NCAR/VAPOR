#pragma once

#include <QWidget>
#include <QStackedWidget>
#include <vector>

class TFInfoWidget;
class TFMapWidget;
class TFMap;
class TFMapInfoGroupWidget;

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}




class TFMapGroupWidget : public QWidget {
    Q_OBJECT
    
    std::vector<TFMapWidget*> _maps;
    
public:
    TFMapGroupWidget();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    TFMapInfoGroupWidget *CreateInfoGroup();

    void Add(TFMapWidget *mapWidget);
    void Add(TFMap *map);
    void Add(const std::initializer_list<TFMap *> &layeredMaps);
    
private slots:
    void mapActivated(TFMapWidget *map);
};





class TFMapInfoGroupWidget : public QStackedWidget {
    Q_OBJECT
    
    std::vector<TFInfoWidget*> _infos;
    
public:
    void Update(VAPoR::RenderParams *rParams);
    
    friend class TFMapGroupWidget;
    
private:
    void add(TFMapWidget *map);
    
private slots:
    void mapActivated(TFMap *map);
};
