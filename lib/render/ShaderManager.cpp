#include "vapor/glutil.h"
#include "vapor/ShaderManager.h"

using namespace VAPoR;

using std::map;
using std::pair;
using std::string;

ShaderProgram2 *ShaderManager::GetShader(const std::string name) { return GetResource(name); }

SmartShaderProgram ShaderManager::GetSmartShader(const std::string name) { return SmartShaderProgram(GetResource(name)); }

#ifdef _WINDOWS
    #define PATH_SEPARATOR "\\"
#else
    #define PATH_SEPARATOR "/"
#endif

bool ShaderManager::LoadResourceByKey(const std::string name)
{
    if (HasResource(name)) {
        assert(!"Shader already loaded");
        return false;
    }
    ShaderProgram2 *shader = new ShaderProgram2;
    // TODO GL add use of GetAppPath for windows separators
    shader->AddShaderFromFile(GL_VERTEX_SHADER, _resourceDirectory + PATH_SEPARATOR + name + ".vert");
    shader->AddShaderFromFile(GL_FRAGMENT_SHADER, _resourceDirectory + PATH_SEPARATOR + name + ".frag");
    shader->Link();
    if (!shader->WasLinkingSuccessful()) {
        assert(!"Shader linking failed");
        delete shader;
        return false;
    }
    AddResource(name, shader);
    return true;
}
