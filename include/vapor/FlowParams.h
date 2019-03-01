#ifndef FLOWPARAMS_H
#define FLOWPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR 
{

class PARAMS_API FlowParams : public RenderParams 
{
public:

    //FlowParams( DataMgr*                 dataManager, 
    //            ParamsBase::StateSave*   stateSave,
    //            std::string              classType );
    FlowParams( DataMgr*                 dataManager, 
                ParamsBase::StateSave*   stateSave );
    FlowParams( DataMgr*                 dataManager, 
                ParamsBase::StateSave*   stateSave, 
                XmlNode*                 xmlNode );

    virtual ~FlowParams();

    //
    // (Pure virtual methods from RenderParams)
    //
    virtual bool IsOpaque() const override
    { 
        return false; 
    }
    virtual bool usingVariable(const std::string& varname) override
    {
        return false;
    }

    static std::string GetClassType() 
    {
        return ("FlowParams");
    }

protected:

};

}

#endif
