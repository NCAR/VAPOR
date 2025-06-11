#pragma once

#include <vector>
#include <string>

namespace VAPoR {
    class ControlExec;
}

namespace DatasetImportUtils {
    enum class DatasetExistsAction { Prompt, ReplaceFirst, AddNew };
    bool ImportDataset(VAPoR::ControlExec *ce, const std::vector<std::string> &files, std::string format, DatasetExistsAction action);
}

