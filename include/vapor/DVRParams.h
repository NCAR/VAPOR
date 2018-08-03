#ifndef DVRPARAMS_H
#define DVRPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/GetAppPath.h>

namespace VAPoR {

class PARAMS_API DVRParams : public RenderParams {
public:
    DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave);
    DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    virtual ~DVRParams();

    static std::string GetClassType() { return ("DVRParams"); }

    //
    // (Pure virtual methods from RenderParams)
    //
    virtual bool IsOpaque() const override { return false; }
    virtual bool usingVariable(const std::string &varname) override
    {
        return false;    // since this class is for an image, not rendering a variable.
    }

    //
    //! Obtain current MapperFunction for the primary variable.
    //
    virtual MapperFunction *GetMapperFunc();

    bool GetLighting() const;
    void SetLighting(bool);

private:
    static const std::string _lightingTag;
};

}    // namespace VAPoR

#endif
