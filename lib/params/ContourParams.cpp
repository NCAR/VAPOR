
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/ContourParams.h>

namespace VAPoR {

//
// Register class with object factory!!!
//
static RenParamsRegistrar<ContourParams> registrar(ContourParams::GetClassType());
//static ParamsRegistrar<Contours> registrar2(Contours::GetClassType());

const string ContourParams::_thicknessScaleTag = "LineThickness";
const string ContourParams::_varsAre3dTag = "VarsAre3D";
const string ContourParams::_numContoursTag = "NumIsovalues";
const string ContourParams::_contoursTag = "Contours";
const string ContourParams::_numDigitsTag = "NumDigits";
const string ContourParams::_textDensityTag = "TextDensity";
const string ContourParams::_lineColorTag = "LineColor";
const string ContourParams::_textEnabledTag = "TextEnabled";
const string ContourParams::_contourMinTag = "ContourMinimum";
const string ContourParams::_contourSpacingTag = "ContourSpacing";
const string ContourParams::_lockToTFTag = "LockToTF";
//const string Contours::_minTag = "Min";
//const string Contours::_countTag = "Count";
//const string Contours::_spacingTag = "Spacing";

ContourParams::ContourParams(
    DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, ContourParams::GetClassType(), 2) {
    SetDiagMsg("ContourParams::ContourParams() this=%p", this);

    _contours = new ParamsContainer(ssave, _contoursTag);
    _contours->SetParent(this);

    _init();
}

ContourParams::ContourParams(
    DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 2) {
    SetDiagMsg("ContourParams::ContourParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != ContourParams::GetClassType()) {
        node->SetTag(ContourParams::GetClassType());
    }

    if (node->HasChild(_contoursTag)) {
        _contours = new ParamsContainer(ssave, node->GetChild(_contoursTag));
    } else {
        // Node doesn't contain a contours container
        _contours = new ParamsContainer(ssave, _contoursTag);
        _contours->SetParent(this);
    }
}

ContourParams::ContourParams(const ContourParams &rhs)
    : RenderParams(rhs) {
    _contours = new ParamsContainer(*(rhs._contours));
}

ContourParams &ContourParams::operator=(const ContourParams &rhs) {
    if (_contours)
        delete _contours;

    ParamsBase::operator=(rhs);
    _contours = new ParamsContainer(*(rhs._contours));

    return (*this);
}

ContourParams::~ContourParams() {
    SetDiagMsg("ContourParams::~ContourParams() this=%p", this);

    if (_contours != NULL) {
        delete _contours;
        _contours = NULL;
    }
}

bool ContourParams::IsOpaque() const {
    return true;
}

bool ContourParams::usingVariable(const std::string &varname) {
    if ((varname.compare(GetHeightVariableName()) == 0)) {
        return (true);
    }

    return (varname.compare(GetVariableName()) == 0);
}

vector<double> ContourParams::GetIsovalues(string varName) {
    Contours *c = (Contours *)_contours->GetParams(varName);
    if (c == NULL) {
        MakeNewContours(varName);
        c = (Contours *)_contours->GetParams(varName);
    }
    return c->GetIsovalues();
}

void ContourParams::SetIsovalues(string varName, vector<double> vals) {
    Contours *c = (Contours *)_contours->GetParams(varName);
    if (c == NULL) {
        MakeNewContours(varName);
        c = (Contours *)_contours->GetParams(varName);
    }
    c->SetIsovalues(vals);
}

ContourParams::Contours *ContourParams::GetContours() {
    string varName = GetVariableName();
    ContourParams::Contours *c = (ContourParams::Contours *)_contours->GetParams(varName);
    if (c == NULL) {
        MakeNewContours(varName);
        c = (ContourParams::Contours *)_contours->GetParams(varName);
    }
    return c;
}

void ContourParams::MakeNewContours(string varName) {
    Contours newContours(_ssave);
    _contours->Insert(&newContours, varName);

    MapperFunction *mf = GetMapperFunc(varName);
    if (mf == NULL) {
        mf = MakeMapperFunc(varName);
    }
    vector<double> minMax = mf->getMinMaxMapValue();

    Contours *c = (Contours *)_contours->GetParams(varName);
    c->SetMin(minMax[0]);
    double spacing = (minMax[1] - minMax[0]) / ((double)c->GetCount() - 1);
    c->SetSpacing(spacing);
}

//Set everything to default values
void ContourParams::_init() {
    SetDiagMsg("ContourParams::_init()");

    // Only 2D variables supported. Override base class
    //
    vector<string> varnames = _dataMgr->GetDataVarNames(2, true);
    string varname;

    if (!varnames.empty())
        varname = varnames[0];
    SetVariableName(varname);

    // Initialize 2D box
    //
    if (varname.empty())
        return;

    vector<double> minExt, maxExt;
    int rc = _dataMgr->GetVariableExtents(0, varname, -1, minExt, maxExt);

    SetUseSingleColor(true);
    float rgb[] = {.95, .66, .27};
    SetConstantColor(rgb);

    // Crap. No error handling from constructor. Need Initialization()
    // method.
    //
    assert(rc >= 0);
    assert(minExt.size() == maxExt.size() && minExt.size() >= 2);

    GetBox()->SetExtents(minExt, maxExt);

    vector<double> cVals;
    int spacing = GetContourSpacing();
    int numContours = GetNumContours();
    double min = GetContourMin();
    for (size_t i = 0; i < numContours; i++) {
        cVals.push_back(min + spacing * i);
    }
    SetIsovalues(varname, cVals);

    //GetBox()->SetPlanar(true);
    //GetBox()->SetOrientation(2);
}

int ContourParams::GetNumContours() {
    Contours *c = GetContours();
    return c->GetCount();
}

void ContourParams::SetNumContours(int num) {
    Contours *c = GetContours();
    c->SetCount(num);
}

double ContourParams::GetContourMin() {
    Contours *c = GetContours();
    return c->GetMin();
}

void ContourParams::SetContourMin(double min) {
    Contours *c = GetContours();
    c->SetMin(min);
}

double ContourParams::GetContourSpacing() {
    Contours *c = GetContours();
    return c->GetSpacing();
}

void ContourParams::SetContourSpacing(double spacing) {
    Contours *c = GetContours();
    c->SetSpacing(spacing);
}

void ContourParams::GetLineColor(int lineNum, float color[3]) {
    GetConstantColor(color);
    string cmVar = GetColorMapVariableName();
    if ((cmVar == "") || (cmVar == "Default")) {
        string varName = GetVariableName();
        TransferFunction *tf = 0;
        tf = (TransferFunction *)GetMapperFunc(varName);
        if (!tf) {
            tf = MakeTransferFunc(varName);
        }
        assert(tf);

        vector<double> vals = GetIsovalues(varName);
        double val = vals[lineNum];

        tf->rgbValue(val, color);
    } else {
        GetConstantColor(color);
    }
}

void ContourParams::SetLockToTF(bool lock) {
    string l = "false";
    if (lock) {
        l = "true";
    }
    SetValueString(_lockToTFTag, "Lock settings to TF", l);
}

bool ContourParams::GetLockToTF() const {
    if (GetValueString(_lockToTFTag, "true") == "true") {
        return true;
    } else {
        return false;
    }
}

bool ContourParams::GetTextEnabled() const {
    if (GetValueString(_textEnabledTag, "false") == "false") {
        return false;
    } else {
        return true;
    }
}

void ContourParams::SetTFLock(bool lock) {
    string l = "false";
    if (lock)
        l = "true";
    SetValueString(_lockToTFTag, "Lock contours to transfer function"
                                 " bounds",
                   l);
}

bool ContourParams::GetTFLock() {
    string l = GetValueString(_lockToTFTag, "true");
    if (l == "false")
        return false;
    return true;
}

}; // end namespace VAPoR
