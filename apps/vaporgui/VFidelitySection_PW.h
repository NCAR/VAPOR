#pragma once

#include <QObject>
#include "vapor/MyBase.h"
#include "Flags.h"
#include "VSection.h"

//
// Include HLI template objects instead of predeclaring them, as per
// Google's C++ style guide.
//
// https://google.github.io/styleguide/cppguide.html#Forward_Declarations  
//

#include "PLODSelectorHLI.h"
#include "PRefinementSelectorHLI.h"

QT_USE_NAMESPACE

class QGroupBox;
class PGroup;
class PLODSelector;
class VFidelityButtons;

class VFidelitySection_PW : public VSection
{
    Q_OBJECT

    public:
        VFidelitySection_PW();

        void Reinit( VariableFlags variableFlags );

        void Update(
            VAPoR::RenderParams* rParams,
            VAPoR::ParamsMgr*    paramsMgr,
            VAPoR::DataMgr*      dataMgr
        );

        std::string GetCurrentLodString() const;
        std::string GetCurrentMultiresString() const;
 
    private:
        VAPoR::DataMgr*      _dataMgr;
        VAPoR::ParamsMgr*    _paramsMgr;
        VAPoR::RenderParams* _rParams;

        static const std::string _sectionTitle;
        
        VariableFlags _variableFlags;

        VFidelityButtons*                               _fidelityButtons;

        PGroup*                                         _pGroup;
        PLODSelectorHLI<VAPoR::RenderParams>*           _plodHLI;
        PRefinementSelectorHLI<VAPoR::RenderParams>*    _pRefHLI;

        std::string            _currentLodStr;
        std::string            _currentMultiresStr;
        std::vector <string>   _fidelityLodStrs;
        std::vector <string>   _fidelityMultiresStrs;
        
    private slots:
        void setNumRefinements( int num );
        void setCompRatio( int num );

};
