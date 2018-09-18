#include "vapor/glutil.h"
#include "vapor/ShaderManager.h"
#include <sys/stat.h>

using namespace VAPoR;

using std::map;
using std::pair;
using std::string;
using std::vector;

#define SHADER_AUTORELOAD 1

#ifdef NDEBUG
    #if SHADER_AUTORELOAD
        #warning Disabling shader autoreloading
        #undef SHADER_AUTORELOAD
    #endif
#endif

#ifdef _WINDOWS
    #define PATH_SEPARATOR "\\"
#else
    #define PATH_SEPARATOR "/"
#endif

std::vector<std::string> ShaderManager::_getSourceFilePaths(const std::string &name) const
{
    // TODO GL add use of GetAppPath for windows separators
    vector<string> paths;
    paths.push_back(_resourceDirectory + PATH_SEPARATOR + name + ".vert");
    paths.push_back(_resourceDirectory + PATH_SEPARATOR + name + ".frag");
    return paths;
}

unsigned int ShaderManager::_getShaderTypeFromPath(const std::string &path) const
{
    string ext = path.substr(path.length() - 4, 4);
    if (ext == "vert") return GL_VERTEX_SHADER;
    if (ext == "frag") return GL_FRAGMENT_SHADER;
    if (ext == "geom") return GL_GEOMETRY_SHADER;
    return GL_INVALID_ENUM;
}

long ShaderManager::_getFileModifiedTime(const std::string &path)
{
    struct stat attrib;
    stat(path.c_str(), &attrib);
    return attrib.st_mtime;
}

bool ShaderManager::_wasFileModified(const std::string &path) const { return false; }

ShaderProgram *ShaderManager::GetShader(const std::string &name)
{
#if SHADER_AUTORELOAD
    if (HasResource(name)) {
        const vector<string> paths = _getSourceFilePaths(name);
        for (auto it = paths.begin(); it != paths.end(); ++it) {
            long mtime = _getFileModifiedTime(*it);
            if (mtime > _modifiedTimes[*it]) {
                _modifiedTimes[*it] = mtime;
                DeleteResource(name);
                break;
            }
        }
    }
#endif
    return GetResource(name);
}

SmartShaderProgram ShaderManager::GetSmartShader(const std::string &name) { return SmartShaderProgram(GetShader(name)); }

bool ShaderManager::LoadResourceByKey(const std::string &name)
{
    if (HasResource(name)) {
        assert(!"Shader already loaded");
        return false;
    }
    ShaderProgram *      shader = new ShaderProgram;
    const vector<string> paths = _getSourceFilePaths(name);
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        shader->AddShaderFromFile(_getShaderTypeFromPath(*it), *it);
        _modifiedTimes[*it] = _getFileModifiedTime(*it);
    }
    shader->Link();
    if (!shader->WasLinkingSuccessful()) {
        assert(!"Shader linking failed");
        delete shader;
        return false;
    }
    AddResource(name, shader);
    return true;
}
