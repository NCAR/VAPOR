#ifndef FLOWPARAMS_H
#define FLOWPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR 
{

class PARAMS_API FlowParams : public RenderParams 
{
public:

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

    // True  == Steady
    // False == Unteady
    void SetSteady( bool steady );
    bool GetSteady() const;

protected:

    static const std::string _isSteadyTag;

};

}

#endif
