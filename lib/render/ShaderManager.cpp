#include "vapor/glutil.h"
#include "vapor/ShaderManager.h"
#include "vapor/FileUtils.h"
#include <vapor/ResourcePath.h>

using namespace VAPoR;
using namespace Wasp;

using std::string;
using std::vector;
using std::map;
using std::pair;


#define SHADER_AUTORELOAD 1


#ifdef NDEBUG
    #if SHADER_AUTORELOAD
        #ifndef WIN32
            #warning Disabling shader autoreloading
        #endif
        #undef SHADER_AUTORELOAD
    #endif
#endif

std::vector<std::string> ShaderManager::_getSourceFilePaths(const std::string &name) const
{
    vector<string> paths;
    paths.push_back(GetSharePath("shaders/" + name + ".vert"));
    paths.push_back(GetSharePath("shaders/" + name + ".frag"));
    return paths;
}

bool ShaderManager::_wasFileModified(const std::string &path) const
{
    return false;
}

ShaderProgram *ShaderManager::GetShader(const std::string &name)
{
#if SHADER_AUTORELOAD
    if (HasResource(name)) {
        const vector<string> paths = _getSourceFilePaths(name);
        for (auto it = paths.begin(); it != paths.end(); ++it) {
            long mtime = FileUtils::GetFileModifiedTime(*it);
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

SmartShaderProgram ShaderManager::GetSmartShader(const std::string &name)
{
    return SmartShaderProgram(GetShader(name));
}

int ShaderManager::LoadResourceByKey(const std::string &name)
{
    if (HasResource(name)) {
        assert(!"Shader already loaded");
        return -1;
    }
    ShaderProgram *program = new ShaderProgram;
    const vector<string> paths = _getSourceFilePaths(name);
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        program->AddShader(CompileNewShaderFromFile(*it));
        _modifiedTimes[*it] = FileUtils::GetFileModifiedTime(*it);
    }
    program->Link();
    if (!program->WasLinkingSuccessful()) {
        SetErrMsg("Failed to link shader:\n%s", program->GetLog().c_str());
        delete program;
        return -1;
    }
    AddResource(name, program);
    return 1;
}

Shader *ShaderManager::CompileNewShaderFromFile(const std::string &path)
{
    unsigned int shaderType = GetShaderTypeFromPath(path);
    if (shaderType == GL_INVALID_ENUM) {
        SetErrMsg("File \"%s\" does not have a valid shader file extension", FileUtils::Basename(path).c_str());
        return nullptr;
    }
    if (!FileUtils::IsRegularFile(path)) {
        SetErrMsg("Path \"%s\" is not a valid file", path.c_str());
        return nullptr;
    }
    Shader *shader = new Shader(shaderType);
    int compilationSuccess = shader->CompileFromSource(FileUtils::ReadFileToString(path));
    if (compilationSuccess < 0) {
        SetErrMsg("Shader \"%s\" failed to compile", FileUtils::Basename(path).c_str());
        delete shader;
        return nullptr;
    }
    return shader;
}

unsigned int ShaderManager::GetShaderTypeFromPath(const std::string &path)
{
    string ext = FileUtils::Extension(path);
    if (ext == "vert") return GL_VERTEX_SHADER;
    if (ext == "frag") return GL_FRAGMENT_SHADER;
    if (ext == "geom") return GL_GEOMETRY_SHADER;
    return GL_INVALID_ENUM;
}
