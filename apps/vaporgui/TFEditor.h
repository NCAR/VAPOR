#pragma once

#include <QTabWidget>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>

class TFFunctionEditor;

class TFEditor : public QTabWidget {
    Q_OBJECT
    
public:
    TFEditor();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
private:
    
    TFFunctionEditor *tff;
    QWidget *_tab() const;
};
