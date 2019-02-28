#pragma once

#include "ui_VolumeAppearanceGUI.h"
#include "ui_VolumeVariablesGUI.h"
#include "ui_VolumeGeometryGUI.h"
#include "ui_VolumeAnnotationGUI.h"
#include "Flags.h"
#include <vapor/MapperFunction.h>

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class VolumeVariablesSubtab : public QWidget, public Ui_VolumeVariablesGUI {
    Q_OBJECT

public:
    VolumeVariablesSubtab(QWidget *parent)
    {
        setupUi(this);
        _variablesWidget->Reinit((VariableFlags)(SCALAR), (DimFlags)(THREED));
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _variablesWidget->Update(dataMgr, paramsMgr, rParams); }
};

class VolumeAppearanceSubtab : public QWidget, public Ui_VolumeAppearanceGUI {
    Q_OBJECT

public:
    VolumeAppearanceSubtab(QWidget *parent)
    {
        setupUi(this);
        _TFWidget->SetOpacityIntegrated(true);
        _TFWidget->Reinit((TFFlags)(CONSTANT_COLOR));
        _isoEdit->SetLabel("Iso Value");
    }

    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
    {
        VAPoR::VolumeParams *vp = (VAPoR::VolumeParams *)rParams;
        _volumeParams = vp;

        _TFWidget->Update(dataMgr, paramsMgr, rParams);

        //------------------------
        // Algorithm Selector
        //------------------------

        string algorithm = vp->GetAlgorithm();
        int    index = _algorithmCombo->findText(QString::fromStdString(algorithm));

        if (index == -1) {
            _algorithmCombo->clear();
            const vector<string> algorithms = VAPoR::VolumeParams::GetAlgorithmNames();
            for (const string &s : algorithms) _algorithmCombo->addItem(QString::fromStdString(s));

            index = _algorithmCombo->findText(QString::fromStdString(algorithm));
        }

        _algorithmCombo->setCurrentIndex(index);

        //------------------------
        // Iso Slider
        //------------------------
        VAPoR::MapperFunction *tf = rParams->GetMapperFunc(rParams->GetVariableName());
        vector<double>         mapRange = tf->getMinMaxMapValue();
        _isoEdit->SetExtents(mapRange[0], mapRange[1]);
    }

private slots:
    void on__algorithmCombo_currentIndexChanged(const QString &text)
    {
        if (!text.isEmpty()) _volumeParams->SetAlgorithm(text.toStdString());
    }

    void on__isoEdit_valueChanged(double value) { _volumeParams->SetIsoValue(value); }

private:
    VAPoR::VolumeParams *_volumeParams;
};

class VolumeGeometrySubtab : public QWidget, public Ui_VolumeGeometryGUI {
    Q_OBJECT

public:
    VolumeGeometrySubtab(QWidget *parent)
    {
        setupUi(this);
        _geometryWidget->Reinit((DimFlags)THREED, (VariableFlags)SCALAR);
    }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams)
    {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
        _copyRegionWidget->Update(paramsMgr, rParams);
        _transformTable->Update(rParams->GetTransform());
    }
};

class VolumeAnnotationSubtab : public QWidget, public Ui_VolumeAnnotationGUI {
    Q_OBJECT

public:
    VolumeAnnotationSubtab(QWidget *parent) { setupUi(this); }

    void Update(VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr, VAPoR::RenderParams *rParams) { _colorbarWidget->Update(dataMgr, paramsMgr, rParams); }
};
