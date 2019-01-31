#include <vapor/VolumeAlgorithm.h>
#include <vapor/VolumeRegular.h>
#include <vapor/VolumeResampled.h>
#include <vapor/VolumeTest.h>
#include <vapor/VolumeCellTraversal.h>

using namespace VAPoR;
using std::string;
using std::vector;

const std::vector<std::string> VolumeAlgorithm::_algorithmNames = {
    "Regular",
    "Resampled",
    "Cell Traversal"};

const std::vector<std::string> &VolumeAlgorithm::GetAlgorithmNames() {
    return _algorithmNames;
}

VolumeAlgorithm *VolumeAlgorithm::NewAlgorithm(const std::string &name) {
    if (name == "Regular")
        return new VolumeRegular;
    if (name == "Resampled")
        return new VolumeResampled;
    if (name == "Test")
        return new VolumeTest;
    if (name == "Cell Traversal")
        return new VolumeCellTraversal;
    return nullptr;
}
