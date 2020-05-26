#pragma once

#include <QObject>
#include "vapor/MyBase.h"
#include "Flags.h"
#include "VSection.h"

QT_USE_NAMESPACE

namespace VAPoR {
	class RenderParams;
	class ParamsMgr;
	class DataMgr;
}

class QGroupBox;
class VLineComboBox;
class VFidelityWidget3;
class VLineItem;

class PVFidelityWidget : public PWidget {
    public:
        PVFidelityWidget();

        void updateGUI() const override;
        void Reinit(VariableFlags variableFlags);

    private:
        VFidelityWidget3* _fidelityWidget;
};

//!
//! \class VFidelityWidget3
//! \ingroup Public_GUI
//! \brief A Widget that can be reused to provide fidelity 
//! selection in any renderer EventRouter class
//! \author Scott Pearse
//! \version 3.0
//! \date  December 2017

class VFidelityWidget3 : public VSection {

	Q_OBJECT

    public: 
        VFidelityWidget3();

        void Reinit(VariableFlags variableFlags);

        virtual void Update(
           VAPoR::DataMgr *dataMgr,
           VAPoR::ParamsMgr *paramsMgr,
           VAPoR::ParamsBase *params
        );

        QButtonGroup* GetFidelityButtons();
        std::vector<int> GetFidelityLodIdx() const;

        std::string GetCurrentLodString() const;
        std::string GetCurrentMultiresString() const;
         
    protected slots:
        //! Connected to the image file text editor
        void setNumRefinements(int num);

        //! Connected to the compression ratio selector, setting the lod index.
        void setCompRatio(int num);

        //! Connected to the fidelity button selector, setting the fidelity index.
        void setFidelity(int buttonID);

    private:
           VariableFlags        _variableFlags;
           VAPoR::DataMgr*      _dataMgr;
           VAPoR::ParamsMgr*    _paramsMgr;
           VAPoR::RenderParams* _rParams;

           // Get the compression rates as a fraction for both the LOD and
           // Refinment parameters. Also format these factors into a displayable
           // string
           //
           void getCmpFactors(
              string varname, vector <long> &lodCF, vector <string> &lodStr,
              vector <long> &multiresCF, vector <string> &multiresStr
          ) const;

           void uncheckFidelity();

           void setupFidelity(
              VAPoR::RenderParams* dParams
           );

           VLineComboBox* _lodCombo;
           VLineComboBox* _refCombo;
           VLineItem*     _vle;

           // Support for fidelity settings
           //
           std::vector <int>      _fidelityLodIdx;
           std::vector <int>      _fidelityMultiresIdx;
           std::vector <string>   _fidelityLodStrs;
           std::vector <string>   _fidelityMultiresStrs;
           std::string            _currentLodStr;
           std::string            _currentMultiresStr;

           static const std::string _sectionTitle;
};
