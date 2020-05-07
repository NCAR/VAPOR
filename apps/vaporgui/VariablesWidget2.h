#pragma once

#include "VSection.h"
#include "Flags.h"

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

//class foo;
template<typename T> class Foo;
class PGroup;
class PVariableSelector;
class VComboBox;
class VLineComboBox;
class VLineItem;
class FidelityWidget2;

class VariablesWidget2 : public VSection {

    Q_OBJECT

public:

    VariablesWidget2();
    void Reinit(
        VariableFlags varFlags,
        DimFlags      dimFlags
    );

    virtual void Update(
        //const VAPoR::DataMgr *dataMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams
    );

    int GetActiveDimension() const;
    DimFlags GetDimFlags() const;

    void Configure2DFieldVars();
    void Configure3DFieldVars();

private:
    //foo* _foo;
    Foo<int>* _foo;

    size_t _activeDim;
    bool   _initialized;

    VariableFlags _variableFlags;
    DimFlags      _dimFlags;

    VLineComboBox* _dimCombo;
    VLineComboBox* _scalarCombo;
    VLineComboBox* _xFieldCombo;
    VLineComboBox* _yFieldCombo;
    VLineComboBox* _zFieldCombo;
    VLineComboBox* _colorCombo;
    VLineComboBox* _heightCombo;
    

    FidelityWidget2* _fidelityWidget;

    static const std::string _sectionTitle;

    /*
    /Users/pearse/VAPOR/apps/vaporgui/PVariableSelectorHLI.h:13:7: 
    error: redefinition of 'PVariableSelectorHLI' as different kind of symbol

    class PVariableSelectorHLI :
      ^
    /Users/pearse/VAPOR/apps/vaporgui/VariablesWidget2.h:13:7: 
    note: previous definition is here
    class PVariableSelectorHLI;
    */

    //
    //PVariableSelectorHLI* _scalarCombo;
    //

    //const VAPoR::DataMgr* _dataMgr;
    VAPoR::DataMgr* _dataMgr;
    VAPoR::ParamsMgr* _paramsMgr;
    VAPoR::RenderParams* _rParams;

private slots:
    void _dimChanged();
    void _scalarVarChanged( std::string var );
    void _xFieldVarChanged( std::string var );
    void _yFieldVarChanged( std::string var );
    void _zFieldVarChanged( std::string var );
    void _colorVarChanged(  std::string var );
    void _heightVarChanged( std::string var );
};
