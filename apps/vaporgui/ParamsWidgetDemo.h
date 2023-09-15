#pragma once

#include <QWidget>

namespace VAPoR {
class ParamsBase;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class PGroup;

//! \class ParamsWidgetDemo
//! Shows a demo kitchen sink for Params Widgets
//! To view, from the menu bar press Developer > Show PWidget Demo

class ParamsWidgetDemo : public QWidget {
    Q_OBJECT

    PGroup *pg;

public:
    ParamsWidgetDemo();
    void Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr = nullptr, VAPoR::DataMgr *dataMgr = nullptr);
};
