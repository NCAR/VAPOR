#pragma once

#include "VFrame.h"
#include "ParamsUpdatable.h"

class PProjectionStringWidget;

class VProjectionStringFrame : public VFrame , public ParamsUpdatable {
    Q_OBJECT

    PProjectionStringWidget *_section;

public:
    VProjectionStringFrame(PProjectionStringWidget *psection);
    void Update(VAPoR::ParamsBase *p, VAPoR::ParamsMgr *pm = nullptr, VAPoR::DataMgr *dm = nullptr) override;

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void closed();
};
