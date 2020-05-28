#include <sstream>
#include <QGroupBox>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>

#include "vapor/RenderParams.h"
#include "vapor/ParamsMgr.h"
#include "vapor/DataMgr.h"

#include "VFidelitySection.h"
#include "VLineComboBox.h"
#include "VSection.h"
#include "VariableGetter.h"

using namespace VAPoR;

const std::string VFidelitySection::_sectionTitle = "Data Fidelity";

// This namespace provides a function for the three Fidelity Widgets
// (FidelityButtons, and comboBoxes for lod and ref) to acquire strings
// that correspond to different refinement and compression factors.
namespace {
    void getCompressionFactors(
        VAPoR::RenderParams*      rParams,
        VAPoR::DataMgr*           dataMgr,
        VariableFlags             variableFlags,
        std::vector<float>&       lodCFs,
        std::vector<float>&       multiresCFs,
        std::vector<std::string>& lodStrs,
        std::vector<std::string>& multiresStrs
    ) {
        lodCFs.clear();
        lodStrs.clear();
        multiresCFs.clear();
        multiresStrs.clear();

        std::string varName = getCurrentVariable( rParams, dataMgr, variableFlags );
        VAssert(! varName.empty());
        int numLevels = dataMgr->GetNumRefLevels(varName);

        // Compute sorted list of number of grids points
        // at each level in multiresolution hierarchy
        //
        vector <size_t> nGridPts;
        for (int l=0; l<numLevels; l++) {

            vector <size_t> dims_at_level;
            int rc = dataMgr->GetDimLensAtLevel(varName, l, dims_at_level);
            VAssert(rc >= 0);

            size_t n = 1;
            ostringstream oss;
            oss << l << " (";
            for (int j=0; j<dims_at_level.size(); j++) {
                n *= dims_at_level[j];

                oss << dims_at_level[j];
                if (j < dims_at_level.size()-1) oss << "x";
            }
            nGridPts.push_back(n);

            oss << ")";
            multiresStrs.push_back(oss.str());
        }

        for (int i=0; i<nGridPts.size()-1; i++) {
            float cf = 1.0 / (nGridPts[nGridPts.size()-1] / nGridPts[i]);
            multiresCFs.push_back(cf);
        }
        multiresCFs.push_back(1.0);

        // Now get the "levels of detail" compression factors
        //
        vector <size_t> cratios = dataMgr->GetCRatios(varName);

        for (int i=0; i<cratios.size(); i++) {
            ostringstream oss;
            lodCFs.push_back((float) 1.0 / cratios[i]);

                oss << i << " (" << cratios[i] << ":1)";
                lodStrs.push_back(oss.str());
        }
    }
}

VFidelitySection::VFidelitySection()
    : VSection( _sectionTitle )
{
    _fidelityButtons = new VFidelityButtons();
    layout()->addWidget( _fidelityButtons );

    _lodCombo = new VLineComboBox( "Level of detail" );
    layout()->addWidget( _lodCombo );
    connect( _lodCombo, &VLineComboBox::IndexChanged,
        this, &VFidelitySection::setCompRatio );

    _refCombo = new VLineComboBox( "Refinement level" );
    layout()->addWidget( _refCombo );
    connect( _refCombo, &VLineComboBox::IndexChanged,
        this, &VFidelitySection::setNumRefinements );
}

void VFidelitySection::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
    _fidelityButtons->Reinit( _variableFlags);
}

void VFidelitySection::Update(
    VAPoR::RenderParams* rParams,
    VAPoR::ParamsMgr*    paramsMgr,
    VAPoR::DataMgr*      dataMgr
) {
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(rParams);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = rParams;

    _fidelityButtons->Update( _rParams, _paramsMgr, _dataMgr );
    
    vector <float> lodCFs, multiresCFs;
    vector <string> lodStrs, multiresStrs;
    getCompressionFactors(
        _rParams,
        _dataMgr,
        _variableFlags,
        lodCFs, 
        multiresCFs, 
        lodStrs, 
        multiresStrs
    );

    _lodCombo->SetOptions( lodStrs );
    _lodCombo->SetIndex( _rParams->GetCompressionLevel() );
    
    _refCombo->SetOptions( multiresStrs );
    _refCombo->SetIndex( _rParams->GetRefinementLevel() );
}

void VFidelitySection::setNumRefinements( int num ) {
    _rParams->SetRefinementLevel(num);
}

void VFidelitySection::setCompRatio( int num ) {
    _rParams->SetCompressionLevel(num);
}

VFidelityButtons::VFidelityButtons()
    : VLineItem( "Fidelity", _fidelityBox = new QGroupBox("low <--> high") )
{
    _fidelityBox->setAlignment( Qt::AlignHCenter );
    _fidelityButtons  = new QButtonGroup(_fidelityBox);
    QHBoxLayout* hlay = new QHBoxLayout(_fidelityBox);
    hlay->setAlignment(Qt::AlignHCenter);
    _fidelityButtons->setExclusive(true);
    _fidelityBox->setLayout(hlay);

    connect(
        _fidelityButtons,SIGNAL(buttonClicked(int)),
        this, SLOT(setFidelity(int))
    );
}

void VFidelityButtons::Reinit( VariableFlags varFlags ) {
    _variableFlags = varFlags;
}

void VFidelityButtons::Update(
    VAPoR::RenderParams* params,
    VAPoR::ParamsMgr* paramsMgr,
    VAPoR::DataMgr* dataMgr
) {
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(params);

    _dataMgr   = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams   = params;

    setEnabled(true);
    show();

    // Get the effective compression rates as a floating point value,
    // and as a string that can be displayed, for the LOD and refinement
    // control
    //
    vector <float> lodCFs, multiresCFs;
    vector <string> lodStrs, multiresStrs;
    getCompressionFactors(
        _rParams,
        _dataMgr,
        _variableFlags,
        lodCFs, 
        multiresCFs, 
        lodStrs, 
        multiresStrs
    );

    int lod = _rParams->GetCompressionLevel();
    int refLevel = _rParams->GetRefinementLevel();

    // set up the refinement and LOD combos
    //
    for (int i = 0; i<lodStrs.size(); i++){
        QString s = QString::fromStdString(lodStrs[i]);
    }

    _fidelityBox->adjustSize();

    // Linearize the LOD and refinement compression ratios so that
    // when combined they increase (decrease) monotonically
    //
    _fidelityLodIdx.clear();
    _fidelityMultiresIdx.clear();
    _fidelityLodStrs.clear();
    _fidelityMultiresStrs.clear();

    int l = 0;
    int m = 0;
    do {
        _fidelityLodIdx.push_back(l);
        _fidelityMultiresIdx.push_back(m);

        _fidelityLodStrs.push_back(lodStrs[l]);
        _fidelityMultiresStrs.push_back(multiresStrs[m]);

        if (lodCFs[l] < multiresCFs[m]) {
            l++;
        }
        else {
            m++;
        }
    } while (l<lodCFs.size() && m < multiresCFs.size());

    _fidelityButtons->blockSignals(true);
    // Remove buttons from the group
    //
    QList<QAbstractButton*> btns = _fidelityButtons->buttons();
    for (int i = 0; i<btns.size(); i++){
        _fidelityButtons->removeButton(btns[i]);
    }

    // Remove and delete buttons from the layout
    //
    QHBoxLayout* hlay = (QHBoxLayout*) _fidelityBox->layout();
    QLayoutItem *child;
    while ((child = hlay->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }

    int numButtons = _fidelityLodStrs.size();
    for (int i = 0; i<numButtons; i++)
    {
        QRadioButton * rd = new QRadioButton();
        hlay->addWidget(rd);

        _fidelityButtons->addButton(rd, i);
        QString qs = "Refinement " + QString::fromStdString(_fidelityMultiresStrs[i])
                     + "\nLOD " + QString::fromStdString(_fidelityLodStrs[i]);

        rd->setToolTip(qs);

        if (lod == _fidelityLodIdx[i] && refLevel == _fidelityMultiresIdx[i])
        {
            rd->setChecked(true);
        }
    }
    _fidelityButtons->blockSignals(false);
}


std::string VFidelitySection::GetCurrentLodString() const {
    return _lodCombo->GetValue();
}

std::string VFidelitySection::GetCurrentMultiresString() const {
    return _refCombo->GetValue();
}

void VFidelityButtons::setFidelity(int buttonID) {
    VAssert(_rParams);

    VAssert(buttonID >= 0 && buttonID <_fidelityLodIdx.size());

    int lod = _fidelityLodIdx[buttonID];
    int ref = _fidelityMultiresIdx[buttonID];

    _paramsMgr->BeginSaveStateGroup( "Set variable fidelity" );
    _rParams->SetCompressionLevel(lod);
    _rParams->SetRefinementLevel(ref);
    _paramsMgr->EndSaveStateGroup();
}
