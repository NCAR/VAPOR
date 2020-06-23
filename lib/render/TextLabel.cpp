#include "vapor/glutil.h"
#include "vapor/TextLabel.h"
#include "vapor/GLManager.h"
#include <vapor/FontManager.h>
#include "vapor/LegacyGL.h"

using namespace VAPoR;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::string;

TextLabel::TextLabel(GLManager *glManager, const std::string &fontName, unsigned int fontSize)
: _glManager(glManager), FontName(fontName), FontSize(fontSize), BackgroundColor(0.f), ForegroundColor(1.f), HorizontalAlignment(Left), VerticalAlignment(Bottom), Padding(0)
{
}

void TextLabel::DrawText(const glm::vec2 &position, const std::string &text) { DrawText(vec3(position, 0.f), text); }

void TextLabel::DrawText(const glm::vec3 &position, const std::string &text)
{
    Font *         font = _glManager->fontManager->GetFont(FontName, FontSize);
    MatrixManager *mm = _glManager->matrixManager;
    LegacyGL *     lgl = _glManager->legacy;
    glm::vec2      p = mm->ProjectToScreen(position);

    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);

    vec4  ndc = mm->GetModelViewProjectionMatrix() * vec4(position, 1.f);
    float z = ndc.z / ndc.w;

    mm->MatrixModeProjection();
    mm->PushMatrix();
    mm->Ortho(0, viewport[2], 0, viewport[3]);
    mm->MatrixModeModelView();
    mm->PushMatrix();
    mm->LoadIdentity();

    float x = (p.x + 1) / 2 * viewport[2];
    float y = (p.y + 1) / 2 * viewport[3];

    vec2 textDimensions = font->TextDimensions(text);

    switch (HorizontalAlignment) {
    case Alignment::Left: break;
    case Alignment::Center: x -= textDimensions.x / 2.f; break;
    case Alignment::Right: x -= textDimensions.x; break;
    default: break;
    }
    switch (VerticalAlignment) {
    case Alignment::Bottom: break;
    case Alignment::Center: y -= textDimensions.y / 2.f; break;
    case Alignment::Top: y -= textDimensions.y; break;
    default: break;
    }

    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    mm->Translate((int)x, (int)y, -z);

    if (BackgroundColor.a > 0) {
        lgl->Color(BackgroundColor);
        lgl->Begin(LGL_QUADS);
        lgl->Vertex2f(-Padding, -Padding);
        lgl->Vertex2f(textDimensions.x + Padding, -Padding);
        lgl->Vertex2f(textDimensions.x + Padding, textDimensions.y + Padding);
        lgl->Vertex2f(-Padding, textDimensions.y + Padding);
        lgl->End();
    }

    font->DrawText(text, ForegroundColor);

    glDepthFunc(GL_LESS);

    mm->PopMatrix();
    mm->MatrixModeProjection();
    mm->PopMatrix();
    mm->MatrixModeModelView();
}

Font *TextLabel::GetFont() const { return _glManager->fontManager->GetFont(FontName, FontSize); }
