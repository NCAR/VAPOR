
#include <string>
#include <vapor/VolumeParams.h>


using namespace Wasp;
using namespace VAPoR;


//
// Register class with object factory!!!
//
static RenParamsRegistrar<VolumeParams> registrar(VolumeParams::GetClassType());


VolumeParams::VolumeParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave
) : RenderParams(dataMgr, ssave, VolumeParams::GetClassType(), 3) {
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


//Set everything to default values
void VolumeParams::_init() {
	SetDiagMsg("VolumeParams::_init()");

}

