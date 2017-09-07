
//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//	File:		Renderer.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September 2004
//
//	Description:	Defines the Renderer class.
//		A pure virtual class that is implemented for each renderer.
//		Methods are called by the visualizer class as needed.
//

#ifndef RENDERER_H
#define RENDERER_H

#include <vapor/MyBase.h>
#include <vapor/ParamsMgr.h>
#include <vapor/RenderParams.h>
#include <vapor/textRenderer.h>

namespace VAPoR {

class ShaderMgr;

//! \class RendererBase
//! \ingroup Public_Render
//! \brief A base class for Renderer classes
//! \author Alan Norton
//! \version 3.0
//! \date July 2015
//!
//! RendererBase is a base class of Renderer, for special rendering
//! tasks that are not associated with RenderParams instances.
//! For example the VizFeatureRenderer has a special role during
//! Visualizer OpenGL rendering. RendererBase instances perform rendering
//! in a VAPOR Visualizer but are not associated with RenderParams instances.
//!
class RENDER_API RendererBase : public MyBase {
public:
    RendererBase(const ParamsMgr *pm, string winName, string dataSetName, string paramsType, string classType, string instName, DataMgr *dataMgr);
    virtual ~RendererBase();
    //! Pure virtual method
    //! Any OpenGL initialization is performed in initializeGL
    //! It will be called from an OpenGL rendering context.
    //! Sets _initialized to true if successful.
    virtual int initializeGL(ShaderMgr *sm);

    //! Obtain the Visualizer associated with this Renderer
    string GetVisualizer() { return _winName; }

    //! Identify the name of the current renderer
    //! \return name of renderer
    string GetMyName() const { return (_instName); };

    //! Identify the type of the current renderer
    //! \return type of renderer
    string GetMyType() const { return (_classType); };

    //! Identify the type of the current renderer
    //! \return type of renderer
    string GetMyParamsType() const { return (_paramsType); };

    //! Return boolean indicating whether initializeGL has been
    //! successfully called
    //!
    bool IsGLInitialized() const { return (_glInitialized); }

protected:
    const ParamsMgr *_paramsMgr;
    string           _winName;
    string           _dataSetName;
    string           _paramsType;
    string           _classType;
    string           _instName;
    DataMgr *        _dataMgr;

    ShaderMgr *_shaderMgr;

    //! Pure virtual method
    //! Any OpenGL initialization is performed in initializeGL
    //! It will be called from an OpenGL rendering context.
    virtual int _initializeGL() = 0;

protected:
    RendererBase() {}

private:
    bool _glInitialized;
};

//! \class Renderer
//! \ingroup Public_Render
//! \brief A class that performs rendering in a Visualizer
//! \author Alan Norton
//! \version 3.0
//! \date July 2015
//!
//! Renderer class is a pure virtual class that supports
//! OpenGL rendering in the VAPOR visualizer window, using a RenderParams instance to describe the rendering.
//! All Renderer classes must derive from this class.
class RENDER_API Renderer : public RendererBase {
public:
    //! Constructor should be invoked by any derived renderers.
    //! It is invoked when the user enables a renderer.
    //! Provides any needed setup of renderer state, but not of OpenGL state.
    //
    Renderer(const ParamsMgr *pm, string winName, string dataSetName, string paramsType, string classType, string instName, DataMgr *dataMgr);

    virtual ~Renderer();

    //! All OpenGL rendering is performed in the paintGL method.
    //! This invokes _paintGL on the renderer subclass
    //! \param[in] dataMgr Current (valid) dataMgr
    //! \retval int zero if successful.
    virtual int paintGL();

#ifdef DEAD
#endif

#ifdef DEAD
    //! Call setBypass to indicate that the renderer will not work until the state of the params is changed
    //! This will result in the renderer not being invoked for the specified timestep
    //! \param[in] int timestep The timestep when the renderer fails
    void setBypass(int timestep)
    {
        if (_currentRenderParams) _currentRenderParams->setBypass(timestep);
    }

    //! Partial bypass is currently only used by DVR and Isosurface renderers.
    //! Indicates a renderer that should be bypassed at
    //! full resolution but not necessarily at interactive resolution.
    //! \param[in] timestep Time step that should be bypassed
    void setPartialBypass(int timestep)
    {
        if (_currentRenderParams) _currentRenderParams->setPartialBypass(timestep);
    }

    //! SetAllBypass is set to indicate all (or no) timesteps should be bypassed
    //! Should be set true when render failure is independent of timestep
    //! Should be set false when state changes and rendering should be reattempted.
    //! \param[in] val indicates whether it is being turned on or off.
    void setAllBypass(bool val)
    {
        if (_currentRenderParams) _currentRenderParams->setAllBypass(val);
    }

    //! doBypass indicates the state of the bypass flag.
    //! \param[in] timestep indicates the timestep being checked
    //! \retval bool indicates that the rendering at the timestep should be bypassed.
    bool doBypass(int timestep) { return (_currentRenderParams && _currentRenderParams->doBypass(timestep)); }

    //! doAlwaysBypass is used in the presence of partial bypass.
    //! Indicates that the rendering should be bypassed at any resolution
    //! \param[in] int timestep Time step
    //! \retval bool value of flag
    bool doAlwaysBypass(int timestep) { return (_currentRenderParams && _currentRenderParams->doAlwaysBypass(timestep)); }

    //! General method to force a renderer to re-obtain its data.
    //! Default does nothing
    //! Must be re-implemented if
    //! there is state or a cache that is retained between renderings.
    virtual void setAllDataDirty() { return; }
#endif

#ifdef DEAD
    //! Set the current ControlExec
    //! \param[in] ds Current DataStatus instance
    static void SetControlExec(ControlExec *ce) { _controlExec = ce; }

    //! @name Internal
    //! Internal methods not intended for general use
    //!
    ///@{

    //! Static method invoked during Undo and Redo of Renderer enable or disable.
    //! This function must be passed in Command::CaptureStart for enable and disable rendering.
    //! It causes the Undo and Redo operations on enable/disable renderer to actually create
    //! or destroy the renderer.
    //! \sa UndoRedoHelpCB_T
    //! \param[in] isUndo indicates whether an Undo or Redo is being performed
    //! \param[in] instance indicates the RenderParams instance that is enabled or disabled
    //! \param[in] beforeP is a copy of the InstanceParams at the start of the Command
    //! \param[in] afterP is a copy of the InstanceParams at the end of the Command
    //! \param[in] auxPrev is a copy of auxiliary InstanceParams at the start of the Command, not currently used
    //! \param[in] afterP is a copy of auxiliary InstanceParams at the end of the Command, not currently used
    static void UndoRedo(bool isUndo, int instance, Params *beforeP, Params *afterP, Params *auxPrev = 0, Params *auxNext = 0);

#endif

#ifdef DEAD
    //! Construct transform of form (x,y)-> (a[0]x+b[0],a[1]y+b[1],const)
    //! Mapping [-1,1]X[-1,1] into local 3D volume coordinates.
    //! This is used to map plane coordinates (for various 2D renderers) into User coordinates
    //! \param[in] orientation is 0 1 or 2 for z, y, or x-axis orientation.
    //! \param[out] a is linear term of mapping
    //! \param[out] b is constant term of mapping
    //! \param[out] constVal is the coordinate along the axis orthogonal to the image of the mapping
    //! \param[out] mappedDims gives the three axes of the mapped image
    void buildLocal2DTransform(int dataOrientation, float a[2], float b[2], float *constVal, int mappedDims[3]);
#endif

#ifdef DEAD

    //! Obtain the extents of a region that contains a rotated (3D) box associated with a renderer.
    //! \param[out] regMin Minimum coordinates of containing region.
    //! \param[out] regMix Maximum coordinates of containing region.
    void getLocalContainingRegion(float regMin[3], float regMax[3]);
#endif

    //! Render the colorbar for this renderer (if it has one)
    void renderColorbar();

    //! Render colorbar text
    void renderColorbarText(ColorbarPbase *cbpb, float fbWidth, float fbHeight, float llx, float lly, float urx, float ury);

    ///@}

    //! Obtain the current RenderParams instance
    //! \retval RenderParams* current render params
    RenderParams *GetActiveParams() const { return (_paramsMgr->GetRenderParams(_winName, _dataSetName, _paramsType, _instName)); }

protected:
    Renderer() {}

    //! Obtain current texture for the renderer colorbar.  This is an array of 3x256x256 bytes.
    //! \return 0 if successful
    int makeColorbarTexture();

    //! All OpenGL rendering is performed in the pure virtual paintGL method.
    virtual int _paintGL() = 0;

    //! Enable specified clipping planes during the GL rendering
    //! Must be invoked during _paintGL()
    //! Clipping planes are specified in User coordinates.
    //! \sa disableClippingPlanes
    //! \param[in] extents Specifies the user extents of the clipping bounds
    void enableClippingPlanes(vector<double> minExts, vector<double> maxExts, vector<int> axes) const;

    //! Enable clipping planes associated with the full 3D data domain of the VDC.
    //! \sa disableClippingPlanes
    void enableFullClippingPlanes();

#ifdef DEAD
    //! Enable clipping planes associated with the current RegionParams extents
    //! \sa disableClippingPlanes
    void enableRegionClippingPlanes();
#endif

    //! Enable clipping planes associated with horizontal extents of the current VDC.
    //! \sa disableClippingPlanes
    void enable2DClippingPlanes();
    //! Disable the clipping planes that were previously enabled during
    //! enableClippingPlanes(), enableFullClippingPlanes(), enableRegionClippingPlanes, or enable2DClippingPlanes()
    void disableClippingPlanes();

    static const int _imgHgt;
    static const int _imgWid;
    unsigned char *  _colorbarTexture;
    TextObject *     _textObject;
    string           _fontFile;

private:
    size_t _timestep;

#ifdef DEAD
    static ControlExec *_controlExec;
#endif
};

//////////////////////////////////////////////////////////////////////////
//
// RendererFactory Class
//
/////////////////////////////////////////////////////////////////////////

class PARAMS_API RendererFactory {
public:
    static RendererFactory *Instance()
    {
        static RendererFactory instance;
        return &instance;
    }

    void RegisterFactoryFunction(string myName, string myParamsName, function<Renderer *(const ParamsMgr *, string, string, string, string, DataMgr *)> classFactoryFunction)
    {
        // register the class factory function
        _factoryFunctionRegistry[myName] = classFactoryFunction;
        _factoryMapRegistry[myName] = myParamsName;
    }

    Renderer *(CreateInstance(const ParamsMgr *pm, string winName, string dataSetName, string classType, string instName, DataMgr *dataMgr));

    string              GetRenderClassFromParamsClass(string paramsClass) const;
    string              GetParamsClassFromRenderClass(string renderClass) const;
    std::vector<string> GetFactoryNames() const;

private:
    map<string, function<Renderer *(const ParamsMgr *, string, string, string, string, DataMgr *)>> _factoryFunctionRegistry;
    map<string, string>                                                                             _factoryMapRegistry;

    RendererFactory() {}
    RendererFactory(const RendererFactory &) {}
    RendererFactory &operator=(const RendererFactory &) { return *this; }
};

//////////////////////////////////////////////////////////////////////////
//
// RendererRegistrar Class
//
// Register RendererBase derived class with:
//
//	static RendererRegistrar<RendererClass> \
//		registrar("myclassname", "myparamsclassname");
//
// where 'RendererClass' is a class derived from 'Renderer', and
// "myclassname" is the name of the class
//
/////////////////////////////////////////////////////////////////////////

template<class T> class RENDER_API RendererRegistrar {
public:
    RendererRegistrar(string classType, string paramsClassType)
    {
        // register the class factory function
        //
        RendererFactory::Instance()->RegisterFactoryFunction(classType, paramsClassType,
                                                             [](const ParamsMgr *pm, string winName, string dataSetName, string classType, string instName, DataMgr *dataMgr) -> Renderer * {
                                                                 return new T(pm, winName, dataSetName, instName, dataMgr);
                                                             });
    }
};

};    // namespace VAPoR

#endif    // RENDERER_H
