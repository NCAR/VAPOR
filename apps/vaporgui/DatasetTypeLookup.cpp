#include "DatasetTypeLookup.h"
#include "vapor/ParamsBase.h"

namespace {
    //const std::map<std::string, std::string> datasets =
    //const std::unordered_map<std::string, std::string> datasets =
    const std::vector<std::pair<std::string, std::string>> datasets =
    {
        { "wrf"   , "WRF-ARW" },
        { "cf"    , "NetCDF-CF" },
        { "bov"   , "Brick of Values (BOV)" },
        { "dcp"   , "Data Collection Particles (DCP)" },
        { "mpas"  , "MPAS" },
        { "ugrid" , "Unstructured Grid (UGRID)" },
        { "vdc"   , "VDC" }
    };
}

//const std::map<std::string, std::string>& GetDatasets() {
//const std::unordered_map<std::string, std::string>& GetDatasets() {
const std::vector<std::pair<std::string, std::string>>& GetDatasets() {
    for (auto d : datasets) std::cout << "GetDatasets " << d.first << " " << d.second << std::endl;
    return datasets;
}

std::vector<std::string> GetDatasetTypeDescriptions() {
    std::vector<std::string> descriptions;
    for (const auto& pair : datasets) descriptions.push_back(pair.second);
    return descriptions;
}

std::string DatasetTypeDescriptiveName(const std::string& type) {
    auto it = std::find_if(datasets.begin(), datasets.end(), [&type](const auto& pair) {
        return pair.first==type;
    });
    return (it != datasets.end()) ? it->second : "No description for type";
    //if (datasets.find(type) != datasets.end()) return datasets[type];
    //else return type;
    //auto it = datasets.find(type);
    //return (it != datasets.end()) ? it->second : type;
    //return (it != datasets.end()) ? it->second : "No description for "+type;
}

std::string DatasetTypeShortName(const std::string& descriptiveName) {
    static std::unordered_map<std::string, std::string> descriptiveToShort;
    //static std::map<std::string, std::string> descriptiveToShort;
    for (const auto& pair : datasets) {
        descriptiveToShort[pair.second] = pair.first;
    }

    if (descriptiveToShort.find(descriptiveName) != descriptiveToShort.end()) return descriptiveToShort[descriptiveName];
    else return descriptiveName;
}
