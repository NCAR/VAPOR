
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/ContourParams.h>

namespace VAPoR {

//
// Register class with object factory!!!
//
static RenParamsRegistrar<ContourParams>        registrar(ContourParams::GetClassType());
static ParamsRegistrar<ContourParams::Contours> registrar2(ContourParams::Contours::GetClassType());

const string ContourParams::_thicknessScaleTag = "LineThickness";
const string ContourParams::_contoursTag = "Contours";
const string ContourParams::_numDigitsTag = "NumDigits";
const string ContourParams::_textDensityTag = "TextDensity";
const string ContourParams::_lineColorTag = "LineColor";
const string ContourParams::_textEnabledTag = "TextEnabled";
const string ContourParams::_lockToTFTag = "LockToTF";
const string ContourParams::Contours::_valuesTag = "Values";

ContourParams::ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, ContourParams::GetClassType(), 3)
{
    SetDiagMsg("ContourParams::ContourParams() this=%p", this);

    _contours = new ParamsContainer(ssave, _contoursTag);
    _contours->SetParent(this);

    _init();
}

ContourParams::ContourParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3)
{
    SetDiagMsg("ContourParams::ContourParams() this=%p", this);

    if (node->HasChild(_contoursTag)) {
        _contours = new ParamsContainer(ssave, node->GetChild(_contoursTag));
    } else {
        // Node doesn't contain a contours container
        _contours = new ParamsContainer(ssave, _contoursTag);
        _contours->SetParent(this);
    }
}

ContourParams::ContourParams(const ContourParams &rhs) : RenderParams(rhs) { _contours = new ParamsContainer(*(rhs._contours)); }

ContourParams &ContourParams::operator=(const ContourParams &rhs)
{
    if (_contours) delete _contours;

    ParamsBase::operator=(rhs);
    _contours = new ParamsContainer(*(rhs._contours));

    return (*this);
}

ContourParams::~ContourParams()
{
    SetDiagMsg("ContourParams::~ContourParams() this=%p", this);

    if (_contours != NULL) {
        delete _contours;
        _contours = NULL;
    }
}

vector<double> ContourParams::GetIsoValues(const string &variable) { return GetContourValues(variable); }

void ContourParams::SetIsoValues(const string &variable, const vector<double> &values) { SetContourValues(variable, values); }

vector<double> ContourParams::GetContourValues(const string &varName)
{
    Contours *c = (Contours *)_contours->GetParams(varName);
    if (c == NULL) {
        bool wasEnabled = _ssave->GetEnabled();
        _ssave->SetEnabled(false);
        MakeNewContours(varName);
        _ssave->SetEnabled(wasEnabled);
        c = (Contours *)_contours->GetParams(varName);
    }
    return c->GetContourValues();
}

void ContourParams::SetContourValues(const string &varName, const vector<double> &vals)
{
    Contours *c = (Contours *)_contours->GetParams(varName);
    if (c == NULL) {
        MakeNewContours(varName);
        c = (Contours *)_contours->GetParams(varName);
    }
    c->SetContourValues(vals);
}

ContourParams::Contours *ContourParams::GetCurrentContours()
{
    string                   varName = GetVariableName();
    ContourParams::Contours *c = (ContourParams::Contours *)_contours->GetParams(varName);
    if (c == NULL) {
        MakeNewContours(varName);
        c = (ContourParams::Contours *)_contours->GetParams(varName);
    }
    return c;
}

void ContourParams::MakeNewContours(string varName)
{
    Contours newContours(_ssave);

    MapperFunction *mf = GetMapperFunc(varName);
    VAssert(mf);
    vector<double> minMax = mf->getMinMaxMapValue();
    int            numContours = newContours.GetContourValues().size();
    double         spacing = (minMax[1] - minMax[0]) / (numContours - 1);

    GenerateContourValues(minMax[0], spacing, numContours, &newContours);

    _contours->Insert(&newContours, varName);
}

void ContourParams::GenerateContourValues(double start, double spacing, int num, Contours *c)
{
    if (!c) c = GetCurrentContours();

    vector<double> vals;
    for (int i = 0; i < num; i++) vals.push_back(start + i * spacing);
    c->SetContourValues(vals);
}

// Set everything to default values
void ContourParams::_init()
{
    SetDiagMsg("ContourParams::_init()");

    float rgb[] = {1., 1., 1.};
    SetConstantColor(rgb);

    GetBox()->SetPlanar(true);
    GetBox()->SetOrientation(VAPoR::Box::XY);
    SetDefaultVariables(2, true);
}

int ContourParams::GetContourCount()
{
    Contours *     c = GetCurrentContours();
    vector<double> vals = c->GetContourValues();
    return vals.size();
}

double ContourParams::GetContourMin()
{
    Contours *     c = GetCurrentContours();
    vector<double> vals = c->GetContourValues();
    return vals[0];
}

double ContourParams::GetContourMax()
{
    Contours *     c = GetCurrentContours();
    vector<double> vals = c->GetContourValues();
    return vals[vals.size() - 1];
}

double ContourParams::GetContourSpacing()
{
    Contours *     c = GetCurrentContours();
    vector<double> vals = c->GetContourValues();

    if (vals.size() < 2)
        return 1;
    else
        return vals[1] - vals[0];
}

void ContourParams::SetContourCount(int num) { GenerateContourValues(GetContourMin(), GetContourSpacing(), num); }

void ContourParams::SetContourMin(double val) { GenerateContourValues(val, GetContourSpacing(), GetContourCount()); }

void ContourParams::SetContourSpacing(double val) { GenerateContourValues(GetContourMin(), val, GetContourCount()); }

void ContourParams::GetLineColor(int lineNum, float color[3])
{
    if (UseSingleColor()) {
        GetConstantColor(color);
    } else {
        string          varName = GetVariableName();
        MapperFunction *tf = 0;
        tf = (MapperFunction *)GetMapperFunc(varName);
        VAssert(tf);

        vector<double> vals = GetContourValues(varName);
        double         val = vals[lineNum];

        tf->rgbValue(val, color);
    }
}

void ContourParams::SetLockToTF(bool lock)
{
    string l = "false";
    if (lock) { l = "true"; }
    SetValueString(_lockToTFTag, "Lock settings to TF", l);
}

bool ContourParams::GetLockToTF() const
{
    if (GetValueString(_lockToTFTag, "false") == "false") {
        return false;
    } else {
        return true;
    }
}

bool ContourParams::GetTextEnabled() const
{
    if (GetValueString(_textEnabledTag, "false") == "false") {
        return false;
    } else {
        return true;
    }
}

void ContourParams::SetTFLock(bool lock)
{
    string l = "false";
    if (lock) l = "true";
    SetValueString(_lockToTFTag,
                   "Lock contours to transfer function"
                   " bounds",
                   l);
}

bool ContourParams::GetTFLock()
{
    string l = GetValueString(_lockToTFTag, "false");
    if (l == "false") return false;
    return true;
}

ContourParams::Contours::Contours(ParamsBase::StateSave *ssave) : ParamsBase(ssave, Contours::GetClassType()) {}

ContourParams::Contours::Contours(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node) {}

ContourParams::Contours::~Contours() { MyBase::SetDiagMsg("Contours::~Contours() this=%p", this); }

};    // end namespace VAPoR
