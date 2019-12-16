#pragma once

#include <string>
#include <QWidget>

namespace VAPoR {
class ParamsBase;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class PWidget : public QWidget {
    Q_OBJECT

    VAPoR::ParamsBase *_params = nullptr;
    VAPoR::ParamsMgr * _paramsMgr = nullptr;
    VAPoR::DataMgr *   _dataMgr = nullptr;
    const std::string  _tag;

public:
    PWidget(const std::string &tag, QWidget *widget);
    void               Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr = nullptr, VAPoR::DataMgr *dataMgr = nullptr);
    const std::string &GetTag() const;

protected:
    virtual void updateGUI() const = 0;
    virtual bool requireParamsMgr() const { return false; }
    virtual bool requireDataMgr() const { return false; }

    VAPoR::ParamsBase *getParams() const;
    VAPoR::ParamsMgr * getParamsMgr() const;
    VAPoR::DataMgr *   getDataMgr() const;
};
