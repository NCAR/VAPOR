#pragma once

namespace VAPoR {
    class ParamsBase;
    class ParamsMgr;
    class DataMgr;
}

class Updateable {
public:
    virtual void Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr = nullptr, VAPoR::DataMgr *dataMgr = nullptr) = 0;
};
