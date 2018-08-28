#pragma once

#include <string>
#include <map>
#include <glm/fwd.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace VAPoR {

struct GLManager;

class Font {
    struct Glyph {
        unsigned int textureID;
        unsigned int sizeX;
        unsigned int sizeY;
        int          bearingX;
        int          bearingY;
        long         advance;
    };

    GLManager *_glManager;
    FT_Library _library;
    FT_Face    _face;

    std::map<int, Glyph> _glyphMap;
    int                  _size;
    unsigned int         _VAO, _VBO;

    bool  LoadGlyph(int c);
    Glyph GetGlyph(int c);

public:
    Font(GLManager *glManager, const std::string &path, int size, FT_Library library = nullptr);
    ~Font();

    void DrawText(const std::string &text, float x = 0, float y = 0);
};

}    // namespace VAPoR
