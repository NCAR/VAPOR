#pragma once

#include <string>
#include <map>
#include <glm/glm.hpp>
#include "vapor/MyBase.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace VAPoR {

struct GLManager;

//! \class Font
//! \ingroup Public_Render
//!
//! \brief Renders charachter glyphs using FreeType2
//! This class does not do any transformation, formatting,
//! etc., please use the TextLabel class for that.
//!
//! \author Stanislaw Jaroszynski

class RENDER_API Font : public Wasp::MyBase {
    struct Glyph {
        unsigned int textureID;
        int          sizeX;
        int          sizeY;
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

    //! Draws text in pixel coordinates i.e. if font is 10px,
    //! text will be 10 OpenGL units tall.
    //!
    //! \param[in] text
    //! \param[in] color default is white
    //!
    void DrawText(const std::string &text, const glm::vec4 &color = glm::vec4(1));

    //! Returns pixel dimensions of text
    //!
    glm::vec2 TextDimensions(const std::string &text);
    float     LineHeight() const;
};

}    // namespace VAPoR
