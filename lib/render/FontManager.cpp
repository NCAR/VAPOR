#include "vapor/glutil.h"
#include "vapor/FontManager.h"
#include <vapor/ResourcePath.h>

using namespace VAPoR;
using namespace Wasp;

using std::string;
using std::map;
using std::pair;

FontManager::FontManager(GLManager *glManager)
: _glManager(glManager), _library(nullptr)
{
    VAssert(glManager);
    VAssert(!FT_Init_FreeType(&_library));
}

FontManager::~FontManager()
{
    for (auto it = _map.begin(); it != _map.end(); ++it)
        delete it->second;
    _map.clear();
    if (_library)
        FT_Done_FreeType(_library);
}

Font *FontManager::GetFont(const std::string &name, unsigned int size)
{
    return GetResource(pair<string, unsigned int>(name, size));
}

int FontManager::LoadResourceByKey(const std::pair<std::string, unsigned int> &key)
{
    if (HasResource(key)) {
        VAssert(!"Font already loaded");
        return -1;
    }
    const string name = key.first;
    const unsigned int size = key.second;
    const string path = GetSharePath("fonts/" + name + ".ttf");
    Font *f = new Font(_glManager, path, size, _library);
    AddResource(key, f);
    return 1;
}
