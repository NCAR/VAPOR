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
}    // namespace VAPoR

//! \class TFMapGroupWidget
//! Manages a vertical stack of TFMap without any spacing between them
//! to create the illusion of a continuous widget

class TFMapGroupWidget : public QWidget {
    Q_OBJECT

    std::vector<TFMapWidget *> _maps;

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

//! \class TFMapInfoGroupWidget
//! Holds a group of TFMapInfo and only displays the relevant one.

class TFMapInfoGroupWidget : public QStackedWidget {
    Q_OBJECT

    std::vector<TFInfoWidget *> _infos;

public:
    void Update(VAPoR::RenderParams *rParams);

    friend class TFMapGroupWidget;

private:
    void add(TFMapWidget *map);

private slots:
    void mapActivated(TFMap *map);
};
