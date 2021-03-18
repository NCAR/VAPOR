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
#include "Flags.h"

namespace VAPoR {
class ControlExec;
class RenderParams;
class ParamsMgr;
}    // namespace VAPoR

#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

class ColorbarWidget;

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
//! If there is a MouseMode (manipulator) associated with the tab,
//! then EventRouter::captureMouseUp() and
//! EventRouter::captureMouseDown() must be implemented.
//!
//
class RenderEventRouter : public EventRouter {
public:
    RenderEventRouter(VAPoR::ControlExec *ce, string paramsType) : EventRouter(ce, paramsType) { _instName = ""; }

    virtual ~RenderEventRouter() {}

    void SetActive(string instName) { _instName = instName; }

    virtual void hookUpTab() {}

    virtual void updateTab();

    virtual void confirmText() { EventRouter::confirmText(); }

    //! Pure virtual method that indicates whether the current RenderEventRouter
    //! and its associated renderer support 2D variables.
    virtual bool Supports2DVariables() const = 0;

    //! Pure virtual method that indicates whether the current RenderEventRouter
    //! and its associated renderer support 3D variables.
    virtual bool Supports3DVariables() const = 0;

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

#ifdef VAPOR3_0_0_ALPHA
    //! Method used to indicate that the mapping bounds have changed,
    //! in the transfer function editor, requiring update of the display.
    //! Must be reimplemented in every EventRouter which has a transfer function.
    //!
    //! \param[in] RenderParams* owner of the Transfer Function
    //
    virtual void UpdateMapBounds() {}
#endif

    //! Virtual method to fit the color map editor interval to the current map
    //! bounds.
    //! By default does nothing
    virtual void fitToView() {}

    //! Virtual method identifies the ColorbarWidget associated with an EventRouter.
    //! Must be implemented in every EventRouter with a MappingFrame
    //! \retval ColorbarWidget* is ColorbarWidget associated with the EventRouter
    virtual ColorbarWidget *getColorbarWidget() { return NULL; }

    //! Respond to a variable change
    //! Make any gui changes beyond updating the variable combos.
    //! Default does nothing
    virtual void variableChanged() {}

#ifdef VAPOR3_0_0_ALPHA
    //! Determine the value of the current variable at specified point.
    //! Return _OUT_OF_BOUNDS if not in current extents but in full domain
    //!
    //! \param[in] point[3] coordinates of point to evaluate.
    //! \return value at point, or _OUT_OF_BOUNDS
    virtual float CalcCurrentValue(const double point[3]);
#endif

    //! Return the currently active params instance for this event
    //! router.
    //!
    VAPoR::RenderParams *GetActiveParams() const;

    //! Return the currently active DataMgr instance for this event
    //! router.
    //!
    VAPoR::DataMgr *GetActiveDataMgr() const;

    //! Return a brief (3 or 4 sentence description of the renderer
    //!
    string GetDescription() const { return (_getDescription()); }

    //! Return the path name of a raster file containing an icon for
    //! the renderer
    //!
    string GetSmallIconImagePath() const;
    string GetIconImagePath() const;

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

    virtual string _getDescription() const = 0;

    virtual string _getSmallIconImagePath() const = 0;

    virtual string _getIconImagePath() const = 0;

private:
    string _instName;
};


//////////////////////////////////////////////////////////////////////////
//
// RenderEventRouterFactory Class
//
/////////////////////////////////////////////////////////////////////////

class RenderEventRouterFactory {
public:
    static RenderEventRouterFactory *Instance()
    {
        static RenderEventRouterFactory instance;
        return &instance;
    }

    void RegisterFactoryFunction(string name, function<RenderEventRouter *(QWidget *, VAPoR::ControlExec *)> classFactoryFunction)
    {
        // register the class factory function
        _factoryFunctionRegistry[name] = classFactoryFunction;
    }

    RenderEventRouter *(CreateInstance(string classType, QWidget *, VAPoR::ControlExec *));

    vector<string> GetFactoryNames() const;

private:
    map<string, function<RenderEventRouter *(QWidget *, VAPoR::ControlExec *)>> _factoryFunctionRegistry;

    RenderEventRouterFactory() {}
    RenderEventRouterFactory(const RenderEventRouterFactory &) {}
    RenderEventRouterFactory &operator=(const RenderEventRouterFactory &) { return *this; }
};

//////////////////////////////////////////////////////////////////////////
//
// RenderEventRouterRegistrar Class
//
// Register RenderEventRouter derived class with:
//
//	static RenderEventRouterRegistrar<RERClass> registrar("myclassname");
//
// where 'RERClass' is a class derived from 'RenderEventRouter', and
// "myclassname" is the name of the class
//
/////////////////////////////////////////////////////////////////////////

template<class T> class RenderEventRouterRegistrar {
public:
    RenderEventRouterRegistrar(string classType)
    {
        // register the class factory function
        //
        RenderEventRouterFactory::Instance()->RegisterFactoryFunction(classType, [](QWidget *parent, VAPoR::ControlExec *ce) -> RenderEventRouter * { return new T(parent, ce); });
    }
};

#endif    // RENDEREREVENTROUTER_H
