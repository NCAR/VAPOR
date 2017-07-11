//************************************************************************
//															*
//			 Copyright (C)  2016										*
//	 University Corporation for Atmospheric Research					*
//			 All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		ColorbarWidget.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016
//
//	Description:	Defines the ColorbarWidget class.  This provides
//		a frame that controls colorbar settings
//
#ifndef COLORBARWIDGET_H
#define COLORBARWIDGET_H
#include <QFrame>
#include <qwidget.h>
#include <vector>
#include <vapor/RenderParams.h>
#include "RenderEventRouter.h"
#include "ColorbarWidgetGUI.h"

namespace VAPoR {
class ColorbarPbase;
class ParamsBase;
}    // namespace VAPoR

//! \class ColorbarWidget
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

class ColorbarWidget : public QFrame, public Ui_ColorbarWidgetGUI {
    Q_OBJECT

public:
    ColorbarWidget(QWidget *parent = 0);
    ~ColorbarWidget();

    void SetEventRouter(RenderEventRouter *er) { _eventRouter = er; }

    //! Update the values displayed in this frame, by obtaining them from the
    //! ColorbarPbase instance in the Params.
    //! \param[in] p Params associated with this frame.
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);

signals:

protected slots:
    void colorbarTextChanged(const QString &);
    void enabledChange();
    void setBackgroundColor();
    void applyToAll();
    void colorbarReturnPressed();

private:
    RenderEventRouter *_eventRouter;
    void               disableSignals(bool disabled);

    bool                 _textChangedFlag;
    VAPoR::ParamsMgr *   _paramsMgr;
    VAPoR::DataMgr *     _dataMgr;
    VAPoR::RenderParams *_rParams;

    VAPoR::ColorbarPbase *GetActiveParams() const { return (_rParams->GetColorbarPbase()); }
};

#endif    // COLORBARSETTINGS_H
