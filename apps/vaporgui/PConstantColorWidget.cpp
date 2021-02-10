#include "PConstantColorWidget.h"
#include "PCheckbox.h"
#include "PColorSelector.h"
#include <vapor/RenderParams.h>

using VAPoR::RenderParams;

PConstantColorWidget::PConstantColorWidget()
{
    Add(new PCheckbox(RenderParams::_useSingleColorTag, "Use Constant Color"));
    Add((new PSubGroup({new PColorSelector(RenderParams::_constantColorTag, "Constant Color")}))->ShowBasedOnParam(RenderParams::_useSingleColorTag));
}
