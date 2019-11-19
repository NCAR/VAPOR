
#include <string>
#include <vapor/RenderParams.h>
#include <vapor/TwoDDataParams.h>


using namespace Wasp;
using namespace VAPoR;


//
// Register class with object factory!!!
//
static RenParamsRegistrar<TwoDDataParams> registrar(TwoDDataParams::GetClassType());


TwoDDataParams::TwoDDataParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave
) : RenderParams(dataMgr, ssave, TwoDDataParams::GetClassType(), 2) {
	SetDiagMsg("TwoDDataParams::TwoDDataParams() this=%p", this);

	_init();
}

TwoDDataParams::TwoDDataParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
) : RenderParams(dataMgr, ssave, node, 2) {
	SetDiagMsg("TwoDDataParams::TwoDDataParams() this=%p", this);

	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != TwoDDataParams::GetClassType()) {
		node->SetTag(TwoDDataParams::GetClassType());
		_init();
	}
}


TwoDDataParams::~TwoDDataParams() {
	SetDiagMsg("TwoDDataParams::~TwoDDataParams() this=%p", this);

}

bool TwoDDataParams::IsOpaque() const {
	return true;
}

bool TwoDDataParams::usingVariable(const std::string& varname) {
	if ((varname.compare(GetHeightVariableName()) == 0)){
		return(true);
	}

	return(varname.compare(GetVariableName()) == 0);
}


//Set everything to default values
void TwoDDataParams::_init() {
	SetDiagMsg("TwoDDataParams::_init()");

	// Only 2D variables supported. Override base class
	//
    vector <string> varnames = _dataMgr->GetDataVarNames(2);
	string varname;

	if (! varnames.empty()) varname = varnames[0];
    SetVariableName(varname);
	SetColorMapVariableName(varname);

	SetFieldVariableNames(vector <string>());

	// Initialize 2D box
	//
	if (varname.empty()) return;

    if (! _dataMgr->VariableExists(0,varname, 0, 0)) return;

    vector <double> minExt, maxExt;
    int rc = _dataMgr->GetVariableExtents(0, varname, 0, 0, minExt, maxExt);

	// Crap. No error handling from constructor. Need Initialization()
	// method.
	//
	VAssert (rc >=0);
	VAssert(minExt.size()==maxExt.size() && minExt.size()==2);

	GetBox()->SetExtents(minExt, maxExt);
	GetBox()->SetPlanar(true);	
	GetBox()->SetOrientation(Box::XY);	
}

