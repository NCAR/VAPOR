#include "vapor/glutil.h"
#include "vapor/ShaderManager.h"

using namespace VAPoR;

using std::map;
using std::pair;
using std::string;

ShaderProgram2 *ShaderManager::GetShader(const std::string name)
{
    auto it = _shaderMap.find(name);
    if (it == _shaderMap.end()) {
        if (!LoadShaderByName(name)) {
            assert(!"Shader does not exist and unable to load by name");
            return nullptr;
        }
        it = _shaderMap.find(name);
    }
    return it->second;
}

SmartShaderProgram ShaderManager::GetSmartShader(const std::string name) { return SmartShaderProgram(GetShader(name)); }

bool ShaderManager::HasShader(const std::string name) { return _shaderMap.find(name) != _shaderMap.end(); }

bool ShaderManager::HasShader(const ShaderProgram2 *shader)
{
    for (auto it = _shaderMap.begin(); it != _shaderMap.end(); ++it)
        if (it->second == shader) return true;
    return false;
}

bool ShaderManager::SetShaderDirectory(const std::string path)
{
    _shaderPathPrefix = path;
    return true;
}

#ifdef _WINDOWS
    #define PATH_SEPARATOR "\"
#else
    #define PATH_SEPARATOR "/"
#endif

bool ShaderManager::LoadShaderByName(const std::string name)
{
    if (HasShader(name)) {
        assert(!"Shader already loaded");
        return false;
    }
    ShaderProgram2 *shader = new ShaderProgram2;
    // TODO GL add use of GetAppPath for windows separators
    shader->AddShaderFromFile(GL_VERTEX_SHADER, _shaderPathPrefix + PATH_SEPARATOR + name + ".vert");
    shader->AddShaderFromFile(GL_FRAGMENT_SHADER, _shaderPathPrefix + PATH_SEPARATOR + name + ".frag");
    shader->Link();
    if (!shader->WasLinkingSuccessful()) {
        assert(!"Shader linking failed");
        delete shader;
        return false;
    }
    AddShader(name, shader);
    return true;
}

bool ShaderManager::AddShader(const std::string name, VAPoR::ShaderProgram2 *shader)
{
    if (HasShader(name) || HasShader(shader)) {
        assert(!"Shader already exists");
        return false;
    }
    _shaderMap.insert(pair<string, ShaderProgram2 *>(name, shader));
    return true;
}
