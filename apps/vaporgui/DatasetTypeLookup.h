#pragma once

//#include <unordered_map>
#include <map>
#include <string>
#include <vector>

//extern const std::unordered_map<std::string, std::string> datasets;
//extern const std::map<std::string, std::string> datasets;

const std::map<std::string, std::string>& GetDatasets();
std::vector<std::string> GetDatasetTypeDescriptions();
std::string DatasetTypeDescriptiveName(const std::string& type);
std::string DatasetTypeShortName(const std::string& descriptiveName);
