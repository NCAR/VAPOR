#include "vapor/glutil.h"
#include "vapor/Font.h"
#include <cassert>
#include "vapor/ShaderManager.h"
#include <glm/glm.hpp>
#include "vapor/GLManager.h"

using namespace VAPoR;
using glm::vec2;
using std::string;

Font::Font(GLManager *glManager, const std::string &path, int size, FT_Library library) : _glManager(glManager), _library(nullptr), _size(size)
{
    if (library == nullptr) {
        assert(!FT_Init_FreeType(&_library));
        library = _library;
    }
    assert(!FT_New_Face(library, path.c_str(), 0, &_face));
    FT_Set_Pixel_Sizes(_face, 0, _size);

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Font::~Font()
{
    FT_Done_Face(_face);
    if (_library) FT_Done_FreeType(_library);

    glDeleteVertexArrays(1, &_VAO);
    glDeleteBuffers(1, &_VBO);
    for (auto it = _glyphMap.begin(); it != _glyphMap.end(); ++it) glDeleteTextures(1, &it->second.textureID);
}

bool Font::LoadGlyph(int c)
{
    if (FT_Load_Char(_face, c, FT_LOAD_RENDER)) {
        printf("FAILED TO LOAD CHAR\n");
        return false;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, _face->glyph->bitmap.width, _face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, _face->glyph->bitmap.buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    _glyphMap[c] = {texture, _face->glyph->bitmap.width, _face->glyph->bitmap.rows, _face->glyph->bitmap_left, _face->glyph->bitmap_top, _face->glyph->advance.x};

    return true;
}

Font::Glyph Font::GetGlyph(int c)
{
    auto it = _glyphMap.find(c);
    if (it == _glyphMap.end()) {
        LoadGlyph(c);
        return _glyphMap[c];
    }
    return it->second;
}

void Font::DrawText(const std::string &text, const glm::vec4 &color)
{
    SmartShaderProgram shader = _glManager->shaderManager->GetSmartShader("font");
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("color", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float xStart = 0;
    const float yStart = 0;

    float cursorX = xStart;
    float cursorY = yStart;

    for (int i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            cursorY -= LineHeight();
            cursorX = xStart;
            continue;
        }
        if (text[i] == '\r') {
            cursorX = xStart;
            continue;
        }

        Glyph ch = GetGlyph(text[i]);

        float x = cursorX + ch.bearingX;
        float y = cursorY - (ch.sizeY - ch.bearingY);
        float w = ch.sizeX;
        float h = ch.sizeY;

        float vertices[6][4] = {{x, y + h, 0, 0}, {x, y, 0, 1},     {x + w, y, 1, 1},

                                {x, y + h, 0, 0}, {x + w, y, 1, 1}, {x + w, y + h, 1, 0}};
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        cursorX += ch.advance / 64;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
}

glm::vec2 Font::TextDimensions(const std::string &text)
{
    vec2  dimensions(0.f);
    float lineWidth = 0;
    float maxHeightForLine = 0;
    for (int i = 0; i < text.size(); i++) {
        if (text[i] == '\n') {
            if (lineWidth > dimensions.x) dimensions.x = lineWidth;
            dimensions.y += LineHeight();
            maxHeightForLine = 0;
            lineWidth = 0;
            continue;
        }
        if (text[i] == '\r') {
            if (lineWidth > dimensions.x) dimensions.x = lineWidth;
            lineWidth = 0;
            continue;
        }

        Glyph ch = GetGlyph(text[i]);
        lineWidth += ch.advance / 64;
        if (ch.sizeY > maxHeightForLine) maxHeightForLine = ch.sizeY;
    }
    if (lineWidth > dimensions.x) dimensions.x = lineWidth;
    dimensions.y += maxHeightForLine;
    return dimensions;
}

float Font::LineHeight() const
{
    return _face->size->metrics.height / 64;
    // return _face->height / 64;
}
