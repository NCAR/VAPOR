//************************************************************************
//									*
//		     Copyright (C)  2006				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		EventRouter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		May 2006
//
//	Description:  Definition of EventRouter class
//		This (pure virtual) class manages communication between
//		gui elements, visualizers, and params.

#ifndef EVENTROUTER_H
#define EVENTROUTER_H
#include <cassert>
#include <QObject>
#include <QLineEdit>
#include <QSlider>
#include "vapor/ControlExecutive.h"
#include "GUIStateParams.h"

#ifdef WIN32
//Annoying unreferenced formal parameter warning
#pragma warning(disable : 4100)
#endif

class ColorbarSettings;
class MappingFrame;

//!
//! \class EventRouter
//! \ingroup Public_GUI
//! \brief A pure virtual class specifying the common properties of
//! all the parameter tabs in the VAPOR GUI
//! \author Alan Norton
//! \version 3.0
//! \date  May 2015

//!	The EventRouter class manages communication between
//! tabs in the GUI, visualizers, and params.
//! Implementers of new tabs in vaporgui must implement an EventRouter class
//! that translates user actions in
//! the tab to changes in the corresponding Params instance, and conversely,
//! populates the tab display based on the
//! most recent state of the params.
//!
//! EventRouter classes are currently of two types:  Those that are
//! associated with RenderParams and those that are not.
//! Those that are associated with RenderParams support having multiple
//! sub-tabs, and are presented in the RenderHolder
//! that displays the parameters of the currently selected renderer.
//! Other EventRouter classes are themselves tabs
//! and have their content defined by a Qt Designer .ui file.
//! Such EventRouter classes are derived from the _UI class that is based
//! on the .ui widget.
//!
//! Each widget in the tab is connected to slots in the EventRouter by
//! signals emitted by the widget when its state is changed.  The
//! connections must
//! be set up in the EventRouter::hookUpTab() method.
//!
//! In addition to implementing the various pure virtual methods on this
//! class, additional virtual methods
//! must be implemented to support changes in rendering, Transfer
//! Functions, etc., based on the functionality of the tab.
//!
//! Specifically, each EventRouter subclass implementor must provide
//! the following:
//!
//! Implement hookUpTab() to connect all signals/slots that are
//! permanent (not data dependent).  If some
//! widgets are created dynamically, the appropriate connections must
//! be established at that time.
//!
//! Implement _updateTab() to set the values of all gui elements based
//! on current Params settings, whenever the tab is exposed.
//!
//! Implement _confirmText() to read all the text values in the
//! tab and set them in the Params
//!
//! Implement a slot for each widget in the tab to respond to user changes,
//! except QLineEdits get two slots
//! (as described in EventRouter::hookUpTab()) )
//!
//!
//! Call confirmText() in the slots associated with each widget change,
//! so that text changes are updated prior to committing the widget change.
//!
//! If any data variables are associated with the tab (for RenderParams),
//! include the VariablesWidget as a tab inside this gui tab.
//! Note that the VariablesWidget handles its own signals and slots.
//!
//!
//! If there is a MappingFrame in the tab, invoke EventRouter::loadTF(),
//! EventRouter::fileSaveTF(),
//! EventRouter::loadInstalledTF(), EventRouter::saveTF() to load and
//! save the transfer functions.  Refer to MappingFrame documentation
//! for additional instructions.
//!
//! Various widgets associated with the TransferFunction editor are managed
//! by the MappingFrame as described under MappingFrame::hookup().
//! These must implement EventRouter::RefreshHistogram() to refresh
//! the histogram in the Transfer Function Editor.
//!
//! If there is a MouseMode (manipulator) associated with the tab,
//! then EventRouter::captureMouseUp() and
//! EventRouter::captureMouseDown() must be implemented.
//!
//
class EventRouter {

  public:
    EventRouter(
        VAPoR::ControlExec *ce, string paramsType);

    virtual ~EventRouter() {}

    //! Return the currently active params instance for this event
    //! router.
    //!
    virtual VAPoR::ParamsBase *GetActiveParams() const;

    //! Virtual method connects all the Qt signals and slots
    //! associated with the tab.
    //! Must also include connections that send signals when any QTextEdit box
    //! is changed and when enter is pressed.
    //! Each QLineEdit has a connection to a setTextChanged and enterPressed
    //! slot, to register when the value has changed,
    //! and when the user has pressed enter over the lineEdit.
    //! If there is a TransferFunction editor or IsoSelection panel in the
    //! tab, call MappingFrame::hookup() in this method
    //!
    virtual void hookUpTab() {}

    //! Pure virual method that returns documentation URLs
    //!
    //! \param[out] help A vector of string pairs. The first string is a
    //! descriptive text. The secone is a URL.
    //!
    virtual void GetWebHelp(
        std::vector<std::pair<string, string>> &help) const = 0;

    //! Update GUI display
    //!
    //! This method set the values of all the gui elements in the tab based on
    //! current Params state.
    //! This will do appropriate setup and then invoke the pure
    //! virtual method _updateTab().
    //! This is invoked whenever the tab is redisplayed and the values in the tab
    //! need to be refreshed.
    //!
    //! The method has three responsibilities:
    //! 1. Reconcile any anomalous conditions between the DataMgr and the
    //! Params instance managed by this event router. Thus if the Params
    //! instance contains values that would be viable with the current DataMgr
    //! (e.g. a variable name that is not present in the DataMgr) the method
    //! will attempt to change the Params instance to a valid state.
    //!
    //! 2. Refresh (rebuild) the GUI to reflect the state of the Params.
    //!
    //! 3. Ensure that the options presented by the GUI are viable for the
    //! current DataMgr.
    //!
    virtual void updateTab();

    //! Method to respond to changes in text in the tab.
    //! This method should be called whenever user presses enter, or changes
    //! the state of
    //! a widget (other than a textEdit) in the tab.
    //!
    virtual void confirmText();

    //! Set the TextChanged flag.  The flag should be turned on whenever any
    //! textbox (affecting the
    //! state of the Params) changes in the tab.  The change will not take
    //! effect until confirmText() is called.
    //! The flag will be turned off when confirmText() or updateTab() is called.
    //!
    //! \param[in] bool on : true indicates the flag is set.
    //!
    void SetTextChanged(bool on) {
#ifdef DEAD
        if (!_updatingTab)
            _textChangedFlag = on;
#endif
    }

    //! Method for classes that capture mouse event events from the visualizers
    //! (i.e. have manipulators)
    //! This must be reimplemented to respond when the mouse is released.
    //! The mouse release event is received by the VizWin instance, which
    //! then calls
    //! captureMouseUp() for the EventRouter that is associated with the
    //! current mouse mode.
    //! Ordinarily this method only needs to redisplay the layout and
    //! rerender.
    //
    virtual void captureMouseUp(){};

    //! Method for classes that capture mouse event events (i.e. have manipulators)
    //! This must be reimplemented to respond appropriately when the mouse
    //! is pressed.
    //! The mouse press event is received by the VizWin instance, which then calls
    //! captureMouseDown() for the EventRouter that is associated with the
    //! current mouse mode.
    //! This method should forget any previous text changes, since that would
    //! confuse the
    //! extents calculation.
    //!
    //! \param[in] mouseNum is 1,2, or 3 for left, middle, or right mouse.
    //!
    virtual void captureMouseDown(int mouseNum) {}

    //! Virtual method responds to cursor move in image window.
    //! Default just updates the window
    virtual void StartCursorMove();

    //! Virtual method responds to cursor move in image window.
    //! Default just updates the window
    virtual void EndCursorMove();

    GUIStateParams *GetStateParams() const {
        assert(_controlExec != NULL);
        return ((GUIStateParams *)
                    _controlExec->GetParamsMgr()
                        ->GetParams(GUIStateParams::GetClassType()));
    }

    size_t GetCurrentTimeStep() const {
        GUIStateParams *sP = GetStateParams();
        assert(sP);

        string vizName = sP->GetActiveVizName();
        VAPoR::ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
        return (paramsMgr->GetAnimationParams()->GetCurrentTimestep());
    }

    //! Return derived class type
    //!
    //! Return a string identifier for the derived EventRouter type
    //!
    virtual string GetType() const = 0;

  protected:
    EventRouter() {}

    //! Pure virtual method to set the values of all the gui elements in the
    //! tab based on current Params state.
    //! This is invoked whenever the tab is redisplayed and the values in the
    //! tab need to be refreshed.
    //! If there is a VariablesWidget, _updateTab() must invoke
    //! VariablesWidget::updateTab().
    //!
    //! \param[in] p Params instance associated with the current active tab.
    virtual void _updateTab() = 0;

    //! Pure virtual method to respond to changes in text in the tab.
    //! This method should be called whenever user presses enter, or changes
    //! the state of
    //! a widget (other than a textEdit) in the tab.
    //! In each implementation, the values of ALL QLineEdit's in the tab must
    //! be read and set in the
    //! corresponding Params instance.
    //!
    //! \param[in] p Params instance associated with the current active tab.
    virtual void _confirmText() = 0;

    VAPoR::ControlExec *_controlExec;
    bool _textChangedFlag;

  protected:
    string _paramsType;

  private:
};
#endif // EVENTROUTER_H
