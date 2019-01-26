#include <vapor/VolumeAlgorithm.h>
#include <vapor/VolumeRegular.h>
#include <vapor/VolumeResampled.h>

using namespace VAPoR;
using std::string;
using std::vector;

std::vector<std::string> VolumeAlgorithm::GetAlgorithmNames() { return {"Regular", "Resampled"}; }

VolumeAlgorithm *VolumeAlgorithm::NewAlgorithm(const std::string &name)
{
    if (name == "Regular") return new VolumeRegular;
    if (name == "Resampled") return new VolumeResampled;
    return nullptr;
}
