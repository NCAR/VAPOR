#include "PRegionSelector.h"
#include "QRangeSliderTextCombo.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include <QVBoxLayout>

PRegionSelector::PRegionSelector(const std::string &tag, const std::string &label) : PWidget(tag, _container = new QWidget)
{
    assert(!"This class is not finished. Do not use. Waiting on vgeometry selector to be fixed.");

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    _container->setLayout(layout);

    layout->addWidget(new VLineItem("X", _sx = new QRangeSliderTextCombo));
    layout->addWidget(new VLineItem("Y", _sy = new QRangeSliderTextCombo));
    layout->addWidget(new VLineItem("Z", _sz = new QRangeSliderTextCombo));

    _sx->setEnabled(false);
    _sy->setEnabled(false);
    _sz->setEnabled(false);

    //    _vgeometry->SetRange(<#const std::vector<float> &range#>)
    // connect(_qcheckbox, &VCheckBox::ValueChanged, this, &PRegionSelector::checkboxStateChanged);
}

void PRegionSelector::updateGUI() const
{
    bool on = getParams()->GetValueLong(getTag(), 0);
    //_qcheckbox->SetValue(on);
}

void PRegionSelector::checkboxStateChanged(bool on) { getParams()->SetValueLong(getTag(), "", on); }
