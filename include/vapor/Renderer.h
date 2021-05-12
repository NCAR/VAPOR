
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

namespace VAPoR {

class ShaderProgram;
struct GLManager;

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
    virtual int initializeGL(GLManager *glManager);

    //! Obtain the Visualizer associated with this Renderer
    string GetVisualizer() { return _winName; }

    //! Identify the name of the current renderer
    //! \return name of renderer
    string GetMyName() const { return (_instName); };
    string GetInstanceName() const { return GetMyName(); }

    //! Identify the type of the current renderer
    //! \return type of renderer
    string GetMyType() const { return (_classType); };

    //! Identify the params belonging to the current renderer
    //! \return params for renderer
    string GetMyParamsType() const { return (_paramsType); };

    //! Identifiy the dataset associated with the current renderer
    //! \return dataset name
    string GetMyDatasetName() const { return (_dataSetName); };

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

    GLManager *_glManager;

    //! Pure virtual method
    //! Any OpenGL initialization is performed in initializeGL
    //! It will be called from an OpenGL rendering context.
    virtual int _initializeGL() = 0;

public:
    //! Renderers need to be deleted during the draw loop
    //! to ensure the correct OpenGL context is bound
    void FlagForDeletion();
    bool IsFlaggedForDeletion() const;

    RendererBase() {}

private:
    bool _glInitialized;
    bool _flaggedForDeletion;
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

    //! Get default z value at the base of the domain.  Useful
    //! for applying a height value to 2D renderers.
    //! \param[in] dataMgr Current (valid) dataMgr
    //! \retval default height value for current dataset
    double GetDefaultZ(DataMgr *dataMgr, size_t ts) const;

    //! All OpenGL rendering is performed in the paintGL method.
    //! This invokes _paintGL on the renderer subclass
    //! \param[in] dataMgr Current (valid) dataMgr
    //! \retval int zero if successful.
    virtual int paintGL(bool fast);

    //! Clear render cache
    //!
    //! Called whenever renderer should clear any cached data
    //
    void ClearCache() { _clearCache(); };

#ifdef VAPOR3_0_0_ALPHA
#endif

#ifdef VAPOR3_0_0_ALPHA
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

#ifdef VAPOR3_0_0_ALPHA
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

#ifdef VAPOR3_0_0_ALPHA
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

#ifdef VAPOR3_0_0_ALPHA

    //! Obtain the extents of a region that contains a rotated (3D) box associated with a renderer.
    //! \param[out] regMin Minimum coordinates of containing region.
    //! \param[out] regMix Maximum coordinates of containing region.
    void getLocalContainingRegion(float regMin[3], float regMax[3]);
#endif

    //! Render the colorbar for this renderer (if it has one)
    void renderColorbar();

    ///@}

    //! Obtain the current RenderParams instance
    //! \retval RenderParams* current render params
    RenderParams *GetActiveParams() const { return (_paramsMgr->GetRenderParams(_winName, _dataSetName, _paramsType, _instName)); }

    ViewpointParams *GetViewpointParams() const { return _paramsMgr->GetViewpointParams(_winName); }

    AnnotationParams *GetAnnotationParams() const { return _paramsMgr->GetAnnotationParams(_winName); }

    Transform *GetDatasetTransform() const { return GetViewpointParams()->GetTransform(_dataSetName); }

protected:
    Renderer() {}

    virtual std::string _getColorbarVariableName() const;

    //! All OpenGL rendering is performed in the pure virtual paintGL method.
    virtual int _paintGL(bool fast) = 0;

    //! Enable specified clipping planes during the GL rendering. This
    //! method clips the scene to the bounding box. See
    //!! RenderParams::GetBox()
    //! May be invoked during _paintGL() by classes derived from this class.
    //! Clipping planes are specified in User coordinates.
    //!
    //! \param[in] haloFrac Fraction of bounding box that should be used to
    //! extend the min and max clippig planes. For example, if a min extent
    //! is zero, and a max extent is 1.0, and haloFrac is 0.1 then the
    //! min clipping plane will be set to -0.1 and the max to 1.1
    //!
    //! \sa DisableClippingPlanes
    void EnableClipToBox(ShaderProgram *shader, float haloFrac = 0.0) const;

    //! Disable clipping planes.
    //! If clipping is enabled this  method should be called prior to
    //! returning from _paintGL()
    //
    void DisableClippingPlanes();

    //! Get clipping planes in the model/user coordinates.
    //! There are six planes in total that get stored in "planes"
    //! It is the caller's responsibility to allocate memory for 24 floats.
    void GetClippingPlanes(float planes[24]) const;

    //! return true if all of the specified variables exist in the DataMgr
    //! at the specified timestep, refinement level, and lod. If \p zeroOK
    //! is true variables named "0" or "" evaluate to true.
    //
    virtual bool VariableExists(size_t ts, std::vector<string> &varnames, int level, int lod, bool zeroOK) const;

    virtual void _clearCache() = 0;

    static const int _imgHgt;
    static const int _imgWid;
    unsigned char *  _colorbarTexture;
    string           _fontName;

private:
    size_t _timestep;

#ifdef VAPOR3_0_0_ALPHA
    static ControlExec *_controlExec;
#endif
};

//////////////////////////////////////////////////////////////////////////
//
// RendererFactory Class
//
/////////////////////////////////////////////////////////////////////////

class RENDER_API RendererFactory {
public:
    static RendererFactory *Instance();

    void RegisterFactoryFunction(string myName, string myParamsName, function<Renderer *(const ParamsMgr *, string, string, string, string, DataMgr *)> classFactoryFunction);

    Renderer *(CreateInstance(const ParamsMgr *pm, string winName, string dataSetName, string classType, string instName, DataMgr *dataMgr));

    string              GetRenderClassFromParamsClass(string paramsClass) const;
    string              GetParamsClassFromRenderClass(string renderClass) const;
    std::vector<string> GetFactoryNames() const;

private:
    map<string, function<Renderer *(const ParamsMgr *, string, string, string, string, DataMgr *)>> _factoryFunctionRegistry;
    map<string, string>                                                                             _factoryMapRegistry;

    RendererFactory();
    RendererFactory(const RendererFactory &);
    RendererFactory &operator=(const RendererFactory &);
};

//////////////////////////////////////////////////////////////////////////
//
// RendererRegistrar Class
//
// Register RendererBase derived class with:
//
//	static RendererRegistrar<RendererClass>
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
