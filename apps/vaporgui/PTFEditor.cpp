#include "PTFEditor.h"
#include <vapor/RenderParams.h>
#include "TFColorWidget.h"
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFIsoValueWidget.h"
#include "TFMappingRangeSelector.h"
#include "TFMapGroupWidget.h"
#include "VSection.h"
#include "PDisplay.h"

using VAPoR::RenderParams;

template<typename T> PTFMapWidget<T>::PTFMapWidget(const std::string &tag) : PWidget(tag, _tfWidget = new TFColorWidget(tag)) {}

template<typename T> void PTFMapWidget<T>::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    VAssert(rp);

    _tfWidget->Update(getDataMgr(), getParamsMgr(), rp);
}

template class PTFMapWidget<TFColorWidget>;
template class PTFMapWidget<TFOpacityWidget>;
template class PTFMapWidget<TFHistogramWidget>;
template class PTFMapWidget<TFIsoValueWidget>;

PTFEditor::PTFEditor(const std::string &tag, const std::set<Element> elements, const std::string &label) : PWidget(tag, _section = new VSection(label.empty() ? tag : label))
{
    _maps = new TFMapGroupWidget;
    _histogram = new TFHistogramMap(tag);
    _opacityMap = new TFOpacityMap(tag);
    _colorMap = new TFColorMap(tag);
    _isoMap = new TFIsoValueMap(tag);
    _range = new TFMappingRangeSelector(tag);

    _maps->Add({_opacityMap, _histogram});
    _maps->Add(_isoMap);
    _maps->Add(_colorMap);

    _section->layout()->addWidget(_maps);
    _section->layout()->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    _section->layout()->addWidget(_range);
    connect(_range, SIGNAL(ValueChangedIntermediate(float, float)), _histogram, SLOT(update()));

    QMenu *menu = new QMenu;
    _colorMap->PopulateSettingsMenu(menu);
    menu->addSeparator();
    _opacityMap->PopulateSettingsMenu(menu);
    menu->addSeparator();
    _histogram->PopulateSettingsMenu(menu);
    _section->setMenu(menu);

    _histogram->hide();
    _opacityMap->hide();
    _colorMap->hide();
    _isoMap->hide();

    if (elements.count(Opacity)) _opacityMap->show();
    if (elements.count(Colormap)) _colorMap->show();
    if (elements.count(Histogram)) _histogram->show();
    if (elements.count(IsoValues) || elements.count(RegularIsoArray)) _isoMap->show();

    if (elements.count(RegularIsoArray)) _isoMap->SetEquidistantIsoValues(true);

    if (elements.count(Default)) {
        _histogram->show();
        _opacityMap->show();
        _colorMap->show();
        _isoMap->show();
    }
}

PTFEditor *PTFEditor::ShowOpacityBasedOnParam(const std::string &tag, int value)
{
    _showOpacityBasedOnParam = true;
    _showOpacityBasedOnParamTag = tag;
    _showOpacityBasedOnParamValue = value;
    return this;
}

PTFEditor *PTFEditor::ShowColormapBasedOnParam(const std::string &tag, int value)
{
    _showColormapBasedOnParam = true;
    _showColormapBasedOnParamTag = tag;
    _showColormapBasedOnParamValue = value;
    return this;
}

void PTFEditor::updateGUI() const
{
    VAPoR::DataMgr *     dm = getDataMgr();
    VAPoR::ParamsMgr *   pm = getParamsMgr();
    VAPoR::RenderParams *rp = dynamic_cast<VAPoR::RenderParams *>(getParams());
    VAssert(rp);

    _maps->Update(dm, pm, rp);
    _mapsInfo->Update(rp);
    _range->Update(dm, pm, rp);

    if (_showOpacityBasedOnParam) {
        if (rp->GetValueLong(_showOpacityBasedOnParamTag, 0) == _showOpacityBasedOnParamValue)
            _opacityMap->show();
        else
            _opacityMap->hide();
    }

    if (_showColormapBasedOnParam) {
        if (rp->GetValueLong(_showColormapBasedOnParamTag, 0) == _showColormapBasedOnParamValue)
            _colorMap->show();
        else
            _colorMap->hide();
    }
}
