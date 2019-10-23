#pragma once

#include <string>

namespace VAPoR {
class MapperFunction;
class ParamsMgr;
}    // namespace VAPoR

namespace TFUtils {
void LoadColormap(VAPoR::MapperFunction *tf, const std::string &path);
void LoadColormap(VAPoR::ParamsMgr *paramsMgr, VAPoR::MapperFunction *tf);
void LoadTransferFunction(VAPoR::ParamsMgr *paramsMgr, VAPoR::MapperFunction *tf);
void SaveTransferFunction(VAPoR::ParamsMgr *paramsMgr, VAPoR::MapperFunction *tf);
}    // namespace TFUtils
