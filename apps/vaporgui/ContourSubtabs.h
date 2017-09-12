#ifndef CONTOURSUBTABS_H
#define CONTOURSUBTABS_H

#include "vapor/ContourParams.h"
#include "ui_ContourAppearanceGUI.h"
#include "ui_ContourVariablesGUI.h"
#include "ui_ContourGeometryGUI.h"
#include "RangeCombos.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
class DataMgr;
} // namespace VAPoR

class ContourVariablesSubtab : public QWidget, public Ui_ContourVariablesGUI {

    Q_OBJECT

  public:
    ContourVariablesSubtab(QWidget *parent) {
        setupUi(this);
        _variablesWidget->Reinit((VariablesWidget::DisplayFlags)(VariablesWidget::SCALAR | VariablesWidget::HGT),
                                 (VariablesWidget::DimFlags)(VariablesWidget::THREED | VariablesWidget::TWOD));
    }

    void Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams) {
        _variablesWidget->Update(dataMgr, paramsMgr, rParams);
    }
};

class ContourAppearanceSubtab : public QWidget, public Ui_ContourAppearanceGUI {

    Q_OBJECT

  public:
    ContourAppearanceSubtab(QWidget *parent) {
        setupUi(this);

        _TFWidget->Reinit((TFWidget::Flags)(0));
        _TFWidget->mappingFrame->setIsolineSliders(true);
        _TFWidget->mappingFrame->setOpacityMapping(false);

        _lineWidthCombo = new Combo(lineWidthEdit, lineWidthSlider);
        _countCombo = new Combo(contourCountEdit, contourCountSlider, true);
        _cMinCombo = new Combo(contourMinEdit, contourMinSlider);
        _spacingCombo = new Combo(contourSpacingEdit, contourSpacingSlider);

        connect(_lineWidthCombo, SIGNAL(valueChanged(double)), this,
                SLOT(SetLineThickness(double)));
        connect(_countCombo, SIGNAL(valueChanged(int)), this,
                SLOT(SetContourCount(int)));
        connect(_cMinCombo, SIGNAL(valueChanged(double)), this,
                SLOT(SetContourMinimum(double)));
        connect(_spacingCombo, SIGNAL(valueChanged(double)), this,
                SLOT(SetContourSpacing(double)));
        connect(lockToTFCheckbox, SIGNAL(toggled(bool)), this,
                SLOT(LockToTFChecked(bool)));

        connect(_TFWidget->mappingFrame, SIGNAL(mappingChanged()), this,
                SLOT(MappingChanged()));
        connect(_TFWidget->mappingFrame, SIGNAL(updateParams()), this,
                SLOT(UpdateParams()));
        connect(_TFWidget, SIGNAL(emitChange()), this,
                SLOT(EndTFChange()));
    }

    double GetContourMinOrMax(bool minOrMax) {
        // Apply params to lockTF checkbox
        //
        bool locked = _cParams->GetLockToTF();
        lockToTFCheckbox->setChecked(locked);

        // Apply params to contour minimum.  Restrict the minimum according
        // to the transfer function if we're locking to it (via 'locked'
        // parameter).  If not, restrict according to variable min/max.
        //
        double min = _cParams->GetContourMin();
        string varname = _cParams->GetVariableName();
        double spacing, maxSpacing;
        if (locked) {
            cout << "LOCKED" << endl;
            // Update contour minimum combo
            //
            VAPoR::MapperFunction *mf = _cParams->GetMapperFunc(varname);
            double lower = mf->getMinMapValue();
            double upper = mf->getMaxMapValue();
            if (minOrMax)
                return upper;
            else
                return lower;
        } else {
            cout << "NOT LOCKED" << endl;
            // Apply settings to contour minimum and spacing, bounded only
            // by the min/max values of the variable.
            //
            int lod = _cParams->GetCompressionLevel();
            int level = _cParams->GetRefinementLevel();
            int ts = _cParams->GetCurrentTimestep();
            VAPoR::StructuredGrid *var = _dataMgr->GetVariable(ts, varname, level, lod);
            float range[2];
            var->GetRange(range);
            if (minOrMax)
                return range[1];
            else
                return range[0];
        }
    }

    void Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams) {
        _cParams = (VAPoR::ContourParams *)rParams;
        _dataMgr = dataMgr;
        _paramsMgr = paramsMgr;

        _TFWidget->Update(dataMgr, paramsMgr, _cParams);
        _ColorbarWidget->Update(dataMgr, paramsMgr, _cParams);

        // Apply params to lineThickness, along with system supported
        // minima and maxima for line width
        //
        _cParams = (VAPoR::ContourParams *)rParams;
        GLfloat lineWidthRange[2] = {0.f, 0.f};
        glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
        _lineWidthCombo->Update(lineWidthRange[0], lineWidthRange[1],
                                _cParams->GetLineThickness());

        bool lock = _cParams->GetTFLock();
        lockToTFCheckbox->setChecked(lock);
        // If we're locked, disable sliders and edits
        //
        contourSpacingSlider->setEnabled(!lock);
        contourSpacingEdit->setEnabled(!lock);
        contourMinSlider->setEnabled(!lock);
        contourMinEdit->setEnabled(!lock);

        // Apply params to contour count
        //
        int numContours = _cParams->GetNumContours();
        _countCombo->Update(1, 50, numContours);

        // Update contour spacing and minimum settings, which may
        // or may not be locked within the transfer function bounds.
        //
        double minComboMin = GetContourMinOrMax(false);
        double minComboMax = GetContourMinOrMax(true);
        double minVal = _cParams->GetContourMin();
        _cMinCombo->Update(minComboMin, minComboMax, minVal);

        double spacing = _cParams->GetContourSpacing();
        double maxSpacing = (minComboMax - minVal) / (double)(numContours - 1);
        if (spacing > maxSpacing)
            spacing = maxSpacing;
        _spacingCombo->Update(0, maxSpacing, spacing);
    }

    void initialize(VAPoR::ContourParams *cParams) {
        _cParams = cParams;
        string varname = _cParams->GetVariableName();
        cout << "initializing varname " << varname << endl;
        VAPoR::MapperFunction *mf = _cParams->GetMapperFunc(varname);
        double lower = mf->getMinMapValue();
        double upper = mf->getMaxMapValue();
        int count = 3;
        double spacing = (upper - lower) / (double)(count - 1);

        _cParams->SetNumContours(count);
        _cParams->SetContourMin(lower);
        _cParams->SetContourSpacing(spacing);
        SetIsovalues();
    }

  private:
    VAPoR::ContourParams *_cParams;
    VAPoR::DataMgr *_dataMgr;
    VAPoR::ParamsMgr *_paramsMgr;
    Combo *_lineWidthCombo;
    Combo *_countCombo;
    Combo *_cMinCombo;
    Combo *_spacingCombo;

    void SetIsovalues() {
        vector<double> cVals;
        int numContours = _cParams->GetNumContours();
        double spacing = _cParams->GetContourSpacing();
        double min = _cParams->GetContourMin();
        for (size_t i = 0; i < numContours; i++) {
            cVals.push_back(min + spacing * i);
        }
        _cParams->SetIsovalues(cVals);
    }

  private slots:
    void MappingChanged() {
        cout << "mapping changed!" << endl;
    }

    void EndTFChange() {
        double min = GetContourMinOrMax(false);
        double max = GetContourMinOrMax(true);
        double minVal = _cParams->GetContourMin();
        if (minVal < min)
            minVal = min;
        if (minVal > max)
            minVal = max;
        cout << "TF min " << minVal << endl;
        _cMinCombo->Update(min, max, minVal);
        _cParams->SetContourMin(minVal);

        int numContours = _cParams->GetNumContours();
        double spacing = _cParams->GetContourSpacing();
        double maxSpacing = (max - minVal) / (double)(numContours - 1);
        double span = spacing * (numContours - 1) + minVal;
        //if ((spacing > maxSpacing) || (_cParams->GetLockToTF())) {
        if ((span > max) || (_cParams->GetLockToTF())) {
            spacing = maxSpacing;
            cout << "set sp to: " << spacing << endl;
        }
        _spacingCombo->Update(0, maxSpacing, spacing);
        _cParams->SetContourSpacing(spacing);

        SetIsovalues();
    }

    void UpdateParams() {
    }
    void SetLineThickness(double val) {
        _cParams->SetLineThickness(val);
    }

    void SetContourCount(int count) {
        bool locked = _cParams->GetLockToTF();
        string varname = _cParams->GetVariableName();

        // If we're locked to the transfer function and our span exceeds
        // the TF max value, adjust our spacing to make room for the added
        // contours
        //
        if (locked) {
            VAPoR::MapperFunction *mf = _cParams->GetMapperFunc(varname);
            double lower = mf->getMinMapValue();
            double upper = mf->getMaxMapValue();
            double spacing = (upper - lower) / (double)(count - 1);
            _cParams->SetContourSpacing(spacing);
        }
        _cParams->SetNumContours(count);
        SetIsovalues();
    }

    void SetContourMinimum(double min) {
        double minRange = GetContourMinOrMax(0);
        double maxRange = GetContourMinOrMax(1);

        if (min < minRange)
            min = minRange;
        if (min > maxRange)
            min = maxRange;

        int numContours = _cParams->GetNumContours();
        double spacing = (maxRange - min) / (double)(numContours - 1);
        _cParams->SetContourSpacing(spacing);

        double test = GetContourMinOrMax(0);
        cout << minRange << " ! " << maxRange << endl;

        _cParams->SetContourMin(min);

        SetIsovalues();
    }

    void SetContourSpacing(double spacing) {
        bool locked = _cParams->GetLockToTF();

        double min = _cParams->GetContourMin();
        string varname = _cParams->GetVariableName();
        int numContours = _cParams->GetNumContours();
        double maxSpacing;

        if (numContours <= 1) {
            _cParams->SetContourSpacing(1);
            return;
        }

        int lod = _cParams->GetCompressionLevel();
        int level = _cParams->GetRefinementLevel();
        int ts = _cParams->GetCurrentTimestep();
        VAPoR::StructuredGrid *var = _dataMgr->GetVariable(ts, varname, level, lod);
        float range[2];
        var->GetRange(range);
        maxSpacing = (range[1] - range[0]) / (double)(numContours - 1);
        if (spacing > maxSpacing)
            spacing = maxSpacing;
        _cParams->SetContourSpacing(spacing);
        SetIsovalues();
    }

    void LockToTFChecked(bool checked) {
        _cParams->SetLockToTF(checked);
        if (checked) {
            EndTFChange();
            //		string varname = _cParams->GetVariableName();
            //		int lod = _cParams->GetCompressionLevel();
            //		int level = _cParams->GetRefinementLevel();
            //		int ts = _cParams->GetCurrentTimestep();
            //		VAPoR::StructuredGrid* var = _dataMgr->GetVariable(ts, varname, level, lod);
            //		float range[2];
            //		var->GetRange(range);
            //		_cParams->SetContourMin(range[0]);

            //		int numContours = _cParams->GetNumContours();
            //		double spacing = (range[1] - range[0]) / ((double)numContours-1);
            //		_cParams->SetContourSpacing(spacing);
            //		SetIsovalues();
        }
    }
};

class ContourGeometrySubtab : public QWidget, public Ui_ContourGeometryGUI {

    Q_OBJECT

  public:
    ContourGeometrySubtab(QWidget *parent) {
        setupUi(this);
        _geometryWidget->Reinit(
            GeometryWidget::TWOD);
    }

    void Update(
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::DataMgr *dataMgr,
        VAPoR::RenderParams *rParams) {
        _geometryWidget->Update(paramsMgr, dataMgr, rParams);
    }

  private:
};

#endif //CONTOURSUBTABS_H
