#include "TFEditor.h"
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QFileDialog>
#include "TFOpacityWidget.h"
#include "TFHistogramWidget.h"
#include "TFColorWidget.h"
#include "TFOpacityInfoWidget.h"
#include "TFColorInfoWidget.h"
#include "QRangeSlider.h"
#include "QRangeSliderTextCombo.h"
#include <vapor/ColorMap.h>
#include <vapor/ResourcePath.h>
#include <vapor/FileUtils.h>
#include "SettingsParams.h"
#include "ErrorReporter.h"

using namespace Wasp;
using namespace VAPoR;

TFEditor::TFEditor()
{
    addTab(new QWidget(this), "Transfer Function");
    
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(12);
    _tab()->setLayout(layout);
    
    _tool = new SettingsMenu;
    setCornerWidget(_tool);
    setStyleSheet(_createStylesheet());
    
    layout->addWidget(_maps = new TFMapsGroup);
    layout->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    layout->addWidget(range = new QRangeSliderTextCombo);
    layout->addWidget(colorMapTypeDropdown = new ParamsWidgetDropdown(VAPoR::ColorMap::_interpTypeTag, {"Linear", "Discrete", "Diverging"}, "Color Interpolation"));
    
    
    QMenu *menu = new QMenu;
    menu->addAction("Save Colormap", this, SLOT(_saveTransferFunction()));
    menu->addAction("Load Colormap", this, SLOT(_loadColormap()));
    menu->addSeparator();
    menu->addAction("Save Transfer Function", this, SLOT(_saveTransferFunction()));
    menu->addAction("Load Transfer Function", this, SLOT(_loadTransferFunction()));
    menu->addSeparator();
    menu->addAction("Auto Update Histogram")->setCheckable(true);
    QAction *test = new QAction("TEST", this);
    test->setCheckable(true);
    menu->addAction(test);
    _tool->setMenu(menu);
    
    connect(range, SIGNAL(ValueChanged(float, float)), this, SLOT(_rangeChanged(float, float)));
    
//    this->setStyleSheet(R"(QWidget:hover:!pressed {border: 1px solid red;})");
    
}

void TFEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    _rParams = rParams;
    _paramsMgr = paramsMgr;
    colorMapTypeDropdown->Update(rParams->GetMapperFunc(rParams->GetVariableName())->GetColorMap());
    _maps->Update(dataMgr, paramsMgr, rParams);
    _mapsInfo->Update(rParams);
    _updateMappingRangeControl(dataMgr, paramsMgr, rParams);
}

QWidget *TFEditor::_tab() const
{
    return this->widget(0);
}

void TFEditor::_updateMappingRangeControl(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    float min, max;
    _getDataRange(dataMgr, rParams, &min, &max);
    range->SetRange(min, max);
    vector<double> mapperRange = rParams->GetMapperFunc(rParams->GetVariableName())->getMinMaxMapValue();
    range->SetValue(mapperRange[0], mapperRange[1]);
}

void TFEditor::_getDataRange(VAPoR::DataMgr *d, VAPoR::RenderParams *r, float *min, float *max) const
{
    std::vector<double> range;
    d->GetDataRange(r->GetCurrentTimestep(), r->GetVariableName(), r->GetRefinementLevel(), r->GetCompressionLevel(), d->GetDefaultMetaInfoStride(r->GetVariableName(), r->GetRefinementLevel()), range);
    *min = range[0];
    *max = range[1];
}

QString TFEditor::_createStylesheet() const
{
    std::string stylesheet;
    
#if defined(Darwin)
    stylesheet +=
    R"(
    QTabWidget::right-corner {
        top: 24px;
        right: 3px;
    }
    )";
#elif defined(WIN32)
#error style missing for windows
#else
    stylesheet +=
    R"(
    QTabWidget::right-corner {
        top: -3px;
        right: 5px;
    }
    )";
#endif
    
    return QString::fromStdString(stylesheet);
}

void TFEditor::_rangeChanged(float left, float right)
{
    _rParams->GetMapperFunc(_rParams->GetVariableName())->setMinMaxMapValue(left, right);
}

void TFEditor::_loadColormap()
{
    SettingsParams *sp = (SettingsParams*)_paramsMgr->GetParams(SettingsParams::GetClassType());
    
    QString qDefaultDirectory = QString::fromStdString(sp->GetTFDir());
    QString qSelectedPath = QFileDialog::getOpenFileName(nullptr, "Select a .tf3 file", qDefaultDirectory, "Vapor Transfer Function (*.tf3)");
    if (qSelectedPath.isNull())
        return;
    
    string selectedPath = qSelectedPath.toStdString();
    sp->SetTFDir(FileUtils::Dirname(selectedPath));
    
    MapperFunction *tf = _rParams->GetMapperFunc(_rParams->GetVariableName());
    
    int rc = tf->LoadColormapFromFile(selectedPath);
    
    if (rc < 0)
        MSG_ERR("Failed to load transfer function");
}

void TFEditor::_loadTransferFunction()
{
    SettingsParams *sp = (SettingsParams*)_paramsMgr->GetParams(SettingsParams::GetClassType());
    
    QString qDefaultDirectory = QString::fromStdString(sp->GetTFDir());
    QString qSelectedPath = QFileDialog::getOpenFileName(nullptr, "Select a .tf3 file", qDefaultDirectory, "Vapor Transfer Function (*.tf3)");
    if (qSelectedPath.isNull())
        return;
    
    string selectedPath = qSelectedPath.toStdString();
    sp->SetTFDir(FileUtils::Dirname(selectedPath));
    
    MapperFunction *tf = _rParams->GetMapperFunc(_rParams->GetVariableName());
    
    int rc = tf->LoadFromFile(selectedPath);
    
    if (rc < 0)
        MSG_ERR("Failed to load transfer function");
}

void TFEditor::_saveTransferFunction()
{
    SettingsParams *sp = (SettingsParams*)_paramsMgr->GetParams(SettingsParams::GetClassType());
    
    QString qDefaultDirectory = QString::fromStdString(sp->GetTFDir());
    QString qSelectedPath = QFileDialog::getSaveFileName(nullptr, "Select a .tf3 file", qDefaultDirectory, "Vapor Transfer Function (*.tf3)");
    if (qSelectedPath.isNull())
        return;
    
    string selectedPath = qSelectedPath.toStdString();
    sp->SetTFDir(FileUtils::Dirname(selectedPath));
    
    MapperFunction *tf = _rParams->GetMapperFunc(_rParams->GetVariableName());
    
    int rc = tf->SaveToFile(selectedPath);
    
    if (rc < 0)
        MSG_ERR("Failed to save transfer function");
}


#include <QStylePainter>
SettingsMenu::SettingsMenu()
{
    setIcon(QIcon(QString::fromStdString(GetSharePath("images/gear-dropdown1.png"))));
    setIconSize(QSize(18, 18));
    setCursor(QCursor(Qt::PointingHandCursor));
    setPopupMode(QToolButton::InstantPopup);
    
    setStyleSheet(
        "border: none;"
        "background-color: none;"
        "padding: 0px;"
    );
}

void SettingsMenu::paintEvent(QPaintEvent* event)
{
    // This function is overridden to prevent Qt from drawing its own dropdown arrow
    QStylePainter p(this);
    
    QStyleOptionToolButton option;
    initStyleOption(&option);
    option.subControls = QStyle::SC_ToolButton;
    option.features = QStyleOptionToolButton::None;
    p.drawComplexControl(QStyle::CC_ToolButton, option);
}




TFMapsGroup::TFMapsGroup()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);
    
    TFMapWidget *o;
    add(o= new TFOpacityWidget);
    o->AddMap(new TFColorMap(o));
    add(new TFHistogramWidget);
    add(new TFColorWidget);
}

void TFMapsGroup::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto map : _maps)
        map->Update(dataMgr, paramsMgr, rParams);
}

TFMapsInfoGroup *TFMapsGroup::CreateInfoGroup()
{
    TFMapsInfoGroup *infos = new TFMapsInfoGroup;
    
    for (auto map : _maps) {
        infos->add(map);
    }
    
    return infos;
}

void TFMapsGroup::add(TFMapWidget *map)
{
    _maps.push_back(map);
    layout()->addWidget(map);
    connect(map, SIGNAL(Activated(TFMapWidget*)), this, SLOT(mapActivated(TFMapWidget*)));
}

void TFMapsGroup::mapActivated(TFMapWidget *activatedMap)
{
    for (auto map : _maps)
        if (map != activatedMap)
            map->Deactivate();
}




TFMapsInfoGroup::TFMapsInfoGroup()
{
    
}

void TFMapsInfoGroup::Update(VAPoR::RenderParams *rParams)
{
    for (auto info : _infos)
        info->Update(rParams);
}

void TFMapsInfoGroup::add(TFMapWidget *map)
{
    TFInfoWidget *info = map->GetInfoWidget();
    _infos.push_back(info);
    addWidget(info);
    connect(map, SIGNAL(Activated(TFMapWidget*)), this, SLOT(mapActivated(TFMapWidget*)));
}

void TFMapsInfoGroup::mapActivated(TFMapWidget *activatedMap)
{
    setCurrentWidget(activatedMap->GetInfoWidget());
}
