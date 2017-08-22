#ifndef IMAGESUBTABS_H
#define IMAGESUBTABS_H

#include "ImageAppearanceGUI.h"
#include "ImageVariablesGUI.h"
#include "ImageGeometryGUI.h"
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
    VaporDoubleValidator(QObject *parent = 0) : QDoubleValidator(parent = 0) {}
    VaporDoubleValidator(double bottom, double top, int decimals, QObject *parent = 0) : QDoubleValidator(bottom, top, decimals, parent = 0) {}

    //
    // overload fixup() from QValidator
    //
    void fixup(QString &input) const
    {
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
    ImageVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit(VariablesWidget::HGT, VariablesWidget::TWOD);
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
        connect(GeoTIFFCheckbox, SIGNAL(clicked()), this, SLOT(GeoTIFFClicked()));
        connect(IgnoreTransparencyCheckbox, SIGNAL(clicked()), this, SLOT(IgnoreTransparencyClicked()));

        OpacityEdit->setValidator(new VaporDoubleValidator(0, 1.0, 2, this));    // ranging from 0 to 1
                                                                                 // with 2 decimals
        connect(OpacitySlider, SIGNAL(sliderReleased()), this, SLOT(OpacityChanged1()));
        connect(OpacityEdit, SIGNAL(returnPressed()), this, SLOT(OpacityChanged2()));

        connect(SelectImagePushButton, SIGNAL(clicked()), this, SLOT(SelectImage()));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        _rParams = (ImageParams *)rParams;

        bool state = _rParams->GetIsGeoTIFF();
        GeoTIFFCheckbox->setChecked(state);
        state = _rParams->GetIgnoreTransparency();
        IgnoreTransparencyCheckbox->setChecked(state);

        float opacity = _rParams->GetConstantOpacity();
        OpacityEdit->setText(QString::number(opacity));
        OpacitySlider->setSliderPosition((int)(opacity * 100.0));

        std::string imageFile = _rParams->GetImagePath();
        SelectImageEdit->setText(QString::fromStdString(imageFile));
    }

private slots:
    void GeoTIFFClicked() { _rParams->SetIsGeoTIFF(GeoTIFFCheckbox->isChecked()); }

    void IgnoreTransparencyClicked() { _rParams->SetIgnoreTransparency(IgnoreTransparencyCheckbox->isChecked()); }

    void OpacityChanged1()    // triggered by the slider
    {
        float opacity = OpacitySlider->sliderPosition() / 100.0;
        OpacityEdit->setText(QString::number(opacity));
        _rParams->SetConstantOpacity(opacity);
    }

    void OpacityChanged2()    // triggered by the text box
    {
        QString userInput = OpacityEdit->text();
        OpacityEdit->validator()->fixup(userInput);
        OpacityEdit->setText(userInput);
        float opacity = userInput.toFloat();
        OpacitySlider->setSliderPosition((int)(opacity * 100.0));
        _rParams->SetConstantOpacity(opacity);
    }

    void SelectImage()
    {
        std::vector<std::string> paths;
        paths.push_back("images");
        std::string installedImagePath = Wasp::GetAppPath("VAPOR", "share", paths);
        QString     fileName =
            QFileDialog::getOpenFileName(this, tr("Specify installed image to load"), QString::fromStdString(installedImagePath), tr("TIFF files, tiled images (*.tiff *.tif *.gtif *.tms)"));
        SelectImageEdit->setText(fileName);
        _rParams->SetImagePath(fileName.toStdString());
    }

private:
    ImageParams *_rParams;
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
        _geometryWidget->Reinit(GeometryWidget::TWOD);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _geometryWidget->Update(paramsMgr, dataMgr, rParams); }

private:
};

}    // namespace VAPoR

#endif
