//************************************************************************
//															*
//			 Copyright (C)  2016										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ColorbarSettings.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Defines the ColorbarSettings class.  This provides
//		a frame that controls colorbar settings
//
#ifndef COLORBARSETTINGS_H
#define COLORBARSETTINGS_H
#include <QFrame>
#include <qwidget.h>
#include <vector>
#include <vapor/RenderParams.h>
#include "RenderEventRouter.h"
#include "colorbarframe.h"

namespace VAPoR {
class ColorbarPbase;
class ParamsBase;
} // namespace VAPoR

//! \class ColorbarSettings
//! \ingroup Public_GUI
//! \brief A QFrame that contains text boxes controlling the colorbar settings
//! \author Alan Norton
//! \version 3.0
//! \date	February 2016

//!	This QFrame should be embedded in various EventRouter .ui files to support
//! controlling box extents. EventRouter implementors must do the following:
//!
//! If a slider or text box should not be displayed, invoke hide() on that gui
//! element in the EventRouter constructor.
//!

class ColorbarSettings : public QFrame, public Ui_ColorbarFrame {
    Q_OBJECT

  public:
    ColorbarSettings(QWidget *parent = 0);
    ~ColorbarSettings();

    void SetEventRouter(RenderEventRouter *er) {
        _eventRouter = er;
    }

    //! Update the values displayed in this frame, by obtaining them from the
    //! ColorbarPbase instance in the Params.
    //! \param[in] p Params associated with this frame.
    void updateTab();
    void Update(VAPoR::ParamsMgr *paramsMgr,
                VAPoR::DataMgr *dataMgr,
                VAPoR::RenderParams *rParams);

  signals:
#ifndef DOXYGEN_SKIP_THIS
  protected slots:
    void colorbarTextChanged(const QString &);
    void colorbarReturnPressed();
    void enabledChange();
    void showSettings(bool onoff);
    void setBackgroundColor();
    void applyToAll();

  private:
    RenderEventRouter *_eventRouter;
    void confirmText();
    void updateGUI();
    void disableSignals(bool disabled);

    bool _textChangedFlag;
    bool _showSettings;
    VAPoR::ParamsMgr *_paramsMgr;
    VAPoR::DataMgr *_dataMgr;
    VAPoR::RenderParams *_rParams;

    VAPoR::ColorbarPbase *GetActiveParams() const {
        return (_rParams->GetColorbarPbase());
    }

#endif //DOXYGEN_SKIP_THIS
};

#endif //COLORBARSETTINGS_H
