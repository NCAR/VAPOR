#include <vapor/ParticleParams.h>
#include <vapor/STLUtils.h>
#include <string>
#include <cassert>

using namespace Wasp;
using namespace VAPoR;


static RenParamsRegistrar<ParticleParams> registrar(ParticleParams::GetClassType());

const std::string ParticleParams::ShowDirectionTag = "ShowDirectionTag";
const std::string ParticleParams::DirectionScaleTag = "DirectionScaleTag";
const std::string ParticleParams::StrideTag = "StrideTag";

ParticleParams::ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, ParticleParams::GetClassType(), 3)
{
    SetDiagMsg("ParticleParams::ParticleParams() this=%p", this);
    _init();
}

ParticleParams::ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, std::string classType) : RenderParams(dataMgr, ssave, classType, 3)
{
    SetDiagMsg("ParticleParams::ParticleParams() this=%p", this);
    _init();
}

ParticleParams::ParticleParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node, 3) {}


ParticleParams::~ParticleParams() { SetDiagMsg("ParticleParams::~ParticleParams() this=%p", this); }

// Set everything to default values
void ParticleParams::_init()
{
    SetDiagMsg("ParticleParams::_init()");

    SetValueLong(ShowDirectionTag, "", false);
    SetValueDouble(DirectionScaleTag, "", 1);
    SetValueLong(StrideTag, "", 1);
}
