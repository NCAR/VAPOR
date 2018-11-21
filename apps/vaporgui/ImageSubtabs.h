#ifndef IMAGESUBTABS_H
#define IMAGESUBTABS_H

#include "ui_ImageAppearanceGUI.h"
#include "ui_ImageVariablesGUI.h"
#include "ui_ImageGeometryGUI.h"
#include "RangeCombos.h"
#include "vapor/ImageParams.h"
#include "vapor/ResourcePath.h"
#include <QFileDialog>
#include "Flags.h"

namespace VAPoR {
class ControlExec;
class ParamsMgr;
class DataMgr;

//
// ImageVariablesSubtab class
//
class ImageVariablesSubtab : public QWidget, public Ui_ImageVariablesGUI {
    Q_OBJECT

public:
    ImageVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(HEIGHT), (DimFlags)(TWOD));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

//
// ImageAppearanceSubtab class
//
class ImageAppearanceSubtab : public QWidget, public Ui_ImageAppearanceGUI {
    Q_OBJECT

public:
    ImageAppearanceSubtab(QWidget *parent)
    {
        _rParams = NULL;
        setupUi(this);
        _opacityCombo = new Combo(OpacityEdit, OpacitySlider);
        _opacityCombo->SetPrecision(2);

        connect(GeoRefCheckbox, SIGNAL(clicked()), this, SLOT(GeoRefClicked()));
        connect(IgnoreTransparencyCheckbox, SIGNAL(clicked()), this, SLOT(IgnoreTransparencyClicked()));
        connect(_opacityCombo, SIGNAL(valueChanged(double)), this, SLOT(OpacityChanged()));
        connect(SelectImagePushButton, SIGNAL(clicked()), this, SLOT(SelectImage()));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _rParams = (ImageParams *)rParams;

        bool state = _rParams->GetIsGeoRef();
        GeoRefCheckbox->setChecked(state);
        state = _rParams->GetIgnoreTransparency();
        IgnoreTransparencyCheckbox->setChecked(state);

        float opacity = _rParams->GetConstantOpacity();
        _opacityCombo->Update(0.0, 1.0, opacity);

        std::string imageFile = _rParams->GetImagePath();
        SelectImageEdit->setText(QString::fromStdString(imageFile));
    }

    ~ImageAppearanceSubtab()
    {
        if (_opacityCombo) {
            delete _opacityCombo;
            _opacityCombo = NULL;
        }
    }

private slots:
    void GeoRefClicked() { _rParams->SetIsGeoRef(GeoRefCheckbox->isChecked()); }

    void IgnoreTransparencyClicked() { _rParams->SetIgnoreTransparency(IgnoreTransparencyCheckbox->isChecked()); }

    void OpacityChanged()
    {
        float opacity = _opacityCombo->GetValue();
        _rParams->SetConstantOpacity(opacity);
    }

    void SelectImage()
    {
        std::string installedImagePath = Wasp::GetSharePath("images");
        QString     fileName =
            QFileDialog::getOpenFileName(this, tr("Specify installed image to load"), QString::fromStdString(installedImagePath), tr("TIFF files, tiled images (*.tiff *.tif *.gtif *.tms)"));
        if (!fileName.isNull())    // upon cancel, QFileDialog returns a Null string.
        {
            SelectImageEdit->setText(fileName);
            _rParams->SetImagePath(fileName.toStdString());
        }
    }

private:
    ImageParams *_rParams;

    Combo *_opacityCombo;
};

//
// ImageGeometrySubtab class
//
class ImageGeometrySubtab : public QWidget, public Ui_ImageGeometryGUI {
    Q_OBJECT

public:
    ImageGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit((DimFlags)TWOD, (GeometryFlags)MINMAX, (VariableFlags)SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }

private:
};

}    // namespace VAPoR

#endif
