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
    }

    void updateSpacingAndMin(VAPoR::DataMgr *dataMgr) {
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
            // Update contour minimum combo
            //
            VAPoR::MapperFunction *mf = _cParams->GetMapperFunc(varname);
            double lower = mf->getMinMapValue();
            double upper = mf->getMaxMapValue();
            _cMinCombo->Update(lower, upper, min);

            // Update spacing combo
            //
            spacing = _cParams->GetContourSpacing();
            int numContours = _cParams->GetNumContours();
            double maxSpacing = (upper - min) / numContours;
            double maxContour = min + spacing * numContours;
            //cout << "spacing " << spacing << endl;
            //cout << "m/l/u   " << min << " " << lower << " " << upper << endl;
            //cout << "#/Ms/Mc " << numContours << " " << maxSpacing << " " << maxContour << endl;
            if (maxContour > upper) {
                spacing = maxSpacing;
                //	cout << "spacing shift " << spacing << endl;
            }
            _spacingCombo->Update(0, maxSpacing, spacing);
        } else {
            // Apply settings to contour minimum and spacing, bounded only
            // by the min/max values of the variable.
            //
            int lod = _cParams->GetCompressionLevel();
            int level = _cParams->GetRefinementLevel();
            int ts = _cParams->GetCurrentTimestep();
            VAPoR::StructuredGrid *var = dataMgr->GetVariable(ts, varname, level, lod);
            float range[2];
            var->GetRange(range);

            // Apply params to contour spacing
            //
            spacing = _cParams->GetContourSpacing();
            maxSpacing = range[1] - range[0];
            _spacingCombo->Update(0, maxSpacing, spacing);
            _cMinCombo->Update(range[0], range[1], min);
        } // Whew!
    }

    void Update(
        VAPoR::DataMgr *dataMgr,
        VAPoR::ParamsMgr *paramsMgr,
        VAPoR::RenderParams *rParams) {
        _cParams = (VAPoR::ContourParams *)rParams;

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

        // Apply params to contour count
        //
        int numContours = _cParams->GetNumContours();
        _countCombo->Update(1, 50, numContours);

        // Update contour spacing and minimum settings, which may
        // or may not be locked within the transfer function bounds.
        //
        updateSpacingAndMin(dataMgr);

        // Finally apply contour/isoline parameters to our TFWidget's
        // mapping frame, based off of ContourParams.  This is not done
        // during _TFWidget::Update() because TFWidget can only query
        // RenderParams get methods, not those of ContourParams.
        //
        vector<double> cVals;
        double spacing = _cParams->GetContourSpacing();
        double min = _cParams->GetContourMin();
        for (size_t i = 0; i < numContours; i++) {
            cVals.push_back(min + spacing * i);
        }
        _cParams->SetIsovalues(cVals);
    }

  private:
    VAPoR::ContourParams *_cParams;
    Combo *_lineWidthCombo;
    Combo *_countCombo;
    Combo *_cMinCombo;
    Combo *_spacingCombo;

  private slots:
    void SetLineThickness(double val) {
        _cParams->SetLineThickness(val);
    }

    void SetContourCount(int count) {
        _cParams->SetNumContours(count);
        /*		double min = _cParams->GetContourMin();
		double spacing = _cParams->GetContourSpacing();

		vector<double> sliderVals;
		for (size_t i=0; i<count; i++) {
			sliderVals.push_back(min + spacing*i);
		}
		
		_TFWidget->mappingFrame->setIsolineSliders(sliderVals);
		_cParams->SetNumContours(count);

		cout << "cParams num contours: " << _cParams->GetNumContours() << endl;
*/
    }

    void SetContourMinimum(double min) {
        _cParams->SetContourMin(min);
    }

    void SetContourSpacing(double spacing) {
        _cParams->SetContourSpacing(spacing);
    }

    void LockToTFChecked(bool checked) {
        _cParams->SetLockToTF(checked);
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
