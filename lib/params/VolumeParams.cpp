
#include <string>
#include <vapor/VolumeParams.h>
#include <vapor/STLUtils.h>

using namespace Wasp;
using namespace VAPoR;

//
// Register class with object factory!!!
//
static RenParamsRegistrar<VolumeParams> registrar(VolumeParams::GetClassType());

const std::string VolumeParams::_algorithmTag = "AlgorithmTag";

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, VolumeParams::GetClassType(), 3)
{
    SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);

    _init();
}

VolumeParams::VolumeParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3)
{
    SetDiagMsg("VolumeParams::VolumeParams() this=%p", this);

    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != VolumeParams::GetClassType()) {
        node->SetTag(VolumeParams::GetClassType());
        _init();
    }
}

VolumeParams::~VolumeParams() { SetDiagMsg("VolumeParams::~VolumeParams() this=%p", this); }

bool VolumeParams::IsOpaque() const { return true; }

bool VolumeParams::usingVariable(const std::string &varname) { return (varname.compare(GetVariableName()) == 0); }

std::string VolumeParams::GetAlgorithm() const { return GetValueString(_algorithmTag, "Regular"); }

void VolumeParams::SetAlgorithm(std::string algorithm)
{
    assert(STLUtils::Contains(GetAlgorithmNames(), algorithm));
    SetValueString(_algorithmTag, "Volume rendering algorithm", algorithm);
}

const std::vector<std::string> VolumeParams::GetAlgorithmNames() { return {"Regular", "Resampled", "Cell Traversal", "Test"}; }

// Set everything to default values
void VolumeParams::_init() { SetDiagMsg("VolumeParams::_init()"); }
