#ifndef IMAGESUBTABS_H
#define IMAGESUBTABS_H

#include "ImageAppearanceGUI.h"
#include "ImageVariablesGUI.h"
#include "ImageGeometryGUI.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

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
        setupUi(this);
        connect(GeoreferenceCheckbox, SIGNAL(clicked()), this, SLOT(GeoreferenceClicked()));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) {}

private slots:
    void GeoreferenceClicked() { std::cout << "I'm clicked" << std::endl; }
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

#endif
