#pragma once

#include "PWidgetHLI.h"
#include "PVariableSelector.h"

//! \class PVariableSelectorHLI
//! Allows the user to select variables. Automatically switches between 2D and 3D
//! based on the currently selected variable.
//!
//! Designed to be used with a RenderParams object.

template<class P>
class PVariableSelectorHLI : 
    public PVariableSelector,
    public PWidgetHLIBase<P, std::string> 
{
public:
    
    PVariableSelectorHLI(
        const std::string &label,
        typename PWidgetHLIBase<P, std::string>::GetterType getter,
        typename PWidgetHLIBase<P, std::string>::SetterType setter
    ) : 
        PVariableSelector2D( "", label ),
        PWidgetHLIBase<P, std::string> (
            (PWidget*)this,
            getter,
            setter
        ) 
    {}
};

template<class P>
class PVariableSelector2DHLI : 
    public PVariableSelector2D,
    public PWidgetHLIBase<P, std::string> 
{
public:
    
    PVariableSelector2DHLI(
        const std::string &label,
        typename PWidgetHLIBase<P, std::string>::GetterType getter,
        typename PWidgetHLIBase<P, std::string>::SetterType setter
    ) : 
        PVariableSelector2D( "", label ),
        PWidgetHLIBase<P, std::string> (
            (PWidget*)this,
            getter,
            setter
        ) 
    {}
};

template<class P>
class PVariableSelector3DHLI : 
    public PVariableSelector3D,
    public PWidgetHLIBase<P, std::string> 
{
public:
    
    PVariableSelector3DHLI(
        const std::string &label,
        typename PWidgetHLIBase<P, std::string>::GetterType getter,
        typename PWidgetHLIBase<P, std::string>::SetterType setter
    ) : 
        PVariableSelector3D( "", label ),
        PWidgetHLIBase<P, std::string> (
            (PWidget*)this,
            getter,
            setter
        ) 
    {}
};
