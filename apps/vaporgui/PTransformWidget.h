#pragma once

#include "PWidget.h"
#include <vector>

class V3DInput;
class VGroup;
namespace VAPoR {
class Transform;
}

class PTransformWidget : public PWidget {
    VGroup *       _group;
    V3DInput *     _translate;
    V3DInput *     _scale;
    V3DInput *     _origin;

public:
    PTransformWidget();

protected:
    void updateGUI() const override;

private:
    void translateChanged(const std::vector<double> xyz);
    void scaleChanged(const std::vector<double> xyz);
    void originChanged(const std::vector<double> xyz);
};

class PRendererTransformWidget : public PWidget {
    PTransformWidget *_widget;

public:
    PRendererTransformWidget();

protected:
    void updateGUI() const override;
};


#include "PWidgetWrapper.h"


class PRendererTransformSection : public PWidgetWrapper {
public:
    PRendererTransformSection();
};
