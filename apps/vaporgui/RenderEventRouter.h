//************************************************************************
//									*
//		     Copyright (C)  2017				*
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
//	Description:  Definition of RenderEventRouter class
//		This (pure virtual) class manages communication between
//		renderer gui elements, visualizers, and params.

#ifndef RENDEREREVENTROUTER_H
#define RENDEREREVENTROUTER_H

#include "EventRouter.h"
#include "Histo.h"
class MappingFrame;

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
}    // namespace VAPoR

#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

class Histo;
class ColorbarSettings;

//!
//! \class RenderEventRouter
//! \ingroup Public_GUI
//! \brief A pure virtual class specifying the common properties of
//! all the renderer parameter tabs in the VAPOR GUI
//! \author Alan Norton
//! \version 3.0
//! \date  January 2017

//!	The RendererEventRouter class manages communication between
//! tabs in the GUI, visualizers, and params.
//! Implementers of new renderer tabs in vaporgui must implement
//! an RenderEventRouter class
//! that translates user actions in
//! the tab to changes in the corresponding Params instance, and conversely,
//! populates the tab display based on the
//! most recent state of the params.
//!
//! RenderEventRouter are associated with RenderParams.
//! RenderParams support having multiple
//! sub-tabs, and are presented in the RenderHolder
//! that displays the parameters of the currently selected renderer.
//! Other EventRouter classes are themselves tabs
//! and have their content defined by a Qt Designer .ui file.
//! Such EventRouter classes are derived from the _UI class that is based
//! on the .ui widget.
//!
//! Each widget in the tab is connected to slots in the RenderEventRouter by
//! signals emitted by the widget when its state is changed.  The
//! connections must
//! be set up in the RenderEventRouter::hookUpTab() method.
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
//! Call confirmText() in the slots associated with each widget change,
//! so that text changes are updated prior to committing the widget change.
//!
//! If any data variables are associated with the tab (for RenderParams),
//! include the VariablesWidget as a tab inside this gui tab.
//! Note that the VariablesWidget handles its own signals and slots.
//!
//! Provide text descriptions and URLs for Web-based help as described
//! under EventRouter::makeWebHelpActions()
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
class RenderEventRouter : public EventRouter {
public:
    RenderEventRouter(VAPoR::ControlExec *ce, string paramsType) : EventRouter(ce, paramsType)
    {
        m_currentHistogram = NULL;
        m_winName = "";
        m_instName = "";
    }

    virtual ~RenderEventRouter()
    {
        if (m_currentHistogram) delete m_currentHistogram;
    }

    void SetActive(string winName, string instName)
    {
        m_winName = winName;
        m_instName = instName;
    }

    virtual void hookUpTab() {}

    virtual void updateTab();

    virtual void confirmText() { EventRouter::confirmText(); }

    //! Virtual method to enable or disable rendering when turned on or off by
    //! a gui tab.
    //! Only useful if the tab corresponds to a renderer.
    //!
    //! \param[in] rp RenderParams instance to be enabled/disabled
    //! \param[in] wasEnabled indicates if rendering was previously disabled
    //! \param[in] instance index being enabled
    //! \param[in] newVis indicates if the rendering is being enabled in a
    //! new visualizer.
    //
    void updateRenderer(VAPoR::RenderParams *rParams, bool prevEnabled, string VizName, string renderInstName, bool newWindow);

    //! Virtual method supports loading a transfer function from the session, for
    //! tabs that have transfer functions.
    //! param[in] name indicates the name identifying the transfer function
    //
    virtual void sessionLoadTF(string name) {}

    //! Method to indicate that a transfer function has changed
    //! so the tab display must be refreshed.  Used in all tabs
    //! with RenderParams and that have a transfer function.
    //! If ParamsBase * argument is null, uses default params.
    //! Must be reimplemented if there is more than one MappingFrame in the tab.
    //!
    //! \param[in] RenderParams* is the Params that owns the Transfer Function
    //
    virtual void setEditorDirty();

    //! Method used to indicate that the mapping bounds have changed,
    //! in the transfer function editor, requiring update of the display.
    //! Must be reimplemented in every EventRouter which has a transfer function.
    //!
    //! \param[in] RenderParams* owner of the Transfer Function
    //
    virtual void UpdateMapBounds() {}

    //! Launch a dialog to save the current transfer function to file.
    //!
    //! \param[in] rParams RenderParams instance associated with the
    //! transfer function
    //
    void fileSaveTF();

    //! Launch a dialog to enable user to load an installed transfer function.
    //!
    //! \param[in] varname name of the variable associated with the TF.
    void loadInstalledTF(string varname);

    //! Launch a dialog to enable user to load a transfer function from
    //! session or file.
    //!
    //! \param[in] varname name of the variable associated with the
    //! TF.
    void loadTF(string varname);

    //! Launch a dialog to enable user to load a transfer function from file.
    //!
    //! \param[in] varname name of the variable associated with the TF.
    //! \param[in] startPath file path for the dialog to initially present to user
    //! \param[in] savePath indicates whether or not the resulting path should
    //! be saved to user preferences.
    void fileLoadTF(string varname, const char *startPath, bool savePath);

    //! Obtain the current valid histogram.  Optionally will construct a new
    //! one if needed.
    //!
    //! \param[in] mustGet : Boolean argument indicating that a new histogram
    //! is required.
    //! \param[in] isIsoWin : Boolean argument indicating this is associated
    //! with an IsoSelection panel
    //! \retval Histo* is resulting Histo instance.
    //
    virtual Histo *GetHistogram(bool mustGet, bool isIsoWin = false);

    //! Virtual method to refresh the histogram for the associated EventRouter.
    //! Must be reimplemented in every EventRouter class with a Histogram.
    //!
    virtual void RefreshHistogram();

    //! Virtual method to fit the color map editor interval to the current map
    //! bounds.
    //! By default does nothing
    virtual void fitToView() {}

    //! Helper method, calculate a histogram of a slice of 3d variables,
    //! such as Probe or IsoLines
    //!
    //! \param[in] ts time step for the histogram
    //! \param[out] histo resulting Histo instance.
    void CalcSliceHistogram(int ts, Histo *histo);

    //! Virtual method identifies the MappingFrame associated with an EventRouter.
    //! Must be implemented in every EventRouter with a MappingFrame
    //! \retval MappingFrame* is MappingFrame associated with the EventRouter
    virtual MappingFrame *getMappingFrame() { return NULL; }

    //! Virtual method identifies the ColorbarFrame associated with an EventRouter.
    //! Must be implemented in every EventRouter with a MappingFrame
    //! \retval ColorbarFrame* is ColorbarFrame associated with the EventRouter
    virtual ColorbarSettings *getColorbarFrame() { return NULL; }

    //! Respond to a variable change
    //! Make any gui changes beyond updating the variable combos.
    //! Default does nothing
    virtual void variableChanged() {}

    //! Determine the value of the current variable at specified point.
    //! Return _OUT_OF_BOUNDS if not in current extents but in full domain
    //!
    //! \param[in] point[3] coordinates of point to evaluate.
    //! \return value at point, or _OUT_OF_BOUNDS
    virtual float CalcCurrentValue(const double point[3]);

    //! Return the currently active params instance for this event
    //! router.
    //!
    VAPoR::RenderParams *GetActiveParams() const;

    //! Return the currently active params instance for this event
    //! router.
    //!
    VAPoR::DataMgr *GetActiveDataMgr() const;

#ifdef DEAD
    //! For parameters that multiple instances set the current
    //! instance name.
    //
    void SetActiveInstName(string instName) { m_activeInstName = instName; }
#endif

protected:
    RenderEventRouter() {}

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
    virtual void _confirmText(){};

    Histo *m_currentHistogram;

private:
    string m_winName;
    string m_instName;
};
#endif    // RENDEREREVENTROUTER_H
