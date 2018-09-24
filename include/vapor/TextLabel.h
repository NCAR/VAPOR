#pragma once

#include <string>
#include <glm/glm.hpp>

namespace VAPoR {

struct GLManager;
class Font;

//! \class TextLabel
//! \ingroup Public_Render
//!
//! \brief Utitlity wrapper for Font class
//!
//! Creates a 2D label for a point in 2D or 3D space
//!
//! \author Stanislaw Jaroszynski
//! \date   August, 2018

class TextLabel {
    GLManager *_glManager;

public:
    enum Alignment { Left, Center, Right, Top, Bottom };

    std::string  FontName;
    unsigned int FontSize;
    glm::vec4    BackgroundColor;
    glm::vec4    ForegroundColor;
    Alignment    HorizontalAlignment;
    Alignment    VerticalAlignment;
    float        Padding;

    TextLabel(GLManager *glManager, const std::string &fontName, unsigned int fontSize);

    //! Draws a floating text label if a 3D perspective is used or simply text
    //! if using a pixel coordinate space matrix. Formatting is determined by the
    //! public variables.
    //!
    //! \param[in] position will be transformed according to the current ModelViewProjection matrix
    //! \param[in] text will be drawn
    //!
    void DrawText(const glm::vec3 &position, const std::string &text);
    void DrawText(const glm::vec2 &position, const std::string &text);

    Font *GetFont() const;
};

}    // namespace VAPoR
