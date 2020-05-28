#pragma once

#include <QObject>
#include "vapor/MyBase.h"
#include "Flags.h"
#include "VSection.h"
#include "PLODSelectorHLI.h"
#include "PRefinementSelectorHLI.h"

QT_USE_NAMESPACE

class QGroupBox;
class VFidelityButtons;
class PLODSelector;

class VPFidelitySection : public VSection
{
    Q_OBJECT

    public:
        VPFidelitySection();

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
