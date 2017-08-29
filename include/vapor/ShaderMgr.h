/*
 *  ShaderMgr.h
 *
 *
 *  Created by Yannick Polius on 6/6/11.
 *  Copyright 2011 Yannick Polius. All rights reserved.
 *
 */

#ifndef ShaderMgr_h
#define ShaderMgr_h

#include <vapor/glutil.h>    // Must be included first!!!

#include <vector>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

#include <vapor/MyBase.h>

class ShaderProgram;

namespace VAPoR {
class RENDER_API ShaderMgr : public Wasp::MyBase {
public:
    ShaderMgr();
    ShaderMgr(std::string directory);
    ~ShaderMgr();

    void        SetShaderSourceDir(string directory);
    std::string GLVendor();
    std::string GLRenderer();
    std::string GLVersion();
    void        GLVersion(int &major, int &minor);
    std::string GLShaderVersion();
    std::string GLExtensions();
    bool        SupportsExtension(std::string extension);
    bool        SupportsFeatures(std::string features);
    void        SetGLSLVersion(int version);
    int         LoadShaders();
    int         ReloadShaders();
    int         EnableEffect(std::string effect);
    void        DisableEffect();
    int         UploadEffectData(std::string effect, std::string variable, int value);
    int         UploadEffectData(std::string effect, std::string variable, int value1, int value2);
    int         UploadEffectData(std::string effect, std::string variable, int value1, int value2, int value3);
    int         UploadEffectData(std::string effect, std::string variable, int value1, int value2, int value3, int value4);
    int         UploadEffectData(std::string effect, std::string variable, float value);
    int         UploadEffectData(std::string effect, std::string variable, float value1, float value2);
    int         UploadEffectData(std::string effect, std::string variable, float value1, float value2, float value3);
    int         UploadEffectData(std::string effect, std::string variable, float value1, float value2, float value3, float value4);
    int         DefineEffect(std::string baseName, std::string defines, std::string instanceName);
    int         GetUniformValuei(std::string effect, std::string variable, GLint *result);
    int         GetUniformValuef(std::string effect, std::string variable, GLfloat *result);
    bool        UndefEffect(std::string instanceName);
    bool        EffectExists(std::string effect);
    void        PrintEffects();
    bool        CheckFramebufferStatus();
    int         MaxTexUnits(bool fixed);

private:
    bool                                                           _loaded;
    int                                                            _glsl_version;
    std::map<std::string, ShaderProgram *>                         _effects;
    std::map<std::string, std::map<int, std::vector<std::string>>> _baseEffects;
    std::string                                                    _sourceDir;
    std::string                                                    _findEffect(GLuint prog);

    int _loadEffectFile(std::string file);

    std::string _convertDefines(std::string defines);

    int _loadVertShader(std::string path, std::string fileName, std::string effectName);

    int _loadFragShader(std::string path, std::string fileName, std::string effectName);

    std::vector<std::string> _preprocess(const string &source, std::string &processed);

    int _getEfcFiles(std::string dirname, std::vector<std::string> &efcFiles);
};
};    // namespace VAPoR
#endif
