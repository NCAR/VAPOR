#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include <QGroupBox>
#include <QDesktopWidget>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QRadioButton>

#include <QPushButton>

#include "vapor/RenderParams.h"
#include "vapor/ParamsMgr.h"
#include "vapor/DataMgr.h"

#include "PSection.h"
#include "PLODSelector.h"
#include "PRefinementSelector.h"
#include "PRefinementSelectorHLI.h"
#include "PFidelityWidget3.h"
#include "VLineComboBox.h"
#include "VSection.h"

#include "VFidelityWidget.h"
#include "VFidelitySection.h"

using namespace VAPoR;

const std::string FidelityWidget3::_sectionTitle = "Data Fidelity";

PFidelityWidget3::PFidelityWidget3()
    : PWidget( "", _fidelityWidget = new VFidelitySection() )
//    : PWidget( "", _fidelityWidget = new FidelityWidget3() )
    //: PWidget( "", _fidelityWidget = new FidelityWidget() )
{}

void PFidelityWidget3::updateGUI() const {
    //_fidelityWidget->Update( _dataMgr, _paramsMgr, _params );
    VAPoR::RenderParams* rParams = dynamic_cast<RenderParams*>(_params);
    _fidelityWidget->Update( rParams, _paramsMgr, _dataMgr );
}

void PFidelityWidget3::Reinit( VariableFlags variableFlags ) {
    _fidelityWidget->Reinit( variableFlags );
}

FidelityWidget3::FidelityWidget3() 
    : VSection( "FidelityWidget3" )
{
    //_fidelityFrame    = new QFrame();
    //_fidelityBox      = new QGroupBox( _fidelityFrame );
    _fidelityBox      = new QGroupBox("low <--> high");
    _fidelityBox->setAlignment( Qt::AlignHCenter );
    _fidelityButtons  = new QButtonGroup(_fidelityBox);
    QHBoxLayout* hlay = new QHBoxLayout(_fidelityBox);
    hlay->setAlignment(Qt::AlignHCenter);
    _fidelityButtons->setExclusive(true);
    _fidelityBox->setLayout(hlay);
	int dpi = logicalDpiX();
	if (dpi > 96) {
		_fidelityBox->setMinimumHeight(100);
    }

    _vle = new VLineItem( "Fidelity", _fidelityBox );
    layout()->addWidget( _vle );
    _fidelityBox->layout()->addWidget( new QPushButton("foo") );
    connect(
        _fidelityButtons,SIGNAL(buttonClicked(int)),
        this, SLOT(setFidelity(int))
    );

    //
    // VWidgets
    //

    VSection* vSection = new VSection( "VWidgets" );
    layout()->addWidget( vSection );

    _lodCombo = new VLineComboBox( "V Level of detail" );
    //layout()->addWidget( _lodCombo );
    vSection->layout()->addWidget( _lodCombo );
    connect( _lodCombo, &VLineComboBox::IndexChanged,
        this, &FidelityWidget3::setCompRatio );
    
    _refCombo = new VLineComboBox( "V Refinement Level" );
    connect( _refCombo, &VLineComboBox::IndexChanged,
        this, &FidelityWidget3::setNumRefinements );
    //layout()->addWidget( _refCombo );
    vSection->layout()->addWidget( _refCombo );

    // 
    // PWidgets
    //

    _ps1 = new PSection( "Pure PWidets" );
    layout()->addWidget( _ps1 );

    _plodSelector = new PLODSelector();
    _plodSelector->Reinit( (VariableFlags)SCALAR );
    _ps1->Add( _plodSelector );
    
    _refinementSelector = new PRefinementSelector();
    _refinementSelector->Reinit( (VariableFlags)SCALAR );
    _ps1->Add( _refinementSelector );

    //
    // PWidgetHLI
    //

    _ps2 = new PSection( "PWidetsHLI" );
    layout()->addWidget( _ps2 );

    _plodHLI = new PLODSelectorHLI<VAPoR::RenderParams>( 
        &RenderParams::GetCompressionLevel,
        &RenderParams::SetCompressionLevel
    );
    _plodHLI->Reinit( (VariableFlags)SCALAR );
    _ps2->Add( _plodHLI );
    
    _pRefHLI = new PRefinementSelectorHLI<VAPoR::RenderParams>( 
        &RenderParams::GetRefinementLevel,
        &RenderParams::SetRefinementLevel
    );
    _pRefHLI->Reinit( (VariableFlags)SCALAR );
    _ps2->Add( _pRefHLI );
}

void FidelityWidget3::Reinit( VariableFlags flags ) {
    _variableFlags = flags;

    _plodSelector->Reinit( flags );
    _refinementSelector->Reinit( flags );
}

void FidelityWidget3::setNumRefinements(int num) {
    VAssert(_rParams);

    _rParams->SetRefinementLevel(num);

    // Fidelity settings no longer valid
    //
    uncheckFidelity();
}

//Occurs when user clicks a fidelity radio button
//
void FidelityWidget3::setFidelity(int buttonID){
    VAssert(_rParams);

    VAssert(buttonID >= 0 && buttonID <_fidelityLodIdx.size());

    int lod = _fidelityLodIdx[buttonID];
    int ref = _fidelityMultiresIdx[buttonID];

    _paramsMgr->BeginSaveStateGroup(
        "Set variable fidelity"
    );
    _rParams->SetCompressionLevel(lod);
    _rParams->SetRefinementLevel(ref);

    _lodCombo->SetIndex( lod );
    _refCombo->SetIndex( ref );

    _paramsMgr->EndSaveStateGroup();
}

QButtonGroup* FidelityWidget3::GetFidelityButtons() {
    return _fidelityButtons;
}

std::vector<int> FidelityWidget3::GetFidelityLodIdx() const {
    return _fidelityLodIdx;
}

void FidelityWidget3::getCmpFactors(
    string varname, vector <long> &lodCF, vector <string> &lodStr,
    vector <long> &multiresCF, vector <string> &multiresStr
) const {

    VAssert(! varname.empty());

    lodCF.clear();
    lodStr.clear();
    multiresCF.clear();
    multiresStr.clear();

    int numLevels = _dataMgr->GetNumRefLevels(varname);

    // First get compression factors that are based on grid multiresolution
    //

    // Compute sorted list of number of grids points
    // at each level in multiresolution hierarchy
    //
    vector <size_t> nGridPts;   
    for (int l=0; l<numLevels; l++) {

        vector <size_t> dims_at_level;
        int rc = _dataMgr->GetDimLensAtLevel(varname, l, dims_at_level);
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
        multiresStr.push_back(oss.str());
    }

    for (int i=0; i<nGridPts.size()-1; i++) {
        float cf = 1.0 / (nGridPts[nGridPts.size()-1] / nGridPts[i]);
        multiresCF.push_back(cf);
    }
    multiresCF.push_back(1.0);
        
    // Now get the "levels of detail" compression factors
    //
    vector <size_t> cratios = _dataMgr->GetCRatios(varname);
    for (int i=0; i<cratios.size(); i++) {
        ostringstream oss;
        lodCF.push_back((float) 1.0 / cratios[i]);

            oss << i << " (" << cratios[i] << ":1)";
            lodStr.push_back(oss.str());
    }
}


void FidelityWidget3::uncheckFidelity() {

    // Unset all fidelity buttons
    //
    if (! _fidelityButtons) 
        return;

    QList<QAbstractButton*> btns = _fidelityButtons->buttons();
    for (int i = 0; i<btns.size(); i++){
        if (btns[i]->isChecked()) {
            btns[i]->setChecked(false);
        }
    }
}

void FidelityWidget3::setCompRatio(int num){
    VAssert(_rParams);

    _rParams->SetCompressionLevel(num);

    //_lodCombo->SetIndex( num );

    // Fidelity no longer valid
    //
    uncheckFidelity();
}

void FidelityWidget3::Update( 
    DataMgr *dataMgr, 
    ParamsMgr *paramsMgr,
    ParamsBase *params
) {
    VAssert(dataMgr);
    VAssert(paramsMgr);
    VAssert(params);

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _rParams = dynamic_cast<RenderParams*>( params );

    _ps1->Update( params, _paramsMgr, _dataMgr );
    _ps2->Update( params, _paramsMgr, _dataMgr );

    string varname;
    if (_variableFlags & SCALAR) {
        varname = _rParams->GetVariableName();
    }
    else if (_variableFlags & VECTOR) {
        vector <string> varnames = _rParams->GetFieldVariableNames();
        if( varnames.size() > 0 )
        {
            varname = varnames[0];
            size_t vardim;
            for( int i = 0; i < varnames.size(); i++ )
            {
                vardim = _dataMgr->GetNumDimensions( varnames[i]);
                if( vardim == 3 )
                {
                    varname = varnames[i];
                    break;
                }
            }
        }
    }
    else if (_variableFlags & HEIGHT) {
        varname = _rParams->GetHeightVariableName();
    }
    else if (_variableFlags & AUXILIARY) {
        vector<string> varnames = _rParams->GetAuxVariableNames();
        if(varnames.size() > 0)
        {
            varname = varnames[0];
            size_t vardim;
            for( int i = 0; i < varnames.size(); i++ )
            {
                vardim = _dataMgr->GetNumDimensions( varnames[i]);
                if( vardim == 3 )
                {
                    varname = varnames[i];
                    break;
                }
            }
        }
    }
    else if (_variableFlags & COLOR) {
        varname = _rParams->GetColorMapVariableName();
    }

    if (varname.empty()) {
        setEnabled(false);
        return;
    }

    setEnabled(true);
    show();

    vector <size_t> cratios = _dataMgr->GetCRatios(varname);

    // Get the effective compression rates as a floating point value,
    // and as a string that can be displayed, for the LOD and refinement
    // control
    //
    vector <long> lodCFs, multiresCFs;
    vector <string> lodStrs, multiresStrs;
    getCmpFactors(varname, lodCFs, lodStrs, multiresCFs, multiresStrs);

    int lodReq = _rParams->GetCompressionLevel();
    int refLevelReq = _rParams->GetRefinementLevel();

    int lod = lodReq < 0 ? 0 : lodReq;
    lod = lodReq >= lodCFs.size() ? lodCFs.size()-1 : lodReq;

    int refLevel = refLevelReq < 0 ? 0 : refLevelReq;
    refLevel = refLevelReq >= multiresCFs.size() ? 
        multiresCFs.size()-1 : refLevelReq;

    // set up the refinement and LOD combos
    //
    for (int i = 0; i<lodStrs.size(); i++){
        QString s = QString::fromStdString(lodStrs[i]);
    }
    _currentLodStr = lodStrs.at(lod);

    _currentMultiresStr = multiresStrs.at(refLevel);

_lodCombo->SetOptions( lodStrs );
//_lodCombo->SetValue( _currentLodStr );
_lodCombo->SetValue( _currentLodStr );

_refCombo->SetOptions( multiresStrs );
_refCombo->SetValue( _currentMultiresStr );
//_refCombo->SetIndex( refLevel );

    if (lodReq != lod) {
        _rParams->SetCompressionLevel(lod);
    }
    if (refLevelReq != refLevel) {
        _rParams->SetRefinementLevel(refLevel);
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
    
    //_pg->Update( _rParams, _paramsMgr, _dataMgr ); // takes non-const _dataMgr
    /*std::vector< std::string > lods;
    for ( int i=0; i<lodStrs.size(); i++ ) {
        lods.push_back( lodStrs[i] );
    }*/
  
    // Create vector containing values 0, 1, 2, ... N 
    //std::vector<long> lodEnum( lods.size() );
    //std::iota( std::begin( lodEnum ), std::end( lodEnum ), 0 ); 
    //_lodCombo->SetValues( lodEnum );
    //_lodCombo->SetItems( lods );

//_lodCombo->SetOptions( lodStrs );
//_lodCombo->SetIndex

//_refCombo->SetOptions( multiresStrs );
//_refCombo->SetValues( multiresCFs );
//_refCombo->Update( _rParams, _paramsMgr, _dataMgr );
}


std::string FidelityWidget3::GetCurrentLodString() const
{
    return _currentLodStr;
}
 
std::string FidelityWidget3::GetCurrentMultiresString() const
{
    return _currentMultiresStr;
}
