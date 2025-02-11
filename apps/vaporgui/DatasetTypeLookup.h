#pragma once

#include <string>
#include <vector>

const std::vector<std::pair<std::string, std::string>>& GetDatasets();
std::vector<std::string> GetDatasetTypeDescriptions();
std::string DatasetTypeDescriptiveName(const std::string& type);
std::string DatasetTypeShortName(const std::string& descriptiveName);
