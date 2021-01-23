#pragma once

#include "PWidget.h"
#include <set>

class TFMap;
class TFColorMap;
class TFOpacityMap;
class TFHistogramMap;
class TFIsoValueMap;
class TFMapWidget;
class TFColorWidget;
class TFOpacityWidget;
class TFHistogramWidget;
class TFIsoValueWidget;

//! \class PTFMapWidget
//! Wrapper class that allows TFMapsWidgets to be used as PWidgets
//! \copydoc TFMapsWidget

template<typename T> class PTFMapWidget : public PWidget {
    TFMapWidget *_tfWidget;

public:
    PTFMapWidget(const std::string &tag);

protected:
    virtual void updateGUI() const override;
    virtual bool requireParamsMgr() const override { return true; }
    virtual bool requireDataMgr() const override { return true; }
};

typedef PTFMapWidget<TFColorWidget>     PTFColorWidget;
typedef PTFMapWidget<TFOpacityWidget>   PTFOpacityWidget;
typedef PTFMapWidget<TFHistogramWidget> PTFHistogramWidget;
typedef PTFMapWidget<TFIsoValueWidget>  PTFIsoValueWidget;

class VSection;
class TFMappingRangeSelector;
class TFMapGroupWidget;
class TFMapInfoGroupWidget;
class PStringDisplay;

//! \class PTFEditor
//! PWidget wrapper for TFEditor
//! \copydoc TFEditor

class PTFEditor : public PWidget {
    Q_OBJECT;

    VSection *              _section;
    TFMapGroupWidget *      _maps;
    TFMapInfoGroupWidget *  _mapsInfo;
    TFHistogramMap *        _histogram;
    TFOpacityMap *          _opacityMap;
    TFColorMap *            _colorMap;
    TFIsoValueMap *         _isoMap;
    TFMappingRangeSelector *_range;

    std::vector<QAction *> _colorMapActions;
    std::vector<QAction *> _opacityMapActions;
    std::vector<QAction *> _histogramActions;

    bool        _showOpacityBasedOnParam = false;
    std::string _showOpacityBasedOnParamTag;
    int         _showOpacityBasedOnParamValue;
    bool        _showColormapBasedOnParam = false;
    std::string _showColormapBasedOnParamTag;
    bool        _showColormapBasedOnParamValue;

public:
    enum Element { Opacity, Histogram, Colormap, IsoValues, RegularIsoArray, Default };

    PTFEditor();
    PTFEditor(const std::string &tag, const std::set<Element> elements = {Default}, const std::string &label = "Transfer Function");
    //! Behaves the same as PWidget::ShowBasedOnParam except shows/hides the opacity controls.
    //! @copydoc PWidget::ShowBasedOnParam
    PTFEditor *ShowOpacityBasedOnParam(const std::string &tag, int value);
    //! Behaves the same as PWidget::ShowBasedOnParam except shows/hides the colormap controls.
    //! @copydoc PWidget::ShowBasedOnParam
    PTFEditor *ShowColormapBasedOnParam(const std::string &tag, int value);

protected:
    void updateGUI() const override;
};

class PColormapTFEditor : public PTFEditor {
public:
    PColormapTFEditor();
};
