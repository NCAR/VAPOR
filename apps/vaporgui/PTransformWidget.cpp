#include "PTransformWidget.h"
#include "VSection.h"
#include "V3DInput.h"
#include "VLineItem.h"
#include <vapor/RenderParams.h>
#include <QLabel>

using VAPoR::RenderParams;
using VAPoR::Transform;

PTransformWidget::PTransformWidget() : PWidget("", _group = new VSectionGroup("Transform"))
{
    _group->Add(new VLineItem("Translate", _translate = new V3DInput));
    _group->Add(new VLineItem("Scale      ", _scale = new V3DInput));
    _group->Add(new VLineItem("Origin     ", _origin = new V3DInput));

    QObject::connect(_translate, &V3DInput::ValueChangedVec, this, &PTransformWidget::translateChanged);
    QObject::connect(_scale, &V3DInput::ValueChangedVec, this, &PTransformWidget::scaleChanged);
    QObject::connect(_origin, &V3DInput::ValueChangedVec, this, &PTransformWidget::originChanged);
}

void PTransformWidget::updateGUI() const
{
    Transform *t = getParams<Transform>();

    _translate->SetValue(t->GetTranslations());
    _scale->SetValue(t->GetScales());
    _origin->SetValue(t->GetOrigin());
}

void PTransformWidget::translateChanged(const std::vector<double> xyz) { getParams<Transform>()->SetTranslations(xyz); }

void PTransformWidget::scaleChanged(const std::vector<double> xyz) { getParams<Transform>()->SetScales(xyz); }

void PTransformWidget::originChanged(const std::vector<double> xyz) { getParams<Transform>()->SetOrigin(xyz); }

PRendererTransformWidget::PRendererTransformWidget() : PWidget("", _widget = new PTransformWidget) {}

void PRendererTransformWidget::updateGUI() const
{
    RenderParams *rp = getParams<RenderParams>();
    Transform *   t = rp->GetTransform();
    _widget->Update(t, getParamsMgr(), getDataMgr());
}
