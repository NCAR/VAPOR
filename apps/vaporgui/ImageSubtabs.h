#ifndef IMAGESUBTABS_H
#define IMAGESUBTABS_H

#include "ui_ImageAppearanceGUI.h"
#include "ui_ImageVariablesGUI.h"
#include "ui_ImageGeometryGUI.h"
#include "RangeCombos.h"
#include "vapor/ImageParams.h"
#include "vapor/GetAppPath.h"
#include <QFileDialog>

namespace VAPoR {
class ControlExec;
class ParamsMgr;
class DataMgr;

//
// A derived QDoubleValidator that also implements the fixup function.
//
class VaporDoubleValidator : public QDoubleValidator {
  public:
    VaporDoubleValidator(QObject *parent = 0)
        : QDoubleValidator(parent = 0) {}
    VaporDoubleValidator(double bottom, double top, int decimals, QObject *parent = 0)
        : QDoubleValidator(bottom, top, decimals, parent = 0) {}

    //
    // overload fixup() from QValidator
    //
    void fixup(QString &input) const {
        double val = input.toFloat();
        if (val > top())
            val = top();
        else if (val < bottom())
            val = bottom();

        input = QString::number(val);
    }
};

//
// ImageVariablesSubtab class
//
class ImageVariablesSubtab : public QWidget, public Ui_ImageVariablesGUI {

    Q_OBJECT

  public:
    ImageVariablesSubtab(QWidget *parent) {
        setupUi(this);
        _variablesWidget->Reinit(VariablesWidget::HGT, VariablesWidget::TWOD);
    }

    void Update(VAPoR::DataMgr *dataMgr,
                VAPoR::ParamsMgr *paramsMgr,
                VAPoR::RenderParams *rParams) {
        _variablesWidget->Update(dataMgr, paramsMgr, rParams);
    }
};

//
// ImageAppearanceSubtab class
//
class ImageAppearanceSubtab : public QWidget, public Ui_ImageAppearanceGUI {

    Q_OBJECT

  public:
    ImageAppearanceSubtab(QWidget *parent) {
        _rParams = NULL;
        setupUi(this);
        _opacityCombo = new Combo(OpacityEdit, OpacitySlider);

        connect(GeoRefCheckbox, SIGNAL(clicked()), this, SLOT(GeoRefClicked()));
        connect(IgnoreTransparencyCheckbox, SIGNAL(clicked()), this, SLOT(IgnoreTransparencyClicked()));
        connect(_opacityCombo, SIGNAL(valueChanged(double)), this, SLOT(OpacityChanged()));
        connect(SelectImagePushButton, SIGNAL(clicked()), this, SLOT(SelectImage()));
    }

    void Update(VAPoR::DataMgr *dataMgr,
                VAPoR::ParamsMgr *paramsMgr,
                VAPoR::RenderParams *rParams) {
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

    ~ImageAppearanceSubtab() {
        if (_opacityCombo) {
            delete _opacityCombo;
            _opacityCombo = NULL;
        }
    }

  private slots:
    void GeoRefClicked() {
        _rParams->SetIsGeoRef(GeoRefCheckbox->isChecked());
    }

    void IgnoreTransparencyClicked() {
        _rParams->SetIgnoreTransparency(IgnoreTransparencyCheckbox->isChecked());
    }

    void OpacityChanged() {
        float opacity = _opacityCombo->GetValue();
        _rParams->SetConstantOpacity(opacity);
    }

    void SelectImage() {
        std::vector<std::string> paths;
        paths.push_back("images");
        std::string installedImagePath = Wasp::GetAppPath("VAPOR", "share", paths);
        QString fileName = QFileDialog::getOpenFileName(this,
                                                        tr("Specify installed image to load"),
                                                        QString::fromStdString(installedImagePath),
                                                        tr("TIFF files, tiled images (*.tiff *.tif *.gtif *.tms)"));
        SelectImageEdit->setText(fileName);
        _rParams->SetImagePath(fileName.toStdString());
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
    ImageGeometrySubtab(QWidget *parent) {
        setupUi(this);
        _geometryWidget->Reinit(GeometryWidget::TWOD);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr,
                VAPoR::DataMgr *dataMgr,
                VAPoR::RenderParams *rParams) {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    }

  private:
};

} // namespace VAPoR

#endif
