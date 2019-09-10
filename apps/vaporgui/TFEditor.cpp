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

#include <vapor/GLManager.h>

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
    layout->addWidget(colorMapTypeDropdown = new ParamsWidgetDropdown(
        VAPoR::ColorMap::_interpTypeTag,
        {"Linear", "Discrete", "Diverging"},
        {TFInterpolator::linear, TFInterpolator::discrete, TFInterpolator::diverging},
        "Color Interpolation")
    );
    
    
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
    
    ParamsBase::StateSave stateSave;
    MapperFunction mf(&stateSave);
    
    QMenu *builtin = new QMenu("Load Builtin Colormap");
    string builtinPath = GetSharePath("palettes");
    auto fileNames = FileUtils::ListFiles(builtinPath);
    for (int i = 0; i < fileNames.size(); i++) {
        
        string path = FileUtils::JoinPaths({builtinPath, fileNames[i]});
        mf.LoadColormapFromFile(path);
        ColorMap *cm = mf.GetColorMap();
        
        int nSamples = 64;
        unsigned char buf[nSamples*3];
        float rgb[3];
        for (int i = 0; i < nSamples; i++) {
            cm->colorNormalized(i/(float)nSamples).toRGB(rgb);
            buf[i*3+0] = rgb[0]*255;
            buf[i*3+1] = rgb[1]*255;
            buf[i*3+2] = rgb[2]*255;
        }
        QImage image(buf, nSamples, 1, QImage::Format::Format_RGB888);
        
        
        QAction *item = new ColorMapMenuItem(QPixmap::fromImage(image).scaled(nSamples, 20));
        connect(item, SIGNAL(triggered()), this, SLOT(_test()));
        builtin->addAction(item);
//        builtin->addAction(QPixmap::fromImage(image).scaled(nSamples, 20), "test");
    }
    
    builtin->setStyleSheet(R"(
                       
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

                           
//icon-size: 100% 100%;
                           
                           )");
    
    menu->addMenu(builtin);
    
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

void TFEditor::_test()
{
    printf("TEST\n");
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



ColorMapMenuItem::ColorMapMenuItem(const QIcon &icon)
: QWidgetAction(nullptr)
{
    QPushButton *button = new QPushButton;
    button->setIcon(icon);
    button->setFixedSize(QSize(60, 25));
    connect(button, SIGNAL(clicked()), this, SLOT(trigger()));
    connect(button, SIGNAL(clicked()), this, SLOT(_pressed()));
    setDefaultWidget(button);
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

void ColorMapMenuItem::_pressed()
{
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
