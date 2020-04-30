#include "vapor/glutil.h"
#include "vapor/ShaderManager.h"
#include <vapor/Shader.h>
#include "vapor/FileUtils.h"
#include <vapor/ResourcePath.h>
#include <vapor/STLUtils.h>

using namespace VAPoR;
using namespace Wasp;
using namespace STLUtils;

using std::map;
using std::pair;
using std::string;
using std::vector;

#define SHADER_AUTORELOAD 1

#ifdef NDEBUG
    #if SHADER_AUTORELOAD
        #ifndef WIN32
        //#warning Disabling shader autoreloading
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

bool ShaderManager::_wasFileModified(const std::string &path) const { return false; }

std::string ShaderManager::_getNameFromKey(const std::string &key) { return Split(key, ":")[0]; }

std::vector<std::string> ShaderManager::_getDefinesFromKey(const std::string &key)
{
    vector<string> defines = Split(key, ":");
    defines.erase(defines.begin());
    return defines;
}

ShaderProgram *ShaderManager::GetShader(const std::string &key)
{
#if SHADER_AUTORELOAD
    if (!_dependencyModifiedTimes.count(key)) {
        vector<string>    paths = _getSourceFilePaths(_getNameFromKey(key));
        vector<string>    deps;
        map<string, long> times;

        for (const string &path : paths) STLUtils::AppendTo(deps, GetShaderDependencies(path));

        for (const string &path : deps) times[path] = FileUtils::GetFileModifiedTime(path);

        _dependencyModifiedTimes[key] = times;
    }
    bool reload = false;
    for (auto &pair : _dependencyModifiedTimes[key]) {
        long mtime = FileUtils::GetFileModifiedTime(pair.first);
        if (mtime > pair.second) {
            //            printf("Reload \"%s\"\n", pair.first.c_str());
            pair.second = mtime;
            reload = true;
        }
    }
    if (HasResource(key)) {
        bool rebind = false;
        int  boundProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &boundProgram);
        if (reload && GetResource(key)->GetID() == boundProgram) rebind = true;

        if (reload) DeleteResource(key);

        ShaderProgram *shader = GetResource(key);
        if (rebind) shader->Bind();
        return shader;
    }
#endif
    return GetResource(key);
}

SmartShaderProgram ShaderManager::GetSmartShader(const std::string &name) { return SmartShaderProgram(GetShader(name)); }

#include <vapor/GLManager.h>

int ShaderManager::LoadResourceByKey(const std::string &key)
{
    //    printf("Begin Compile %s\n", key.c_str());
    //    void *t = GLManager::BeginTimer();

    if (HasResource(key)) {
        VAssert(!"Shader already loaded");
        return -1;
    }

    const vector<string> defines = _getDefinesFromKey(key);

    ShaderProgram *      program = new ShaderProgram;
    const vector<string> paths = _getSourceFilePaths(_getNameFromKey(key));
    for (auto it = paths.begin(); it != paths.end(); ++it) { program->AddShader(CompileNewShaderFromFile(*it, defines)); }
    program->Link();
    if (!program->WasLinkingSuccessful()) {
        SetErrMsg("Failed to link shader:\n%s", program->GetLog().c_str());
        delete program;
        return -1;
    }
    AddResource(key, program);
    //    printf("End Compile %f\n", GLManager::EndTimer(t));
    return 1;
}

Shader *ShaderManager::CompileNewShaderFromFile(const std::string &path, const std::vector<std::string> &defines)
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
    int     compilationSuccess = shader->CompileFromSource(PreProcessShader(path, defines));
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

std::string ShaderManager::PreProcessShader(const std::string &path, const std::vector<std::string> &defines)
{
    string source = FileUtils::ReadFileToString(path);
    auto   lines = Split(source, "\n");

    int lineNum = 1;
    for (string &line : lines) {
        if (BeginsWith(line, "#")) {
            if (BeginsWith(line, "#pragma auto_version")) {
                line = "#version ";
                line += std::to_string(GLManager::GetGLSLVersion());
                line += " core";
            }

            if (!defines.empty() && BeginsWith(line, "#version ")) {
                line += "\n";
                for (const string &define : defines) { line += "#define " + define + "\n"; }
                line += "#line " + std::to_string(lineNum + 1) + " 0";
            }

            if (BeginsWith(line, "#include ")) {
                VAssert(Split(line, " ").size() == 2);
                line = "#line 1 1\n" + PreProcessShader(GetSharePath("shaders/" + Split(line, " ")[1]));
                // Sometimes, the reported line number for a syntax error will be incorrect.
                // This is a bug in the glsl compiler.
                line += "\n#line " + std::to_string(lineNum + 1) + " 0";
            }
        }
        lineNum++;
    }

    return Join(lines, "\n");
}

std::vector<std::string> ShaderManager::GetShaderDependencies(const std::string &path)
{
    vector<string> deps = {path};
    string         source = FileUtils::ReadFileToString(path);
    auto           lines = Split(source, "\n");
    for (const string &line : lines) {
        if (BeginsWith(line, "#include ")) {
            auto args = Split(line, " ");
            VAssert(args.size() == 2);
            string include = args[1];
            STLUtils::AppendTo(deps, GetShaderDependencies(GetSharePath("shaders/" + include)));
        }
    }
    return deps;
}
