/*
 *  ShaderMgr.cpp
 *  
 *
 *  Created by Yannick Polius on 6/6/11.
 *  Copyright 2011 Yannick Polius All rights reserved.
 *
 */
#include <vapor/glutil.h> // Must be included first!!!
#include "assert.h"
#include <sstream>
#ifdef _WINDOWS
#include "vapor/direntWin32.h"
#else
#include <dirent.h>
#endif

#include <vapor/CFuncs.h>
#include <vapor/ShaderProgram.h>
#include <vapor/ShaderMgr.h>

using namespace VAPoR;
using namespace Wasp;

ShaderMgr::ShaderMgr() {
    _loaded = false;
    _glsl_version = 120; // default
    _sourceDir.clear();
}

ShaderMgr::ShaderMgr(string directory) {
    SetDiagMsg("ShaderMgr::ShaderMgr(%s)", directory.c_str());

    _loaded = false;
    _glsl_version = 120;
    _sourceDir = directory;
}

ShaderMgr::~ShaderMgr() {
    SetDiagMsg("ShaderMgr::~ShaderMgr()");

    for (std::map<std::string, ShaderProgram *>::const_iterator iter = _effects.begin();
         iter != _effects.end(); ++iter)
        delete iter->second;
}

void ShaderMgr::SetShaderSourceDir(string directory) {
    _sourceDir = directory;
}

//----------------------------------------------------------------------------
// Look through the source directory for the effect files and proceed to load them
//
// Effect files point to the main vertex and fragment shader files for a single effect
// The file hierarchy is :
// -> Effect file source dir
//    *.efc - files to describe effects (bundled fragment + vertex shader)
//	  -> main - contains the main shader programs (must contain main function)
//		 *.fgl - main fragment shader file
//		 *.vgl - main vertex shader file
//    -> includes - contains the shared code fragments (functions, not full shader programs)
//		 *.hgl - shader fragment code include file
//----------------------------------------------------------------------------

int ShaderMgr::LoadShaders() {
    SetDiagMsg("ShaderMgr::loadShaders()");

    if (_loaded)
        return (0);

    vector<string> efcFiles;

    int rc = _getEfcFiles(_sourceDir, efcFiles);
    if (rc < 0) {
        SetErrMsg("Failed to load efc files from %s", _sourceDir.c_str());
        return (-1);
    }

    if (efcFiles.size() == 0) {
        SetErrMsg("No .efc files found in dir: %s", _sourceDir.c_str());
        return (-1);
    }
    for (int i = 0; i < efcFiles.size(); i++) {
        if (_loadEffectFile(efcFiles[i]) < 0) {
            SetErrMsg("EFC file \"%s\" failed to load\n", efcFiles[i].c_str());
            return -1;
        }
    }

    _loaded = true;
    return 0;
}

//----------------------------------------------------------------------------
// Reload shaders from the shaders directory
//----------------------------------------------------------------------------

int ShaderMgr::ReloadShaders() {
    SetDiagMsg("ShaderMgr::reloadShaders()");

    //Check to see if any shaders are currently running
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    if (current != 0) {
        SetErrMsg("shader is in use, cannot delete anything!");
        return -1;
    }

    //Delete old shaders
    for (std::map<std::string, ShaderProgram *>::const_iterator iter = _effects.begin();
         iter != _effects.end(); iter++) {
        delete iter->second;
    }
    //Clear the map
    _effects.clear();
    _baseEffects.clear();

    //Reload shaders
    _loaded = false;
    return LoadShaders();
}
//----------------------------------------------------------------------------
// Enable an effect for rendering
//----------------------------------------------------------------------------
int ShaderMgr::EnableEffect(std::string effect) {
    SetDiagMsg("ShaderMgr::enableEffect(%s)", effect.c_str());
    if (EffectExists(effect)) {
        return (_effects[effect]->Enable());
    } else {
        SetErrMsg("Effect %s does not exist", effect.c_str());
        return -1;
    }
    return 0;
}
//----------------------------------------------------------------------------
// Disable the currently loaded effect
//----------------------------------------------------------------------------
void ShaderMgr::DisableEffect() {
    SetDiagMsg("ShaderMgr::disableEffect()");
    if (GLEW_VERSION_2_0) {
        glUseProgram(0);
    } else {
        glUseProgramObjectARB(0);
    }
    printOpenGLError();
}

void ShaderMgr::SetGLSLVersion(int version) {
    _glsl_version = version;
}
//----------------------------------------------------------------------------
// Convert the define string into OGL define directives
//----------------------------------------------------------------------------
std::string ShaderMgr::_convertDefines(std::string defines) {
    int colonIndex = defines.find(";");
    int pos = 0;
    stringstream ss;
    ss << _glsl_version;
    std::string result = "#version " + ss.str() + "\n";
    if (colonIndex < 0)
        return result;
    //last character must be a semi colon
    if (defines[defines.length() - 1] != ';') {
        return result;
    }
    while (pos < defines.length() - 1) {
        //substring the first section up to semi colon
        std::string token = defines.substr(pos, colonIndex - pos);
        std::string definition = "#define " + token + "\n";
        result += definition;
        definition = "";
        pos = colonIndex + 1;
        colonIndex = defines.find(";", pos);
    }
    return result;
}
//----------------------------------------------------------------------------
// Instantiates a shader program based on the defines
// example define: "PI 3.14;TAU 2.1872;MU"
//----------------------------------------------------------------------------
int ShaderMgr::DefineEffect(
    std::string baseName, std::string defines, std::string instanceName) {
    SetDiagMsg(
        "ShaderMgr::defineEffect(%s,%s,%s)",
        baseName.c_str(), defines.c_str(), instanceName.c_str());

    if (_baseEffects.count(baseName) == 0) {
        SetErrMsg("Base effect \"%s\" is not loaded", baseName.c_str());
        return (-1);
    }

    //check if another effect is already using the instanceName
    if (_effects.count(instanceName) > 0) {
        SetErrMsg("Effect \"%s\" already in use", instanceName.c_str());
        return (-1);
    }

    _effects[instanceName] = new ShaderProgram();
    _effects[instanceName]->Create();

    //Convert defines
    std::string OGLdefines = _convertDefines(defines);
    //iterate through vertex shader code
    for (int i = 0; i < _baseEffects[baseName][0].size(); i++) {
        //add definition tokens to everything
        std::string defined = OGLdefines + _baseEffects[baseName][0][i];
        _effects[instanceName]->LoadVertexSource(
            defined, _baseEffects[baseName][2][i]);
    }

    //iterate through fragment shader code
    for (int i = 0; i < _baseEffects[baseName][1].size(); i++) {

        //add definition tokens to all source code
        //
        std::string defined = OGLdefines + _baseEffects[baseName][1][i];
        _effects[instanceName]->LoadFragmentSource(
            defined, _baseEffects[baseName][3][i]);
    }

    int rc = _effects[instanceName]->Compile();
    if (rc < 0) {
        SetErrMsg(
            "Effect \"%s::%s\" failed to compile",
            baseName.c_str(), instanceName.c_str());
        return -1;
    }
    return (0);
}

bool ShaderMgr::UndefEffect(std::string instanceName) {
    SetDiagMsg("UndefEffect(%s)", instanceName.c_str());

    if (_effects.count(instanceName) == 0) {
        //effect has not been defined
        return false;
    } else {
        delete _effects[instanceName];
        _effects.erase(instanceName);
    }
    return true;
}

//----------------------------------------------------------------------------
// Handles parsing an effect file and constructing the shaderprogram
//----------------------------------------------------------------------------
int ShaderMgr::_loadEffectFile(std::string effect) {
    SetDiagMsg("ShaderMgr::loadEffectFile(%s)", effect.c_str());

    // this is an effect file, open to read info
    //
    string path = Wasp::Catpath("", _sourceDir, effect);

    ifstream file(path);
    if (!file) {
        SetErrMsg("EFC file \"%s\" failed to load\n", path.c_str());
        return (-1);
    }

    std::string line1, line2, line3;

    getline(file, line1);
    getline(file, line2);
    getline(file, line3);
    if (file.bad()) {
        SetErrMsg("EFC file \"%s\" failed to load\n", path.c_str());
        return (-1);
    }

    file.close();

    //--------------------------------------
    // Prepare to parse main shaders
    // Unlike normal GLSL this will load shaders
    // with basic preprocessor support
    //
    // Currently supported macros:
    //
    // #include
    //	included files must reside in the sourcedir/includes directory
    // #defines are supported programmatically, with the defineEffect function
    //--------------------------------------
    if (!line2.empty()) {
        string path = Wasp::Catpath("", _sourceDir, "main");
        path = Wasp::Catpath("", path, line2 + ".vgl");

        int rc = _loadVertShader(path, line2 + ".vgl", line1);
        if (rc < 0)
            return (-1);
    }

    if (!line3.empty()) {
        string path = Wasp::Catpath("", _sourceDir, "main");
        path = Wasp::Catpath("", path, line3 + ".fgl");

        int rc = _loadFragShader(path, line3 + ".fgl", line1);
        if (rc < 0)
            return (-1);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Loads a vertex shader, both main and included
//----------------------------------------------------------------------------
int ShaderMgr::_loadVertShader(
    std::string path, std::string fileName, std::string effectName) {
    SetDiagMsg(
        "ShaderMgr::loadVertShader(%s,,%s)",
        path.c_str(), fileName.c_str());

    //Vertex Shader loading
    ifstream file(path);
    if (!file) {
        SetErrMsg("vertex shader file \"%s\" failed to load\n", path.c_str());
        return (-1);
    }

    std::string code;
    std::string line;
    while (!file.eof()) {
        getline(file, line);
        line += '\n';
        code += line;
    }
    if (file.bad()) {
        SetErrMsg("vertex shader file \"%s\" failed to load\n", path.c_str());
        return (-1);
    }
    file.close();

    std::string processed = "";
    std::vector<std::string> includes = _preprocess(code, processed);

    for (int i = 0; i < includes.size(); i++) {
        string path = Wasp::Catpath("", _sourceDir, "includes");
        path = Wasp::Catpath("", path, includes[i]);

        int rc = _loadVertShader(path, includes[i], effectName);
        if (rc < 0)
            return (-1);
    }

    //Save source code
    _baseEffects[effectName][0].push_back(processed);

    //Save the vert shader name for later use
    _baseEffects[effectName][2].push_back(fileName);

    return 0;
}

//----------------------------------------------------------------------------
// Loads a fragment shader, both main and included
//----------------------------------------------------------------------------
int ShaderMgr::_loadFragShader(
    std::string path, std::string fileName, std::string effectName) {
    SetDiagMsg(
        "ShaderMgr::loadFragShader(%s,,%s)",
        path.c_str(), fileName.c_str());

    //Fragment Shader loading
    ifstream file(path);
    if (!file) {
        SetErrMsg("vertex shader file \"%s\" failed to load\n", path.c_str());
        return (-1);
    }
    std::string code;
    std::string line;
    while (!file.eof()) {
        getline(file, line);
        line += '\n';
        code += line;
    }
    if (file.bad()) {
        SetErrMsg("vertex shader file \"%s\" failed to load\n", path.c_str());
        return (-1);
    }
    file.close();

    std::string processed = "";
    std::vector<std::string> includes = _preprocess(code, processed);
    for (int i = 0; i < includes.size(); i++) {

        string path = Wasp::Catpath("", _sourceDir, "includes");
        path = Wasp::Catpath("", path, includes[i]);
        int rc = _loadFragShader(path, includes[i], effectName);
        if (rc < 0)
            return (-1);
    }

    //Save source code
    _baseEffects[effectName][1].push_back(processed);
    //Save the frag shader name for later use
    _baseEffects[effectName][3].push_back(fileName);

    return 0;
}

//----------------------------------------------------------------------------
// Process the source string to remove preprocessor macros
// A clean version of the source code is returned in the processed handle
//----------------------------------------------------------------------------

std::vector<std::string> ShaderMgr::_preprocess(
    const string &source, std::string &processed) {
    std::string tmpProcess("");
    std::string line;
    std::vector<std::string> includes;
    for (int i = 0; i < source.length(); i++) {
        if (source[i] == '\n') {
            //check line for preprocessor macros
            int index = line.find("#include");
            if (index > -1) {
                //add the included file to the list of includes
                string include(line.substr(index + 8));
                StrRmWhiteSpace(include);

                includes.push_back(include);
                line = "";

            } else {
                line += '\n';
                //normal line, just append to processed
                tmpProcess += line;
                line = "";
            }
        } else {
            line += source[i];
        }
    }
    processed = tmpProcess;
    return includes;
}

//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable, int value) {
    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform1i(_effects[effect]->UniformLocation(variable), value);
        } else {
            glUniform1iARB(_effects[effect]->UniformLocation(variable), value);
        }
        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return -1;
        if (GLEW_VERSION_2_0) {
            glUniform1i(_effects[effect]->UniformLocation(variable), value);
        } else {
            glUniform1iARB(_effects[effect]->UniformLocation(variable), value);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}
//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable, int value1, int value2) {

    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    int rc = printOpenGLError();
    if (rc < 0)
        return (-1);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform2i(_effects[effect]->UniformLocation(variable), value1, value2);
        } else {
            glUniform2iARB(_effects[effect]->UniformLocation(variable), value1, value2);
        }
    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return (-1);

        if (GLEW_VERSION_2_0) {
            glUniform2i(_effects[effect]->UniformLocation(variable), value1, value2);
        } else {
            glUniform2iARB(_effects[effect]->UniformLocation(variable), value1, value2);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}
//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable,
    int value1, int value2, int value3) {

    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform3i(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        } else {
            glUniform3iARB(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        }
        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return (-1);

        if (GLEW_VERSION_2_0) {
            glUniform3i(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        } else {
            glUniform3iARB(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable,
    int value1, int value2, int value3, int value4) {

    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform4i(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        } else {
            glUniform4iARB(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        }

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return (-1);

        if (GLEW_VERSION_2_0) {
            glUniform4i(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        } else {
            glUniform4iARB(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable, float value) {

    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform1f(_effects[effect]->UniformLocation(variable), value);
        } else {
            glUniform1fARB(_effects[effect]->UniformLocation(variable), value);
        }
        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return -1;
        if (GLEW_VERSION_2_0) {
            glUniform1f(_effects[effect]->UniformLocation(variable), value);
        } else {
            glUniform1fARB(_effects[effect]->UniformLocation(variable), value);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable, float value1, float value2) {

    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform2f(_effects[effect]->UniformLocation(variable), value1, value2);
        } else {
            glUniform2fARB(_effects[effect]->UniformLocation(variable), value1, value2);
        }

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return -1;
        if (GLEW_VERSION_2_0) {
            glUniform2f(_effects[effect]->UniformLocation(variable), value1, value2);
        } else {
            glUniform2fARB(_effects[effect]->UniformLocation(variable), value1, value2);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable,
    float value1, float value2, float value3) {

    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform3f(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        } else {
            glUniform3fARB(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        }

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);

    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return -1;

        if (GLEW_VERSION_2_0) {
            glUniform3f(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        } else {
            glUniform3fARB(_effects[effect]->UniformLocation(variable), value1, value2, value3);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return 0;
}

//----------------------------------------------------------------------------
// Uploads data to the selected effect's uniform
//----------------------------------------------------------------------------
int ShaderMgr::UploadEffectData(
    std::string effect, std::string variable,
    float value1, float value2, float value3, float value4) {
    //Check to see if program is currently loaded
    GLint current;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);

    if (!EffectExists(effect)) {
        SetErrMsg("Effect \"%s\" has not been defined", effect.c_str());
        return (-1);
    }

    if (current == _effects[effect]->GetProgram()) {
        //dont enable, just upload data
        if (GLEW_VERSION_2_0) {
            glUniform4f(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        } else {
            glUniform4fARB(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        }

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);

    } else {
        //enable the program to upload to temporarily, then re-enable the previous program
        if (_effects[effect]->Enable() < 0)
            return -1;

        if (GLEW_VERSION_2_0) {
            glUniform4f(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        } else {
            glUniform4fARB(_effects[effect]->UniformLocation(variable), value1, value2, value3, value4);
        }
        glUseProgram(current);

        int rc = printOpenGLError();
        if (rc < 0)
            return (-1);
    }
    return -1;
}

//----------------------------------------------------------------------------
// Prints out the effect name, followed by shader info
//----------------------------------------------------------------------------
void ShaderMgr::PrintEffects() {
    std::cout << "Loaded Effects: " << std::endl;
    for (std::map<std::string, ShaderProgram *>::const_iterator iter = _effects.begin();
         iter != _effects.end(); ++iter) {

        std::cout << '\t' << iter->first << '\t' << iter->second << '\t' << iter->second->GetProgram() << '\n';
        iter->second->PrintContents();
    }
}

//-------------------------------------------------------------------------
// Returns an effect name, given the shader program it is supposed to control
//-------------------------------------------------------------------------
std::string ShaderMgr::_findEffect(GLuint prog) {
    for (std::map<std::string, ShaderProgram *>::const_iterator iter = _effects.begin();
         iter != _effects.end(); ++iter) {
        if (iter->second->GetProgram() == prog)
            return iter->first;
    }
    return std::string("");
}

//-------------------------------------------------------------------------
// Check to see if an effect exists, must be done due to std::map autocreating
// an object that does not exist when checked with []
//-------------------------------------------------------------------------
bool ShaderMgr::EffectExists(std::string effect) {
    if (_effects.count(effect) > 0) {
        return true;
    } else {
        return false;
    }
}

std::string ShaderMgr::GLVendor() {
    const GLubyte *vendor = glGetString(GL_VENDOR);
    std::string vendorString = vendor ? (const char *)vendor : "";
    return vendorString;
}

std::string ShaderMgr::GLRenderer() {
    const GLubyte *renderer = glGetString(GL_RENDERER);
    std::string rendererString = renderer ? (const char *)renderer : "";
    return rendererString;
}

std::string ShaderMgr::GLVersion() {
    const GLubyte *version = glGetString(GL_VERSION);
    std::string versionString = version ? (const char *)version : "";
    return versionString;
}

std::string ShaderMgr::GLShaderVersion() {
    const GLubyte *glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::string glslString = glsl ? (const char *)glsl : "";
    return glslString;
}

std::string ShaderMgr::GLExtensions() {
    const GLubyte *extensions = glGetString(GL_EXTENSIONS);
    std::string extensionString = extensions ? (const char *)extensions : "";
    return extensionString;
}

bool ShaderMgr::SupportsExtension(std::string extension) {
    std::string extensions = GLExtensions();
    if (extensions.find(extension) != std::string::npos)
        return true;
    else
        return false;
}

bool ShaderMgr::SupportsFeatures(std::string features) {
    if (glewIsSupported(features.c_str()))
        return true;
    else
        return false;
}

int ShaderMgr::GetUniformValuei(
    std::string effect, std::string variable, GLint *result) {

    if (!EffectExists(effect)) {
        SetErrMsg("Uniform %s not found", variable.c_str());
        return (-1);
    }

    GLint location = _effects[effect]->UniformLocation(variable);
    glGetUniformiv(_effects[effect]->GetProgram(), location, result);

    return (0);
}

int ShaderMgr::GetUniformValuef(
    std::string effect, std::string variable, GLfloat *result) {
    if (!EffectExists(effect)) {
        SetErrMsg("Uniform %s not found", variable.c_str());
        return (-1);
    }

    GLint location = _effects[effect]->UniformLocation(variable);
    glGetUniformfv(_effects[effect]->GetProgram(), location, result);

    return 0;
}

int ShaderMgr::MaxTexUnits(bool fixed) {
    if (fixed) {
        GLint result;
        glGetIntegerv(GL_MAX_TEXTURE_UNITS, &result);
        return (int)(result);
    } else {
        GLint result;
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &result);
        return (int)(result);
    }
}

bool ShaderMgr::CheckFramebufferStatus() {

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
    case GL_FRAMEBUFFER_COMPLETE:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        std::cout << "[ERROR] Framebuffer incomplete: Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Framebuffer incomplete: Unknown error." << std::endl;
        return false;
    }
}

int ShaderMgr::_getEfcFiles(
    string dirname, vector<string> &efcFiles) {
    efcFiles.clear();

    DIR *dir;
    dir = opendir(dirname.c_str());
    if (!dir) {
        SetErrMsg("Error opening directory %s : %M", dirname.c_str());
        return (-1);
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        string file = ent->d_name;
        string ext = file.size() >= 4 ? file.substr(file.size() - 4, 4) : "";
        if (ext.compare(".efc") == 0) {
            efcFiles.push_back(file);
        }
    }
    closedir(dir);
    return (0);
}
