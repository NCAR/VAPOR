#include <vapor/VolumeAlgorithm.h>
#include <vapor/VolumeRegular.h>
#include <vapor/VolumeResampled.h>
#include <vapor/VolumeTest.h>
#include <vapor/VolumeCellTraversal.h>

using namespace VAPoR;
using std::string;
using std::vector;

VolumeAlgorithm::VolumeAlgorithm(GLManager *gl)
    : _glManager(gl) {}

const std::vector<std::string> VolumeAlgorithm::_algorithmNames = {
    "Regular",
    "Resampled",
    "Cell Traversal"};

const std::vector<std::string> &VolumeAlgorithm::GetAlgorithmNames() {
    return _algorithmNames;
}

VolumeAlgorithm *VolumeAlgorithm::NewAlgorithm(const std::string &name, GLManager *gl) {
    if (name == "Regular")
        return new VolumeRegular(gl);
    if (name == "Resampled")
        return new VolumeResampled(gl);
    if (name == "Test")
        return new VolumeTest(gl);
    if (name == "Cell Traversal")
        return new VolumeCellTraversal(gl);
    return nullptr;
}
