#pragma once

namespace VAPoR {
class ParamsBase;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

//! \class Updateable
//! \brief Provides an interface that standardizes objects that support params updates.
//! \author Stas Jaroszynski

class Updateable {
public:
    virtual void Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr = nullptr, VAPoR::DataMgr *dataMgr = nullptr) = 0;
};
