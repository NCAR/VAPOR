#pragma once

#include <vector>
#include <string>

class ControlExec;

namespace DatasetImportUtils {

enum class DatasetExistsAction { Prompt, ReplaceFirst, AddNew };

bool ImportDataset(ControlExec *ce, const std::vector<std::string> &files, std::string format, DatasetExistsAction action, bool sessionNewFlag, std::function<void()> onImport = nullptr);

}

