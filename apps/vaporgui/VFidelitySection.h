#pragma once

#include <QObject>
#include "vapor/MyBase.h"
#include "Flags.h"
#include "PWidget.h"
#include "VSection.h"
#include "VLineItem.h"

QT_USE_NAMESPACE

namespace VAPoR {
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class QGroupBox;
class VLineComboBox;
class FidelityWidget3;
class VFidelityButtons;

class CompressionWidget
{
    public:
        void Reinit(VariableFlags variableFlags);

    protected:
        void getCompressionFactors(
            VAPoR::RenderParams*      rParams,
            VAPoR::DataMgr*           dataMgr,
            VariableFlags             variableFlags,
            vector <float>&           lodCF, 
            vector <float>&           multiresCF, 
            std::vector<std::string>& lodStr,
            std::vector<std::string>& multiresStr
        );

        std::string getCurrentVariableName( 
            VariableFlags        variableFlags,
            VAPoR::RenderParams* rParams,
            VAPoR::DataMgr*      dataMgr
        ) const;

};

class VFidelitySection : public VSection, public CompressionWidget
{
    Q_OBJECT

    public:
        VFidelitySection();

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

        VFidelityButtons* _fidelityButtons;
        VLineComboBox*    _lodCombo;
        VLineComboBox*    _refCombo;

        std::string            _currentLodStr;
        std::string            _currentMultiresStr;
        std::vector <string>   _fidelityLodStrs;
        std::vector <string>   _fidelityMultiresStrs;
        
    private slots:
        void setNumRefinements( int num );
        void setCompRatio( int num );
};

class VFidelityButtons : public VLineItem, public CompressionWidget
{

	Q_OBJECT

    public: 
        VFidelityButtons();

        void Reinit(VariableFlags variableFlags);

        virtual void Update(
            VAPoR::RenderParams* params,
            VAPoR::ParamsMgr*    paramsMgr,
            VAPoR::DataMgr*      dataMgr
        );

    protected slots:
        void setFidelity(int buttonID);

    private:
        VAPoR::DataMgr*      _dataMgr;
        VAPoR::ParamsMgr*    _paramsMgr;
        VAPoR::RenderParams* _rParams;

        VariableFlags _variableFlags;

        QButtonGroup* _fidelityButtons;
        QGroupBox* _fidelityBox;
        QFrame* _fidelityFrame;

        VLineComboBox* _lodCombo;
        VLineComboBox* _refCombo;

        /*std::string getCurrentVariableName() const;

        // Get the compression rates as a fraction for both the LOD and
        // Refinment parameters. Also format these factors into a displayable
        // string
        //
        void getCmpFactors(
            const std::string& varname, 
            vector <long> &lodCF, 
            vector <string> &lodStr,
            vector <long> &multiresCF, 
            vector <string> &multiresStr
        ) const;*/

        void setupFidelity( VAPoR::RenderParams params );
        void uncheckFidelity();

        std::vector <std::string> _fidelityLodStrs;    
        std::vector <std::string> _fidelityMultiresStrs;    
        std::vector <int>         _fidelityLodIdx;
        std::vector <int>         _fidelityMultiresIdx;
};
