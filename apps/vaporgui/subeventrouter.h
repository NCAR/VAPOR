#ifndef SUBEVENTROUTER_H
#define SUBEVENTROUTER_H

#include <qobject.h>
#include "vapor/MyBase.h"
#include "qwidget.h"
#include "EventRouter.h"

QT_USE_NAMESPACE

namespace VAPoR {
class ControlExec;
class ParamsBase;
} // namespace VAPoR

//!
//! \class SubEventRouter
//! \ingroup Public_GUI
//! \brief A pure virtual class for sub-tabs of renderer event routers
//! \author Alan Norton
//! \version 3.0
//! \date  January 2016

class SubEventRouter : public QWidget {

  public:
    SubEventRouter(
        QWidget *parent, EventRouter *er, const VAPoR::ControlExec *ce) : QWidget(parent) {

        _eventRouter = er;
        _controlExec = ce;
        _textChangedFlag = false;
        m_activeParams = NULL;
    }

    SubEventRouter(QWidget *parent) : QWidget(parent) {
        _eventRouter = NULL;
        _controlExec = NULL;
        _textChangedFlag = false;
        m_activeParams = NULL;
    }

    void initialize(EventRouter *er, const VAPoR::ControlExec *ce) {
        _eventRouter = er;
        _controlExec = ce;
    }

    virtual ~SubEventRouter() {}

    //! Update all the gui elements in the Tab, based on a RenderParams instance.
    //! Called from EventRouter::updateTab()
    //! \param[in] rp RenderParams instance with values to be presented.
    virtual void updateTab() {
        _updateTab();
    }

    //! Respond to user pressing enter after changing text box.
    //! Must implement if there are textboxes.
    virtual void confirmText() {}

    void SetTextChanged(bool val) {
        _textChangedFlag = val;
    }

    VAPoR::ParamsBase *GetActiveParams() const {
        if (!_eventRouter)
            assert(!"bad eventRouter in subEventRouter");
        return (_eventRouter->GetActiveParams());
    }

    EventRouter *GetEventRouter() { return _eventRouter; }
    const VAPoR::ControlExec *GetControlExec() { return _controlExec; }

  protected:
    SubEventRouter() {}

    virtual void _updateTab() = 0;

    bool _textChangedFlag;
    EventRouter *_eventRouter;
    const VAPoR::ControlExec *_controlExec;

  private:
    VAPoR::ParamsBase *m_activeParams;

    // Make these private after reimplementing geometryWidget!
    //EventRouter* _eventRouter;
    //const ControlExec * _controlExec;
};

#endif //SUBEVENTROUTER_H
