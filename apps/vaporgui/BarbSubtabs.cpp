#include <cmath>
#include "BarbSubtabs.h"
#include "vapor/BarbParams.h"

//#define LENGTH_MIN .01
//#define LENGTH_MAX 1
//#define THICKNESS_MIN .01
//#define THICKNESS_MAX 1
#define COUNT_MIN 1
#define COUNT_MAX 50

#define LENGTH_MIN    1
#define LENGTH_MAX    100
#define THICKNESS_MIN 1
#define THICKNESS_MAX 100

void BarbVariablesSubtab::pushVarStartingWithLetter(vector<string> searchVars, vector<string> &returnVars, char letter)
{
    bool foundDefaultU = false;
    for (auto &element : searchVars) {
        if (element[0] == letter || element[0] == toupper(letter)) {
            returnVars.push_back(element);
            foundDefaultU = true;
            break;
        }
    }
    if (!foundDefaultU) returnVars.push_back(searchVars[0]);
}

void BarbVariablesSubtab::Initialize(VAPoR::BarbParams *bParams, VAPoR::DataMgr *dataMgr)
{
    string nDimsTag = _variablesWidget->getNDimsTag();
    int    ndim = bParams->GetValueLong(nDimsTag, 3);
    assert(ndim == 2 || ndim == 3);

    vector<string> varNames = dataMgr->GetDataVarNames(ndim);
    vector<string> defaultVars;

    if (varNames.size() < 2) return;

    pushVarStartingWithLetter(varNames, defaultVars, 'u');
    pushVarStartingWithLetter(varNames, defaultVars, 'v');

    bParams->SetFieldVariableNames(defaultVars);
}

BarbGeometrySubtab::BarbGeometrySubtab(QWidget *parent)
{
    setupUi(this);
    _geometryWidget->Reinit((GeometryWidget::DimFlags)((GeometryWidget::VECTOR) | (GeometryWidget::TWOD)), GeometryWidget::MINMAX, GeometryWidget::VECTOR);
}

BarbAppearanceSubtab::BarbAppearanceSubtab(QWidget *parent)
{
    setupUi(this);
    _TFWidget->Reinit((TFWidget::Flags)(TFWidget::COLORVAR | TFWidget::CONSTANT));

    _xDimCombo = new Combo(xDimEdit, xDimSlider, true);
    _yDimCombo = new Combo(yDimEdit, yDimSlider, true);
    _zDimCombo = new Combo(zDimEdit, zDimSlider, true);
    _lengthCombo = new Combo(lengthScaleEdit, lengthScaleSlider, true);
    _thicknessCombo = new Combo(thicknessEdit, thicknessSlider, true);

    connect(_xDimCombo, SIGNAL(valueChanged(int)), this, SLOT(xDimChanged(int)));
    connect(_yDimCombo, SIGNAL(valueChanged(int)), this, SLOT(yDimChanged(int)));
    connect(_zDimCombo, SIGNAL(valueChanged(int)), this, SLOT(zDimChanged(int)));
    connect(_lengthCombo, SIGNAL(valueChanged(int)), this, SLOT(lengthChanged(int)));
    connect(_thicknessCombo, SIGNAL(valueChanged(int)), this, SLOT(thicknessChanged(int)));
    connect(_recalcScalesButton, SIGNAL(pressed()), this, SLOT(recalculateScales()));
}

void BarbAppearanceSubtab::_hideZDimWidgets()
{
    zDimLabel->hide();
    zDimSlider->hide();
    zDimEdit->hide();
    zDimLabel->resize(0, 0);
    zDimSlider->resize(0, 0);
    zDimEdit->resize(0, 0);
    tab->adjustSize();
    BarbLayoutTab->adjustSize();
    adjustSize();
}

bool BarbAppearanceSubtab::_isVariable2D() const
{
    VAPoR::Grid *       grid;
    int                 ts, level, lod;
    std::vector<string> varNames = _bParams->GetFieldVariableNames();

    for (int i = 0; i < 3; i++) {
        string varName = varNames[i];
        if (varName == "") { continue; }

        ts = _bParams->GetCurrentTimestep();
        level = _bParams->GetRefinementLevel();
        lod = _bParams->GetCompressionLevel();
        grid = _dataMgr->GetVariable(ts, varName, level, lod);

        int dimSize = grid->GetDimensions().size();
        if (dimSize == 2) return true;
    }

    return false;
}

void BarbAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *bParams)
{
    _dataMgr = dataMgr;
    _bParams = (VAPoR::BarbParams *)bParams;
    _paramsMgr = paramsMgr;
    _TFWidget->Update(dataMgr, paramsMgr, bParams);
    _ColorbarWidget->Update(dataMgr, paramsMgr, bParams);

    vector<long> grid = _bParams->GetGrid();
    _xDimCombo->Update(COUNT_MIN, COUNT_MAX, grid[0]);
    _yDimCombo->Update(COUNT_MIN, COUNT_MAX, grid[1]);
    _zDimCombo->Update(COUNT_MIN, COUNT_MAX, grid[2]);

    vector<double> minExt, maxExt;
    int            length = _bParams->GetLengthScale();
    _lengthCombo->Update(LENGTH_MIN, LENGTH_MAX, length);

    int thickness = _bParams->GetLineThickness();
    _thicknessCombo->Update(THICKNESS_MIN, THICKNESS_MAX, thickness);

    if (_isVariable2D()) _hideZDimWidgets();
}

void BarbAppearanceSubtab::xDimChanged(int i)
{
    vector<long> longDims = _bParams->GetGrid();
    int          dims[3];

    dims[0] = i;
    dims[1] = (int)longDims[1];
    dims[2] = (int)longDims[2];
    _bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::yDimChanged(int i)
{
    vector<long> longDims = _bParams->GetGrid();
    int          dims[3];

    dims[0] = (int)longDims[0];
    dims[1] = i;
    dims[2] = (int)longDims[2];
    _bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::zDimChanged(int i)
{
    vector<long> longDims = _bParams->GetGrid();
    int          dims[3];

    dims[0] = (int)longDims[0];
    dims[1] = (int)longDims[1];
    dims[2] = i;
    _bParams->SetGrid(dims);
}

void BarbAppearanceSubtab::lengthChanged(int d) { _bParams->SetLengthScale(d); }

void BarbAppearanceSubtab::thicknessChanged(int d) { _bParams->SetLineThickness(d); }

void BarbAppearanceSubtab::recalculateScales() { _bParams->SetNeedToRecalculateScales(true); }
