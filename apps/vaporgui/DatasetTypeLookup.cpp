#include "DatasetTypeLookup.h"
#include "vapor/ParamsBase.h"

namespace {
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

const std::vector<std::pair<std::string, std::string>>& GetDatasets() {
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
    return (it != datasets.end()) ? it->second : "No description for given data type " + type ;
}

std::string DatasetTypeShortName(const std::string& descriptiveName) {
    auto it = std::find_if(datasets.begin(), datasets.end(), [&descriptiveName](const auto& pair) {
        return pair.second==descriptiveName;
    });
    return (it != datasets.end()) ? it->first : "No shortName for given description " + descriptiveName;
}
