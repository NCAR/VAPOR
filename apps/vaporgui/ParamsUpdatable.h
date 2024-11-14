#pragma once

namespace VAPoR {
class ParamsBase;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

//! \class ParamsUpdatable
//! \brief Provides an interface that standardizes objects that support params updates.
//! \author Stas Jaroszynski

class ParamsUpdatable {
public:
    virtual void Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr = nullptr, VAPoR::DataMgr *dataMgr = nullptr) = 0;
};
