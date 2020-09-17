#ifndef IMAGESUBTABS_H
#define IMAGESUBTABS_H

#include <QFileDialog>
#include "ui_ImageAppearanceGUI.h"
#include "ui_ImageGeometryGUI.h"
#include "vapor/ImageParams.h"
#include "vapor/ResourcePath.h"
#include "vapor/GeoImageTMS.h"
#include "Flags.h"
#include "PGroup.h"
#include "VComboBox.h"
#include "PFidelitySection.h"
#include "PVariableWidgets.h"
#include "PEnumDropdown.h"

namespace VAPoR {
	class ControlExec;
	class ParamsMgr;
	class DataMgr;

class ImageVariablesSubtab : public QWidget {

	Q_OBJECT
    PGroup *_pg;
    
public:
	ImageVariablesSubtab(QWidget* parent) 
  { 
      setLayout( new QVBoxLayout );
      ((QVBoxLayout*)layout())->insertWidget(1, _pg = new PGroup);
      PSection *vars = new PSection("Variable Selection");
      vars->Add(new PHeightVariableSelectorHLI);
      _pg->Add(vars);
      _pg->Add(new PFidelitySection);
	}

	void Update(  VAPoR::DataMgr *dataMgr,
		            VAPoR::ParamsMgr *paramsMgr,
		            VAPoR::RenderParams *rParams) 
    {
      _pg->Update(rParams, paramsMgr, dataMgr);
    }
};

//
// PWidget for selecting TMS file level-of-detail
//

class PTMSLODInput : public PLineItem {
    Q_OBJECT
    VComboBox* _vComboBox;
public:
    PTMSLODInput() : PLineItem("", "TMS level of detail", _vComboBox = new VComboBox({"0"})) {
        connect( _vComboBox, &VComboBox::IndexChanged, this, &PTMSLODInput::dropdownIndexChanged );
    }

protected:
    virtual void updateGUI() const override {
        ImageParams* rp = dynamic_cast<ImageParams*>( getParams() );
        VAssert( rp && "Params must be ImageParams" );

        std::string imageFile = rp->GetImagePath();
        if ( IsTMSFile( imageFile ) ) {
            _vComboBox->setEnabled( true );
        }
        else {
            _vComboBox->setEnabled( false );             // Disable if not using a TMS image
            return;
        }

        std::vector< std::string > options{"default"};
        int lods = GetAvailableTMSLODs( imageFile );
        for ( int i=0; i<lods; i++ ) {
            options.push_back( std::to_string( i ) );
        }
        _vComboBox->SetOptions( options );
        _vComboBox->SetIndex( rp->GetTMSLOD()+1 );
    };

private slots:
    void dropdownIndexChanged( int i ) {
        ImageParams* rp = dynamic_cast<ImageParams*>( getParams() );
        VAssert( rp && "Params must be ImageParams" );
        rp->SetTMSLOD( i-1 );
    }
};

//
// ImageAppearanceSubtab class
//
class ImageAppearanceSubtab : public QWidget, public Ui_ImageAppearanceGUI {

	Q_OBJECT

public:
	ImageAppearanceSubtab(QWidget* parent) 
    {
    _rParams = NULL;
    setupUi(this);
    
    _TMSLODInput = new PTMSLODInput();
    ((QVBoxLayout*)layout())->insertWidget( 4, _TMSLODInput );
        
    _opacityCombo = new Combo( OpacityEdit, OpacitySlider );
    _opacityCombo->SetPrecision( 2 );

    connect( GeoRefCheckbox, SIGNAL(clicked()), this, SLOT( GeoRefClicked() ) );
    connect( IgnoreTransparencyCheckbox, SIGNAL(clicked()), this, SLOT( IgnoreTransparencyClicked() ) );
    connect( _opacityCombo, SIGNAL(valueChanged( double )), this, SLOT( OpacityChanged() ));
    connect( SelectImagePushButton, SIGNAL(clicked()), this, SLOT( SelectImage() ) );
	}

	void Update(  VAPoR::DataMgr*       dataMgr,
		            VAPoR::ParamsMgr*     paramsMgr,
		            VAPoR::RenderParams*  rParams) 
  {
    _rParams = (ImageParams*) rParams;
    
    _TMSLODInput->Update( _rParams, paramsMgr, dataMgr );

    bool state = _rParams->GetIsGeoRef();
    GeoRefCheckbox->setChecked(state);
    state = _rParams->GetIgnoreTransparency();
    IgnoreTransparencyCheckbox->setChecked( state );
  
    float opacity = _rParams->GetConstantOpacity();
    _opacityCombo->Update( 0.0, 1.0, opacity );

    std::string imageFile = _rParams->GetImagePath();
    SelectImageEdit->setText( QString::fromStdString( imageFile ) );
	}

  ~ImageAppearanceSubtab()
  {
    if( _opacityCombo )
    {
      delete _opacityCombo;
      _opacityCombo = NULL;
    }
  }

private slots:
  void GeoRefClicked()
  {
    _rParams->SetIsGeoRef( GeoRefCheckbox->isChecked() );
  }

  void IgnoreTransparencyClicked()
  {
    _rParams->SetIgnoreTransparency( IgnoreTransparencyCheckbox->isChecked() );
  }

  void OpacityChanged()  
  {
    float opacity = _opacityCombo->GetValue();
    _rParams->SetConstantOpacity( opacity );
  }

  void SelectImage()
  {
    std::string installedImagePath = Wasp::GetSharePath("images");
    QString fileName = QFileDialog::getOpenFileName( this, 
                          tr("Specify installed image to load"),
                          QString::fromStdString(installedImagePath),
                          tr("TIFF files, tiled images (*.tiff *.tif *.gtif *.tms)") );
    if( !fileName.isNull() )    // upon cancel, QFileDialog returns a Null string.
    {
        SelectImageEdit->setText(fileName);
        _rParams->SetImagePath( fileName.toStdString() );
    }
  }

private:
  PTMSLODInput* _TMSLODInput;
  ImageParams* _rParams;

  Combo* _opacityCombo;
};


//
// ImageGeometrySubtab class
//
class ImageGeometrySubtab : public QWidget, public Ui_ImageGeometryGUI {

	Q_OBJECT

public:
	ImageGeometrySubtab(QWidget* parent) 
  {
		setupUi(this);
		_geometryWidget->Reinit( 
			(DimFlags)TWOD,
			(VariableFlags)SCALAR
		);
	}
	
	void Update(  VAPoR::ParamsMgr *paramsMgr,
		            VAPoR::DataMgr *dataMgr,
		            VAPoR::RenderParams *rParams) 
  {
		_geometryWidget->Update(paramsMgr, dataMgr, rParams);
		_copyRegionWidget->Update(paramsMgr, rParams);
		_transformTable->Update(rParams->GetTransform());
	}


private:

};

}

#endif 
