#include "vapor/glutil.h"
#include "vapor/TextLabel.h"
#include "vapor/GLManager.h"
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

    mm->MatrixModeProjection();
    mm->PushMatrix();
    mm->Ortho(-1, 1, -1, 1, -1, 1);
    mm->MatrixModeModelView();
    mm->PushMatrix();
    mm->LoadIdentity();

    mm->Translate(p.x, p.y, 0);
    mm->Scale(1 / (float)viewport[2], 1 / (float)viewport[3], 1);
    mm->Scale(2, 2, 1);    // -1 -> +1

    vec2 textDimensions = font->TextDimensions(text);

    switch (HorizontalAlignment) {
    case Alignment::Left: break;
    case Alignment::Center: mm->Translate(-textDimensions.x / 2.f, 0, 0); break;
    case Alignment::Right: mm->Translate(-textDimensions.x, 0, 0); break;
    default: break;
    }
    switch (VerticalAlignment) {
    case Alignment::Bottom: break;
    case Alignment::Center: mm->Translate(0, -textDimensions.y / 2.f, 0); break;
    case Alignment::Right: mm->Translate(0, -textDimensions.y, 0); break;
    default: break;
    }

    glDepthMask(true);
    glDisable(GL_DEPTH_TEST);

    mm->Translate(0, 0, -1);

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

    glDepthMask(true);

    mm->PopMatrix();
    mm->MatrixModeProjection();
    mm->PopMatrix();
    mm->MatrixModeModelView();
}

Font *TextLabel::GetFont() const { return _glManager->fontManager->GetFont(FontName, FontSize); }
