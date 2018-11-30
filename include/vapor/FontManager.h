#pragma once

#include "vapor/IResourceManager.h"
#include "vapor/Font.h"

namespace VAPoR {

struct GLManager;

class FontManager : public IResourceManager<std::pair<std::string, unsigned int>, Font> {
    GLManager *_glManager;
    FT_Library _library;

public:
    FontManager(GLManager *glManager);
    ~FontManager();

    Font *GetFont(const std::string &name, unsigned int size);
    int   LoadResourceByKey(const std::pair<std::string, unsigned int> &key);
};

}    // namespace VAPoR
