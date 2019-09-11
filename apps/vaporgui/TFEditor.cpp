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
#include <algorithm>
TFEditor::TFEditor()
: VSection("Transfer Function")
{
    layout()->addWidget(_maps = new TFMapsGroup);
    layout()->addWidget(_mapsInfo = _maps->CreateInfoGroup());
    layout()->addWidget(range = new QRangeSliderTextCombo);
    layout()->addWidget(colorMapTypeDropdown = new ParamsWidgetDropdown(
        VAPoR::ColorMap::_interpTypeTag,
        {"Linear", "Discrete", "Diverging"},
        {TFInterpolator::linear, TFInterpolator::discrete, TFInterpolator::diverging},
        "Color Interpolation")
    );
    
    QMenu *builtinColormapMenu = new QMenu("Load Built-In Colormap");
    string builtinPath = GetSharePath("palettes");
    auto fileNames = FileUtils::ListFiles(builtinPath);
    std::sort(fileNames.begin(), fileNames.end());
    for (int i = 0; i < fileNames.size(); i++) {
        
        string path = FileUtils::JoinPaths({builtinPath, fileNames[i]});
        
        QAction *item = new ColorMapMenuItem(path);
        connect(item, SIGNAL(triggered(std::string)), this, SLOT(_loadColormap(std::string)));
        builtinColormapMenu->addAction(item);
    }
    
    QMenu *menu = new QMenu;
    menu->addAction("Save Colormap", this, SLOT(_saveTransferFunction()));
    menu->addAction("Load Colormap", this, SLOT(_loadColormap()));
    menu->addMenu(builtinColormapMenu);
    menu->addSeparator();
    menu->addAction("Save Transfer Function", this, SLOT(_saveTransferFunction()));
    menu->addAction("Load Transfer Function", this, SLOT(_loadTransferFunction()));
    menu->addSeparator();
    menu->addAction("Auto Update Histogram")->setCheckable(true);
    QAction *test = new QAction("TEST", this);
    test->setCheckable(true);
    menu->addAction(test);
    setMenu(menu);
    
    
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

void TFEditor::_rangeChanged(float left, float right)
{
    _rParams->GetMapperFunc(_rParams->GetVariableName())->setMinMaxMapValue(left, right);
}

void TFEditor::_test()
{
    printf("TEST\n");
}

void TFEditor::_loadColormap(std::string path)
{
    printf("%s(\"%s\")\n", __func__, path.c_str());
    if (!_rParams)
        return;
    
    MapperFunction *tf = _rParams->GetMapperFunc(_rParams->GetVariableName());
    int rc = tf->LoadColormapFromFile(path);
    
    if (rc < 0)
        MSG_ERR("Failed to load transfer function");
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
    
    _loadColormap(selectedPath);
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











std::map<std::string, QIcon> ColorMapMenuItem::icons;

QIcon ColorMapMenuItem::getCachedIcon(const std::string &path)
{
    auto it = icons.find(path);
    if (it != icons.end())
        return it->second;
    
    ParamsBase::StateSave stateSave;
    MapperFunction mf(&stateSave);
    
    mf.LoadColormapFromFile(path);
    ColorMap *cm = mf.GetColorMap();
    
    QSize size = getIconSize();
    int nSamples = size.width();
    unsigned char buf[nSamples*3];
    float rgb[3];
    for (int i = 0; i < nSamples; i++) {
        cm->colorNormalized(i/(float)nSamples).toRGB(rgb);
        buf[i*3+0] = rgb[0]*255;
        buf[i*3+1] = rgb[1]*255;
        buf[i*3+2] = rgb[2]*255;
    }
    QImage image(buf, nSamples, 1, QImage::Format::Format_RGB888);
    
    icons[path] = QIcon(QPixmap::fromImage(image).scaled(size.width(), size.height()));
    return icons[path];
}

QSize ColorMapMenuItem::getIconSize()
{
    return QSize(50, 15);
}

QSize ColorMapMenuItem::getIconPadding()
{
    return QSize(10, 10);
}

#include <vapor/STLUtils.h>

ColorMapMenuItem::ColorMapMenuItem(const std::string &path)
: QWidgetAction(nullptr), _path(path)
{
    QPushButton *button = new QPushButton;
    setDefaultWidget(button);
    
    button->setIcon(getCachedIcon(path));
    button->setFixedSize(getIconSize() + getIconPadding());
    connect(button, SIGNAL(clicked()), this, SLOT(_clicked()));
    
    string name = STLUtils::Split(FileUtils::Basename(path), ".")[0];
    button->setToolTip(QString::fromStdString(name));
    
    button->setStyleSheet(R"(
                           QPushButton {
                                icon-size: 50px 15px;
                                padding: 0px;
                                margin: 0px;
                                background: none;
                                border: none;
                           }
                           QPushButton::hover {
                                background: #aaa;
                           }
                           )");
}

void ColorMapMenuItem::CloseMenu(QAction *action)
{
    if (!action)
        return;
    
    QList<QWidget *> menus = action->associatedWidgets();
    
    for (QWidget *widget : menus) {
        QMenu *menu = dynamic_cast<QMenu *>(widget);
        if (!menu) continue;
        if (menu->isHidden()) continue;
        
        menu->hide();
        CloseMenu(menu->menuAction());
    }
}

void ColorMapMenuItem::_clicked()
{
    trigger();
    emit triggered(_path);
    CloseMenu(this);
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

void TFMapsInfoGroup::add(TFMapWidget *mapWidget)
{
    for (TFMap *map : mapWidget->GetMaps()) {
        TFInfoWidget *info = map->GetInfoWidget();
        _infos.push_back(info);
        addWidget(info);
        connect(map, SIGNAL(Activated(TFMap*)), this, SLOT(mapActivated(TFMap*)));
    }
}

void TFMapsInfoGroup::mapActivated(TFMap *activatedMap)
{
    setCurrentWidget(activatedMap->GetInfoWidget());
}
