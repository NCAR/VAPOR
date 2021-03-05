#include <vapor/glutil.h>
#include <vapor/ColorbarRenderer.h>
#include <stdio.h>
#include <vapor/RenderParams.h>
#include <vapor/LegacyGL.h>
#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/Texture.h>
#include <vapor/TextLabel.h>
#include <vapor/Font.h>
#include <assert.h>

using namespace VAPoR;
using glm::vec2;
using glm::vec3;
using glm::vec4;

static vec2 D2V2(vector<double> d)
{
    assert(d.size() >= 2);
    return vec2(d[0], d[1]);
}


static void DrawRect(LegacyGL *lgl, vec2 pos, vec2 size)
{
    lgl->Begin(LGL_QUADS);
    lgl->TexCoord2f(0, 0);
    lgl->Vertex2f(pos[0], pos[1]);
    lgl->TexCoord2f(0, 1);
    lgl->Vertex2f(pos[0], pos[1] + size[1]);
    lgl->TexCoord2f(1, 1);
    lgl->Vertex2f(pos[0] + size[0], pos[1] + size[1]);
    lgl->TexCoord2f(1, 0);
    lgl->Vertex2f(pos[0] + size[0], pos[1]);
    lgl->End();
}


static void DrawColorBar(LegacyGL *lgl, MapperFunction *mf, vec2 pos, vec2 size)
{
    float lut[4 * 256];
    mf->makeLut(lut);
    for (int i = 0; i < 256; i++) lut[4 * i + 3] = 1;

    Texture2D tex;
    tex.Generate();
    tex.TexImage(GL_RGB, 1, 256, 0, GL_RGBA, GL_FLOAT, lut);
    tex.Bind();

    lgl->EnableTexture();
    lgl->Color3f(1, 1, 1);
    DrawRect(lgl, pos, size);
    lgl->DisableTexture();
}


class Div {
public:
    vec2                 Size = vec2(0);
    function<void(void)> Draw;

    void Render(GLManager *glm, vec2 pos)
    {
        glm->matrixManager->PushMatrix();
        glm->matrixManager->Translate(pos.x, pos.y, 0);
        RenderInner(glm);
        glm->matrixManager->PopMatrix();
    }

protected:
    virtual void RenderInner(GLManager *glm) { Draw(); }
};

template<float vec2::*primaryAxis, float vec2::*secondaryAxis> class Stack : public Div {
    vector<Div *> _children;
    int           _spacing;

public:
    Stack(const vector<Div *> &c, int spacing = 5)
    {
        _children = c;
        _spacing = spacing;
        Size.*primaryAxis = -spacing;
        for (const auto d : c) {
            Size.*primaryAxis += d->Size.*primaryAxis;
            Size.*secondaryAxis = std::max(Size.*secondaryAxis, d->Size.*secondaryAxis);
            if (d->Size.*primaryAxis > 0) Size.*primaryAxis += spacing;
        }
    }

protected:
    virtual void RenderInner(GLManager *glm)
    {
        vec2 p(0);
        for (auto d : _children) {
            d->Render(glm, p);
            p.*primaryAxis += d->Size.*primaryAxis + _spacing;
        }
    };
};

struct HStack : public Stack<&vec2::x, &vec2::y> {
    using Stack::Stack;
};
struct VStack : public Stack<&vec2::y, &vec2::x> {
    using Stack::Stack;
};



void ColorbarRenderer::Render(GLManager *glm, RenderParams *rp)
{
    LegacyGL *     lgl = glm->legacy;
    ColorbarPbase *cp = rp->GetColorbarPbase();

    if (!cp->IsEnabled()) return;

    auto  size = D2V2(cp->GetSize());
    float scale = size.x;
    size.y += (1.f - size.y) * scale;
    size = glm::max(size, vec2(0.01, 0.01));
    glm->PixelCoordinateSystemPush();
    vec2 viewSize = glm->GetViewportSize();
    size *= viewSize;

    vec3   foregroundColor(0, 0, 0);
    vec3   backgroundColor(1, 1, 1);
    string fontName = "arimo";
    float  border = roundf(glm::length(viewSize) * 0.001 * (1 + scale * 2));
    float  padding = roundf(glm::length(viewSize) * 0.007 * (1 + scale * 2));

    vec2 colorbarSize = glm::round(vec2(glm::length(viewSize) * 0.03 * (1 + scale), size.y - padding * 2));
    colorbarSize = glm::max(colorbarSize, vec2(0, viewSize.y * 0.06f));

    int tickCount = cp->GetNumTicks();
    int fontSize = roundf(colorbarSize.y / tickCount * 0.8);
    fontSize = min(fontSize, (int)(padding * 1.8f));

    int manualFontSize = cp->GetFontSize();
    if (cp->GetValueLong("manual_font", false)) {
        fontSize = manualFontSize;

        padding = max(padding, fontSize / 2.f);
        colorbarSize.y = max(roundf(fontSize * tickCount / 0.8), colorbarSize.y);
        colorbarSize.x = roundf(max(colorbarSize.x, fontSize * 1.618f));
        size.y = roundf(colorbarSize.y + padding * 2);
    }

    int   titleFontSize = max(fontSize * 1.3f, padding * 2.f);
    float tickThickness = roundf(std::max(1.f, fontSize * 0.1f));
    float tickLength = roundf(std::max(8.f, fontSize * 0.4f));
    float tickPadding = 3;

    MapperFunction *mf = rp->GetMapperFunc(rp->GetVariableName());
    vec2            mfRange = D2V2(mf->getMinMaxMapValue());

    Div colorbar;
    colorbar.Size = colorbarSize;
    colorbar.Draw = [&]() { DrawColorBar(lgl, mf, vec2(0), colorbarSize); };

    Div ticks;
    ticks.Size = vec2(tickLength, tickThickness);
    ticks.Draw = [&]() {
        float h = colorbarSize.y;
        lgl->Color(foregroundColor);
        for (int i = 0; i < tickCount; i++) {
            vec2 tickPos = glm::round(vec2(0, (h - tickThickness) * i / (tickCount - 1.f)));
            DrawRect(lgl, tickPos, vec2(tickLength, tickThickness));
        }
    };

    TextLabel tickFont(glm, fontName, fontSize);
    tickFont.ForegroundColor = glm::vec4(foregroundColor, 1);
    tickFont.VerticalAlignment = TextLabel::Center;
    tickFont.HorizontalAlignment = TextLabel::Left;

    auto formatTick = makeFormatter(mf, cp->GetNumDigits(), cp->GetValueLong(cp->UseScientificNotationTag, false));
    auto valueForTick = [&](int i) -> float { return mfRange.x + (mfRange.y - mfRange.x) * i / (tickCount - 1.f); };
    auto textForTick = [&](int i) -> string { return formatTick(valueForTick(i)); };
    auto tickTextSize = [&](int i) -> vec2 { return tickFont.GetFont()->TextDimensions(textForTick(i)); };
    Div  tickLabels;
    for (int i = 0; i < tickCount; i++) { tickLabels.Size = glm::max(tickLabels.Size, tickTextSize(i)); }
    tickLabels.Draw = [&]() {
        float h = colorbarSize.y;
        lgl->Color(foregroundColor);
        for (int i = 0; i < tickCount; i++) {
            vec2 textPos = glm::round(vec2(0, tickThickness / 2.f + (h - tickThickness) * i / (tickCount - 1.f)));
            tickFont.DrawText(textPos, textForTick(i));
        }
    };

    HStack labledColorbar({&colorbar, &ticks, &tickLabels}, tickPadding);

    TextLabel titleFont(glm, fontName, titleFontSize);
    titleFont.ForegroundColor = glm::vec4(foregroundColor, 1);
    titleFont.VerticalAlignment = TextLabel::Bottom;
    titleFont.HorizontalAlignment = TextLabel::Left;
    Div    title;
    string titleText = cp->GetTitle();
    title.Size = titleFont.GetFont()->TextDimensions(titleText);
    title.Draw = [&]() { titleFont.DrawText(vec2(0), titleText); };

    VStack titledColorbar({&labledColorbar, &title}, padding * 1.618);
    size = titledColorbar.Size + vec2(padding * 2);

    vec2 corner = D2V2(cp->GetCornerPosition());
    vec2 pos = (corner * viewSize) - (size * corner);
    vec2 colorbarPos = glm::round(pos + padding);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    lgl->Color(foregroundColor);
    DrawRect(lgl, pos, size);
    lgl->Color(backgroundColor);
    DrawRect(lgl, pos + vec2(border), size - vec2(border * 2));

    titledColorbar.Render(glm, colorbarPos);

    glm->PixelCoordinateSystemPop();
}


std::function<std::string(float)> ColorbarRenderer::makeFormatter(MapperFunction *mf, int sigFigs, bool scientificNotation)
{
    vec2  mfRange = D2V2(mf->getMinMaxMapValue());
    float mfDiff = mfRange[1] - mfRange[0];
    float mfMax = max(abs(mfRange[0]), abs(mfRange[1]));
    int   diffLog10 = ceil(log10(mfDiff));
    int   maxLog10 = ceil(log10(mfMax));
    int   nDecimals = max(sigFigs - max(diffLog10, maxLog10), 0);

    auto formatTick = [=](float x) -> string {
        char buf[64];
        if (scientificNotation) {
            snprintf(buf, 64, "%.*e", sigFigs - 1, x);
        } else {
            if (x > FLT_EPSILON) {
                int exp = ceil(log10(x));
                int shift = sigFigs - exp;
                x = pow(10, -shift) * round((x * pow(10, shift)));
            }

            snprintf(buf, 64, "%.*f", nDecimals, x);
        }
        return string(buf);
    };
    return formatTick;
}
