#include <vapor/VolumeAlgorithm.h>
#include <vapor/VolumeRegular.h>
#include <vapor/VolumeResampled.h>
#include <vapor/VolumeTest.h>

using namespace VAPoR;
using std::vector;
using std::string;

const std::vector<std::string> VolumeAlgorithm::_algorithmNames = {
    "Regular",
    "Resampled"
};

const std::vector<std::string> &VolumeAlgorithm::GetAlgorithmNames()
{
    return _algorithmNames;
}

VolumeAlgorithm *VolumeAlgorithm::NewAlgorithm(const std::string &name)
{
    if (name == "Regular") return new VolumeRegular;
    if (name == "Resampled") return new VolumeResampled;
    if (name == "Test") return new VolumeTest;
    return nullptr;
}
