
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/ContourParams.h>

namespace {
vector <string> string_replace(vector <string> v, string olds, string news) {
    for (int i=0; i<v.size(); i++) {
        if (v[i] == olds) v[i] = news;
    }
    return(v);
}

string string_replace(string s, string olds, string news) {
    if (s == olds) s = news;
    return(s);
}

};

namespace VAPoR{

//
// Register class with object factory!!!
//
static RenParamsRegistrar<ContourParams> registrar(ContourParams::GetClassType());

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

ContourParams::ContourParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave
) : RenderParams(dataMgr, ssave, ContourParams::GetClassType()) {
	SetDiagMsg("ContourParams::ContourParams() this=%p", this);

	_init();
}

ContourParams::ContourParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
) : RenderParams(dataMgr, ssave, node) {
	SetDiagMsg("ContourParams::ContourParams() this=%p", this);

	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != ContourParams::GetClassType()) {
		node->SetTag(ContourParams::GetClassType());
		_init();
	}
}


ContourParams::~ContourParams() {
	SetDiagMsg("ContourParams::~ContourParams() this=%p", this);

}

bool ContourParams::IsOpaque() const {
	return true;
}

bool ContourParams::usingVariable(const std::string& varname) {
	if ((varname.compare(GetHeightVariableName()) == 0)){
		return(true);
	}

	return(varname.compare(GetVariableName()) == 0);
}

void ContourParams::SetVariableName(string varname) {
    varname = string_replace(varname, "0", "NULL");
    varname = string_replace(varname, "", "NULL");

    SetValueString(_variableNameTag,"Specify variable name", varname);
	_init();
}

//Set everything to default values
//void ContourParams::init(string varname) {
void ContourParams::_init() {
	SetDiagMsg("ContourParams::init()");

	// Only 2D variables supported. Override base class
	//
	vector <string> varnames = _dataMgr->GetDataVarNames(3, true);
	string varname = GetVariableName();
	if ((varname=="")) {
		if(!varnames.empty()) {
			varname = varnames[0];
			SetVariableName(varname);
		}
	}

	// Initialize 2D box
	//
	if (varname.empty()) {
		return;
	}

	vector <double> minExt, maxExt;
	int rc = _dataMgr->GetVariableExtents(0, varname, -1, minExt, maxExt);

	SetUseSingleColor(true);
	float rgb[] = {.95,.66,.27};
	SetConstantColor(rgb);

	// Crap. No error handling from constructor. Need Initialization()
	// method.
	//
	assert (rc >=0);
	assert(minExt.size()==maxExt.size() && minExt.size()>=2);

	GetBox()->SetExtents(minExt, maxExt);

	VAPoR::MapperFunction* mf = GetMapperFunc(varname);
	if (mf==NULL) {
		mf = MakeTransferFunc(varname);
	}
	double lower = mf->getMinMapValue();
	double upper = mf->getMaxMapValue();

	vector<double> cVals;
	//int spacing = GetContourSpacing();
	int numContours = GetNumContours();
	double spacing = (upper-lower) / (double)(numContours-1);
	//double min = GetContourMin();
	for (size_t i=0; i<numContours; i++) {
		cVals.push_back(lower + spacing*i);
	}
	SetContourMin(lower);
	SetContourSpacing(spacing);
	SetIsovalues(cVals);

	//GetBox()->SetPlanar(true);	
	//GetBox()->SetOrientation(2);	
}

}; // end namespace VAPoR
