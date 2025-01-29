#include "PTFEditor.h"
#include <vapor/MyBase.h>
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
#include <typeinfo>

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

PTFEditor::PTFEditor() : PTFEditor(RenderParams::_variableNameTag) {}

PTFEditor::PTFEditor(const std::string &tag, const std::set<Element> elements, const std::string &label, bool expandable) : PWidget(tag, _section = new VSection(label.empty() ? tag : label)), _expandable(expandable)
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

    if (_expandable) {
        _section->setExpandedSection();
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

    if (_expandedPTFEditor != nullptr ) {
        _expandedPTFEditor->Update(rp, pm, dm);
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

void PTFEditor::Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr) {
    PWidget::Update(params, paramsMgr, dataMgr);
    if (_expandable==true) {
        GUIStateParams* p = dynamic_cast<GUIStateParams *>(paramsMgr->GetParams(GUIStateParams::GetClassType()));
        vector<string> editors = p->GetExpandedPTFEditors();
        std::string name, inst;
        getExpandedPTFEditorInfo(name, inst);

        // If an expanded editor exists in the GUIStateParams, create and show a new expanded PTFEditor
        if (_expandedPTFEditor==nullptr) {
            auto it = std::find(editors.begin(), editors.end(), name); 
            if(it != editors.end()) showExpandedPTFEditor();
            else std::cout << "Not showing " << name << std::endl;
        }

        // Once the expanded PTFEditor is created, set GUIStateParams to record it as belonging to the
        // currently active render instance, then Update it with the currently active params
        if (_expandedPTFEditor!=nullptr) {
            //std::string type = dynamic_cast<VAPoR::RenderParams *>(params)->GetClassType();
            //std::string type = params->GetClassType();
            // Hack to get the renderer class type.  Is there a better way to do this?  GetClassType() requires
            // that we know the RenderParams derived class which we don't know.  IE - SliceParams::GetClassType()
            //std::vector<std::string> splitName;
            //Wasp::SplitString(name, '_', splitName);
            //for (size_t i=0; i<splitName.size(); i++) std::cout << "    " << splitName[i] << std::endl;
            //std::string type = splitName[0];

            //std::string foo = dynamic_cast<RenderParams*>(params)->GetClassType();
            //std::cout << "GetClassType() " << foo << std::endl;

            //auto it = std::find_if(editors.begin(), editors.end(), [&type](const std::string& str) {return str.find(type) != std::string::npos;});
            //if (it != editors.end()) {
            //    paramsMgr->BeginSaveStateGroup("Replace current expanded PTFEditor with currently active renderer instance");
            //    std::cout << "Replacing " << *it << " with " << name << std::endl;
            //    p->RemoveExpandedPTFEditor(*it);
            //    p->SetExpandedPTFEditor(name);
            //    paramsMgr->EndSaveStateGroup();
            //}
            _expandedPTFEditor->setWindowTitle(QString::fromStdString(name));
            _expandedPTFEditor->Update(params, paramsMgr, dataMgr);
        }
    }
}

void PTFEditor::getExpandedPTFEditorInfo(std::string &name, std::string& type) {
    ParamsMgr* pm = getParamsMgr();
    GUIStateParams* p = dynamic_cast<GUIStateParams *>(pm->GetParams(GUIStateParams::GetClassType()));
    type = p->GetActiveRendererInst();
    p->GetActiveRenderer(p->GetActiveVizName(), type, name);
    if (dynamic_cast<PColormapTFEditor*>(this)) {
        name+="_Colormap";
        type+="_Colormap";
    }
}

void PTFEditor::showExpandedPTFEditor() {
    //std::string name = getExpandedPTFEditorName()[0];
    if (_expandedPTFEditor==nullptr) {
        _expandedPTFEditor = new PTFEditor(_tag, _elements, _label, false);
        connect(_expandedPTFEditor, SIGNAL(closed()), this, SLOT(closeExpandedPTFEditor()));

        if (_showColormapBasedOnParam==true) _expandedPTFEditor->ShowColormapBasedOnParam(_showColormapBasedOnParamTag, _showColormapBasedOnParamValue);
        if (_showOpacityBasedOnParam==true) _expandedPTFEditor->ShowOpacityBasedOnParam(_showOpacityBasedOnParamTag, _showOpacityBasedOnParamValue);

        //VAPoR::ParamsMgr* pm = getParamsMgr();
        //GUIStateParams*    p = dynamic_cast<GUIStateParams *>(pm->GetParams(GUIStateParams::GetClassType()));
        //p->SetExpandedPTFEditor(name);

        _expandedPTFEditor->setAttribute(Qt::WA_ShowWithoutActivating);
        _expandedPTFEditor->setAttribute(Qt::WA_DeleteOnClose);
    }
    _expandedPTFEditor->raise();
}

void PTFEditor::closeExpandedPTFEditor() {
    VAPoR::ParamsMgr *   pm = getParamsMgr();
    GUIStateParams* p = dynamic_cast<GUIStateParams *>(pm->GetParams(GUIStateParams::GetClassType()));
    std::string name, type;
    getExpandedPTFEditorInfo(name, type);
    p->RemoveExpandedPTFEditor(type);

    _expandedPTFEditor->close();
    _expandedPTFEditor=nullptr;
}

void PTFEditor::closeEvent(QCloseEvent* event) {
    emit closed();
    close();
}

PColormapTFEditor::PColormapTFEditor(VAPoR::ControlExec* ce) : PTFEditor(RenderParams::_colorMapVariableNameTag, {PTFEditor::Histogram, PTFEditor::Colormap}, "Colormap Transfer Function") {}
