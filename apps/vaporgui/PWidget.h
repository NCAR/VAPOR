#pragma once

#include <string>
#include <functional>
#include "UWidget.h"
#include <vapor/VAssert.h>

namespace VAPoR {
class ParamsBase;
class ParamsMgr;
class DataMgr;
}    // namespace VAPoR

class PDynamicMixin;
class SettingsParams;
template<class, typename> class PWidgetHLIBase;

//! \class PWidget
//! A Qt Widget that is automatically synced with the Params database.
//! The Update method must be called as per Vapor's convensions
//! All other public methods are self-explanitory. To see a demo and example
//! of how to use these widgets, see ParamsWidgetDemo

class PWidget : public UWidget {
    Q_OBJECT

    VAPoR::ParamsBase *_params = nullptr;
    VAPoR::ParamsMgr * _paramsMgr = nullptr;
    VAPoR::DataMgr *   _dataMgr = nullptr;
    const std::string  _tag;

    bool        _showBasedOnParam = false;
    std::string _showBasedOnParamTag = "";
    int         _showBasedOnParamValue;

    bool        _enableBasedOnParam = false;
    std::string _enableBasedOnParamTag = "";
    int         _enableBasedOnParamValue;

    bool _dynamicUpdateIsOn = false;
    bool _dynamicUpdateInsideGroup = false;

    bool                                             _usingHLI = false;
    std::function<void(void *, long)>                _setterLong;
    std::function<long(void *)>                      _getterLong;
    std::function<void(void *, double)>              _setterDouble;
    std::function<double(void *)>                    _getterDouble;
    std::function<void(void *, const std::string &)> _setterString;
    std::function<std::string(void *)>               _getterString;

public:
    PWidget(const std::string &tag, QWidget *widget);
    //! Follows the Vapor GUI update function convention. Update the element.
    void Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr = nullptr, VAPoR::DataMgr *dataMgr = nullptr) override;

    //! tag must be a key referencing a long value in the Params Database. If the associated value is equal
    //! to whenEqualTo, the current widget will be shown/enabled, and hidden/disabled otherwise.
    PWidget *ShowBasedOnParam(const std::string &tag, int whenEqualTo = true);
    //! @copydoc PWidget::ShowBasedOnParam()
    PWidget *EnableBasedOnParam(const std::string &tag, int whenEqualTo = true);
    //! Wrapping QWidget::setToolTip in case we want to add additional functionality such as automatic tool-tips
    //! without having to refactor.
    PWidget *SetTooltip(const std::string &text);
    void     setToolTip(const QString &) = delete;

protected:
    virtual void updateGUI() const = 0;
    virtual bool requireParamsMgr() const { return false; }
    virtual bool requireDataMgr() const { return false; }
    virtual bool isShown() const { return true; }

    const std::string &getTag() const;
    VAPoR::ParamsBase *getParams() const;
    VAPoR::ParamsMgr * getParamsMgr() const;
    VAPoR::DataMgr *   getDataMgr() const;
    SettingsParams *   getSettingsParams() const;

    void        setParamsDouble(double v);
    void        setParamsLong(long v);
    void        setParamsString(const std::string &v);
    double      getParamsDouble() const;
    long        getParamsLong() const;
    std::string getParamsString() const;

private:
    void dynamicUpdateBegin();
    void dynamicUpdateFinish();
    void _setParamsDouble(double v);
    void _setParamsLong(long v);
    void _setParamsString(const std::string &v);

    friend class PDynamicMixin;
    template<class, typename> friend class PWidgetHLIBase;

protected:
    template<class T> T *getParams() const
    {
        T *p = dynamic_cast<T *>(getParams());
        VAssert(p);
        return p;
    }
};
