#include "PTFEditor.h"
#include <vapor/RenderParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>
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

//PTFEditor::PTFEditor() : PTFEditor(RenderParams::_variableNameTag) {}
PTFEditor::PTFEditor(VAPoR::ControlExec* ce) : PTFEditor(RenderParams::_variableNameTag) {_ce = ce;}

//PTFEditor::PTFEditor(const std::string &tag, const std::set<Element> elements, const std::string &label, bool expandable) : PWidget(tag, _section = new VSection(label.empty() ? tag : label))
PTFEditor::PTFEditor(const std::string &tag, const std::set<Element> elements, const std::string &label, bool expandable) : PWidget(tag, _section = new VSection(label.empty() ? tag : label))
{
    _maps = new TFMapGroupWidget;
    _histogram = new TFHistogramMap(tag);
    _opacityMap = new TFOpacityMap(tag);
    _colorMap = new TFColorMap(tag);
    _isoMap = new TFIsoValueMap(tag);
    _range = new TFMappingRangeSelector(tag);
    _elements = elements;
    _label = label;

    _maps->Add({_opacityMap, _histogram});
    _maps->Add(_isoMap);
    _maps->Add(_colorMap);

    _section->layout()->addWidget(_maps, 1);
    _section->layout()->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    _section->layout()->addWidget(_range, 0);
    connect(_range, SIGNAL(ValueChangedIntermediate(float, float)), _histogram, SLOT(update()));

    int    start = 0;
    QMenu *menu = new QMenu;
    _colorMap->PopulateSettingsMenu(menu);
    for (int i = start; i < menu->actions().size(); i++) _colorMapActions.push_back(menu->actions()[i]);

    menu->addSeparator();

    start = menu->actions().size();
    _opacityMap->PopulateSettingsMenu(menu);
    for (int i = start; i < menu->actions().size(); i++) _opacityMapActions.push_back(menu->actions()[i]);

    menu->addSeparator();

    start = menu->actions().size();
    _histogram->PopulateSettingsMenu(menu);
    for (int i = start; i < menu->actions().size(); i++) _histogramActions.push_back(menu->actions()[i]);

    if (expandable) {
        // 1/27/2025
        //connect(_expandedPTFEditor, SIGNAL(shown()), this, SLOT(showExpandedPTFEditor()));
        //connect(_expandedPTFEditor, SIGNAL(closed()), this, SLOT(closeExpandedPTFEditor()));
        //_expandedPTFEditor = new PTFEditor(tag, elements, label, false);
        //
        //_expandedPTFEditor->setAttribute(Qt::WA_ShowWithoutActivating);
        //_expandedPTFEditor->setAttribute(Qt::WA_DeleteOnClose);
        //std::cout << "creating new _PTFEditor" << std::endl;


        //_expandedPTFEditor->setWindowFlags(_expandedPTFEditor->windowFlags() | Qt::WindowStaysOnBottomHint);

        //_expandedPTFEditor->Update(rp, pm, dm);
        //_expandedPTFEditor->updateGUI();
        //_expandedPTFEditor->hide();
        //connect(this, SIGNAL(show()), _expandedPTFEditor, SLOT(showExpandedPTFEditor()));
        //connect(_expandedPTFEditor, &PTFEditor::shown, this, SLOT(showExpandedPTFEditor()));
        //connect(_expandedPTFEditor, &PTFEditor::shown, SLOT(showExpandedPTFEditor()));
        //connect(_expandedPTFEditor, PTFEditor::shown(), this, SLOT(showExpandedPTFEditor()));


        //connect(_expandedPTFEditor, SIGNAL(closed()), this, SLOT(closeEvent()));
        //connect(this, SIGNAL(shown()), _expandedPTFEditor, SLOT(dynamic_cast<GUIStateParams *>_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType())->SetExpandedPTFEditor("foo")));

        _section->setExpandSection(_expandedPTFEditor);
        //connect(_section, SIGNAL(expandButtonClicked()), this, SLOT(showExpandedPTFEditor));
        //connect(_section, &VSection::expandButtonClicked, this, SLOT(showExpandedPTFEditor));
        connect(_section, &VSection::expandButtonClicked, this, &PTFEditor::showExpandedPTFEditor);
    }

    _section->setMenu(menu);

    _histogram->hide();
    _opacityMap->hide();
    _colorMap->hide();
    _isoMap->hide();

    if (elements.count(Opacity)) _opacityMap->show();
    if (elements.count(Colormap)) _colorMap->show();
    if (elements.count(Histogram)) _histogram->show();
    if (elements.count(IsoValues) || elements.count(RegularIsoArray)) _isoMap->show();

    if (elements.count(RegularIsoArray))
        _isoMap->SetEquidistantIsoValues(true);
    else
        _isoMap->SetEquidistantIsoValues(false);

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
    //std::cout << "updatin? " << (_expandedPTFEditor != nullptr) << " " << _expandedPTFEditor->isVisible() << std::endl;
    //if (_expandedPTFEditor != nullptr && _expandedPTFEditor->isVisible()) {
    std::cout << "is null " << (_expandedPTFEditor==nullptr) << std::endl;
    if (_expandedPTFEditor != nullptr ) {
        GUIStateParams * guiParams = dynamic_cast<GUIStateParams *>(_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
        _expandedPTFEditor->Update(rp, pm, dm);
        //if (_showExpandedPTFEditorBasedOnParam) _expandedPTFEditor->Update(rp, pm, dm);
    }

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

    for (auto a : _colorMapActions) a->setEnabled(_colorMap->IsShown());
    for (auto a : _opacityMapActions) a->setEnabled(_opacityMap->IsShown());
    for (auto a : _histogramActions) a->setEnabled(_histogram->IsShown());
}

//void PTFEditor::Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr)
//{
//    std::cout << "MyUpdate!" << std::endl;
//    _params = params;
//    _paramsMgr = paramsMgr;
//    _dataMgr = dataMgr;
//
//    if (_dynamicUpdateInsideGroup) return;
//
//    if (params == nullptr) {
//        this->setDisabled(true);
//        return;
//    }
//    if (requireDataMgr() && !dataMgr) VAssert(!"Data manager required but missing");
//    if (requireParamsMgr() && !paramsMgr) VAssert(!"Params manager required but missing");
//
//    bool paramsVisible = isShown();
//    if (paramsVisible && _showBasedOnParam) paramsVisible = _showBasedOnParamValue == params->GetValueLong(_showBasedOnParamTag, 0);
//    if (paramsVisible) {
//        setVisible(true);
//        //setVisible(true);
//    } else {
//        setVisible(false);
//        return;
//    }
//
//    bool enabled = isEnabled();
//    if (enabled && _enableBasedOnParam)
//        enabled = params->GetValueLong(_enableBasedOnParamTag, 0) == _enableBasedOnParamValue;
//    setEnabled(enabled);
//
//    updateGUI();
//}

//void PTFEditor::showExpandedPTFEditor() {
void PTFEditor::showExpandedPTFEditor() {

    std::cout << "showin?" << std::endl;
    if (_expandedPTFEditor==nullptr){
        std::cout << "    creating new" << std::endl;
        _expandedPTFEditor = new PTFEditor(_tag, _elements, _label, false);
        connect(_expandedPTFEditor, SIGNAL(closed()), this, SLOT(closeExpandedPTFEditor()));

        _expandedPTFEditor->setAttribute(Qt::WA_ShowWithoutActivating);
        _expandedPTFEditor->setAttribute(Qt::WA_DeleteOnClose);
    }

    VAPoR::DataMgr *     dm = getDataMgr();
    VAPoR::ParamsMgr *   pm = getParamsMgr();
    VAPoR::RenderParams *rp = dynamic_cast<VAPoR::RenderParams *>(getParams());
    _expandedPTFEditor->Update(rp, pm, dm);
        
    //std::string name =  
    GUIStateParams* p = dynamic_cast<GUIStateParams *>(_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    p->SetExpandedPTFEditor("foo");

    std::vector<string> names;
    getParamsMgr()->GetRenderParamNames(p->GetActiveVizName(), p->GetActiveDataset(), names);
    //std::cout << names.size() << std::endl;
    //for (auto name : names) std::cout << "Name: " << name << std::endl;
}

void PTFEditor::closeExpandedPTFEditor() {
    std::cout << "closin?" << std::endl;
    _expandedPTFEditor->close();
    _expandedPTFEditor=nullptr;
    //std::string name =  
    //GUIStateParams* p = dynamic_cast<GUIStateParams *>(_ce->GetParamsMgr()->GetParams(GUIStateParams::GetClassType()));
    //p->SetExpandedPTFEditor("foo");

    //std::vector<string> names;
    //getParamsMgr()->GetRenderParamNames(p->GetActiveVizName(), p->GetActiveDataset(), names);
    //std::cout << names.size() << std::endl;
    //for (auto name : names) std::cout << "Name: " << name << std::endl;
}

void PTFEditor::closeEvent(QCloseEvent* event) {
    emit closed();
    close();
}

void PTFEditor::showEvent(QShowEvent* event) {
    emit shown();
    QWidget::showEvent(event);
}

PColormapTFEditor::PColormapTFEditor() : PTFEditor(RenderParams::_colorMapVariableNameTag, {PTFEditor::Histogram, PTFEditor::Colormap}, "Colormap Transfer Function") {}
