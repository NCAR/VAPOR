
#include <string>
#include <vapor/VolumeParams.h>
#include <vapor/STLUtils.h>


using namespace Wasp;
using namespace VAPoR;


//
// Register class with object factory!!!
//
static RenParamsRegistrar<VolumeParams> registrar(VolumeParams::GetClassType());

std::vector<std::string> VolumeParams::_algorithmNames;

const std::string VolumeParams::_algorithmTag = "AlgorithmTag";
const std::string VolumeParams::_isoValueTag = "IsoValueTag";
const std::string VolumeParams::_isoValuesTag = "IsoValuesTag";
const std::string VolumeParams::_enabledIsoValuesTag = "EnabledIsoValuesTag";

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave)
: RenderParams(dataMgr, ssave, VolumeParams::GetClassType(), 3)
{
    SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);
    _init();
}

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType)
: RenderParams(dataMgr, ssave, classType, 3)
{
    SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);
    _init();
}

VolumeParams::VolumeParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
) : RenderParams(dataMgr, ssave, node, 3) {
	SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);

	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != VolumeParams::GetClassType()) {
		node->SetTag(VolumeParams::GetClassType());
		_init();
	}
}


VolumeParams::~VolumeParams() {
	SetDiagMsg("VolumeParams::~VolumeParams() this=%p", this);

}

bool VolumeParams::IsOpaque() const {
	return true;
}

bool VolumeParams::usingVariable(const std::string& varname) {
	return(varname.compare(GetVariableName()) == 0);
}

string VolumeParams::GetDefaultAlgorithmName() const
{
    return "Regular";
}

std::string VolumeParams::GetAlgorithm() const
{
    return GetValueString(_algorithmTag, GetDefaultAlgorithmName());
}

void VolumeParams::SetAlgorithm(std::string algorithm)
{
    assert(STLUtils::Contains(GetAlgorithmNames(), algorithm));
    SetValueString(_algorithmTag, "Volume rendering algorithm", algorithm);
}

double VolumeParams::GetIsoValue() const
{
    return GetValueDouble(_isoValueTag, 0);
}

void VolumeParams::SetIsoValue(double isoValue)
{
    SetValueDouble(_isoValueTag, "Iso surface value", isoValue);
}


void VolumeParams::SetIsoValues(std::vector<double> mask)
{
    if (mask.size() != 4)
        mask.resize(4, 0.0);
    SetValueDoubleVec(_isoValuesTag, "Iso surface values", mask);
}

std::vector<double> VolumeParams::GetIsoValues() const
{
    std::vector<double> defaultVec( 4, 0.0 );
    return GetValueDoubleVec( _isoValuesTag, defaultVec );
}

void VolumeParams::SetEnabledIsoValues(std::vector<bool> mask)
{
    if (mask.size() != 4)
        mask.resize(4, 0.0);
    vector<long> maskL;
    for (bool b : mask)
        maskL.push_back(b);
    
    SetValueLongVec(_enabledIsoValuesTag, "Iso surface values", maskL);
}

std::vector<bool> VolumeParams::GetEnabledIsoValues() const
{
    auto maskLong = GetValueLongVec(_enabledIsoValuesTag, {1,0,0,0});
    std::vector<bool> mask;
    for (long v : maskLong)
        mask.push_back(v);
    return mask;
}

const std::vector<std::string> VolumeParams::GetAlgorithmNames()
{
    return _algorithmNames;
}

void VolumeParams::Register(const std::string &name)
{
    assert(!STLUtils::Contains(_algorithmNames, name));
    _algorithmNames.push_back(name);
}

//Set everything to default values
void VolumeParams::_init() {
	SetDiagMsg("VolumeParams::_init()");
}

