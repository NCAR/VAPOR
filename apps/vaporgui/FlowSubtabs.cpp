#include "FlowSubtabs.h"
#include "VariablesWidget.h"
#include "TFWidget.h"
#include "GeometryWidget.h"
#include "CopyRegionWidget.h"
#include "TransformTable.h"
#include "ColorbarWidget.h"
//#include "VaporWidgets.h"

QVaporSubtab::QVaporSubtab(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _layout->setContentsMargins(0,0,0,0);
    _layout->insertSpacing(-1, 20);
    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Maximum);
}

FlowVariablesSubtab::FlowVariablesSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _variablesWidget = new VariablesWidget(this);
    _variablesWidget->Reinit(   (VariableFlags)(VECTOR | COLOR),
                                (DimFlags)(THREED) );

    _layout->addWidget( _variablesWidget, 0, 0 );
}

void FlowVariablesSubtab::Update(   VAPoR::DataMgr *dataMgr,
                                    VAPoR::ParamsMgr *paramsMgr,
                                    VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);
    _variablesWidget->Update(dataMgr, paramsMgr, rParams);
}

FlowAppearanceSubtab::FlowAppearanceSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _TFWidget = new TFWidget(this);
    _TFWidget->Reinit((TFFlags)(SAMPLING | CONSTANT_COLOR));

    _layout->addWidget( _TFWidget, 0, 0 );

    _params = NULL;
}

void FlowAppearanceSubtab::Update(  VAPoR::DataMgr *dataMgr,
                                    VAPoR::ParamsMgr *paramsMgr,
                                    VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);

    _TFWidget->Update(dataMgr, paramsMgr, rParams);
}

FlowSeedingSubtab::FlowSeedingSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _geometryWidget   = new GeometryWidget(this);
    _geometryWidget->Reinit( 
        (DimFlags)THREED,
        (VariableFlags)VECTOR
        //(GeometryFlags)RAKE_HACK
    );
    _layout->addWidget( _geometryWidget );
   
/*    _pushTest = new VPushButton(this, "testLabel", "testButton"); 
    connect( _pushTest, SIGNAL( _pressed() ),
        this, SLOT( _pushTestPressed()));
    _layout->addWidget( _pushTest );

    _comboTest = new VComboBox(this);//, "testCombo");
    _comboTest->AddOption( "foo" );
    _comboTest->AddOption( "bar" );
    _comboTest->AddOption( "baz" );
    connect( _comboTest, SIGNAL( _indexChanged(int) ),
        this, SLOT( _comboBoxSelected(int) ));
    _layout->addWidget( _comboTest );

    _checkboxTest = new VCheckBox(this, "testCheckbox");
    connect( _checkboxTest, SIGNAL( _checkboxClicked() ),
        this, SLOT( _checkBoxSelected() ) );
    _layout->addWidget( _checkboxTest );*/
}

void FlowSeedingSubtab::Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams
    )
{
    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
}

void FlowSeedingSubtab::_pushTestPressed() 
{
    cout << "Push button pressed" << endl;
}

void FlowSeedingSubtab::_comboBoxSelected( int index ) 
{
    string option = _comboTest->GetCurrentText();
    cout << "Combo selected at index " << index << " for option " << option << endl;
}

void FlowSeedingSubtab::_checkBoxSelected() 
{
    bool checked = _checkboxTest->GetCheckState();
    cout << "Checkbox is checked? " << checked << endl;
}

FlowGeometrySubtab::FlowGeometrySubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _geometryWidget   = new GeometryWidget(this);
    _copyRegionWidget = new CopyRegionWidget(this);
    _transformTable   = new TransformTable(this);
    _geometryWidget->Reinit( 
        (DimFlags)THREED,
        (VariableFlags)VECTOR
    );

    _layout->addWidget( _geometryWidget, 0 ,0 );
    _layout->addWidget( _copyRegionWidget, 0 ,0 );
    _layout->addWidget( _transformTable, 0 ,0 );

    _params = NULL;
}

void FlowGeometrySubtab::Update( VAPoR::ParamsMgr *paramsMgr,
                                 VAPoR::DataMgr *dataMgr,
                                 VAPoR::RenderParams *rParams) 
{
    _params = dynamic_cast<VAPoR::FlowParams*>(rParams);
    assert(_params);

    _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    _copyRegionWidget->Update(paramsMgr, rParams);
    _transformTable->Update(rParams->GetTransform());
}


FlowAnnotationSubtab::FlowAnnotationSubtab(QWidget* parent) : QVaporSubtab(parent)
{
    _colorbarWidget = new ColorbarWidget(this);
    _layout->addWidget(_colorbarWidget, 0, 0);
}

void FlowAnnotationSubtab::Update(  VAPoR::ParamsMgr *paramsMgr,
                                    VAPoR::DataMgr *dataMgr,
                                    VAPoR::RenderParams *rParams) 
{
    _colorbarWidget->Update(dataMgr, paramsMgr, rParams);
}
