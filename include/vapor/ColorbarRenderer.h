#pragma once

#include <functional>
#include <string>

namespace VAPoR {
struct GLManager;
class RenderParams;
class MapperFunction;

//! \class ColorbarRenderer
//! \ingroup Public_Render
//! \brief Renders annotation colorbar for a given Mapper Function
//! \author Stanislaw Jaroszynski
//!
class ColorbarRenderer {
public:
    static void Render(GLManager *glm, RenderParams *rp);

private:
    static std::function<std::string(float)> makeFormatter(MapperFunction *mf, int sigFigs, bool scientific);
};
}    // namespace VAPoR
