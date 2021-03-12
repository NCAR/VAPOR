
#ifndef ControlExec_h
#define ControlExec_h

#include <string>
#include <vector>
#include <map>

#include <vapor/ParamsMgr.h>
#include <vapor/GLManager.h>

using namespace std;
namespace VAPoR {

class CalcEngineMgr;
class Visualizer;
class DataStatus;

//! \class ControlExec
//! \ingroup Public
//! \brief Provides API for VAPOR visualizer User Interfaces (UIs)
//

class RENDER_API ControlExec : public MyBase {
public:
    //! Initialize the control executive
    //!
    //! \param[in] appParamsNames A vector of unique ParamsBase class
    //! \param[in] cacheSizeMB Size of data cache expressed in Megabytes
    //!
    //! names that will be passed to the ParamsMgr constructor
    //!
    //! \sa ParamsMgr();
    //
    ControlExec(std::vector<string> appParamsNames = std::vector<string>(), std::vector<string> appRenderParamsNames = std::vector<string>(), size_t cacheSize = 1000, int nThreads = 0);
    virtual ~ControlExec();

    //! Set the ControlExec to a default state:
    //! Remove all visualizers
    //
    virtual void LoadState();

    //! Load state from an Xml tree
    //!
    //! Load state from an Xml state tree. Any unrecognized nodes
    //! in the tree are simply ignored.
    //!
    //! \sa ParamsMgr::LoadState(const XmlNode *node);
    //
    virtual void LoadState(const XmlNode *node);

    //!	Restore the session state from a session state file
    //!
    //! This method sets the session state based on the contents of the
    //! session file specified by \p file. It also has the side effect
    //! of deactivating all renderers and unloading any data set
    //! previously loaded by LoadData(). If successful, the state of all
    //! Params objects may have changed.
    //!
    //! \param[in] file	Path to the input file
    //!
    //! \return status A negative int indicates failure. If the method
    //! fails the session state remains unchanged
    //!
    //
    virtual int LoadState(string stateFile);

    //! Set number of execution threads
    //!
    //! Set the number of execution threads. If \p nThreads == 0, the
    //! default,
    //! the system will attempt to set the number of threads equal to
    //! the number of cores detected. Has no effect until
    //! the next data set is loaded.
    //
    void SetNumThreads(size_t nthreads);

    size_t GetNumThreads() const;

    //! Set the data cache size
    //!
    //! Set the size of the data cache in MBs.
    //! Has no effect until
    //! the next data set is loaded.
    //!
    //! \sa DataMgr
    //
    void SetCacheSize(size_t sizeMB);

    //! Create a new visualizer
    //!
    //! This method creates a new visualizer. A visualizer is a drawable
    //! OpenGL object (e.g. window or pbuffer). The caller is responsible
    //! for establishing an OpenGL drawable object and making its context
    //! current before calling NewVisualizer().
    //!
    //! \param[in] name Specifies the name for the new visualizer.
    //! If name visualizer with name \p name already exists the
    //! existing visualizer will be destroyed and a new one created
    //! with the same name.
    //!
    //! \param[in] withDatasets is the list of currently opened datasets.
    //! When a dataset is loaded, it loops over the existing visualizers
    //! and adds params for the currently loading dataset. Since datasets
    //! could've been loaded prior to the creation of this visualizer,
    //! they need to be manually added here.
    //!
    //! \note Need to establish what OpenGL state mgt, if any, is performed
    //! by UI. For example, who calls swapbuffers?
    //!
    //! \note Since the UI is responsible for setting up the graphics
    //! contexts we may need a method that allows the ControlExec
    //! to provide
    //! hints about what kind of graphics context is needed
    //! (e.g. double buffering)
    //
    int NewVisualizer(string name);

    //! Return tag pushed to the undo stack when NewVisualizer() is called
    //
    string GetNewVisualizerUndoTag() const { return ("NewVisualizer"); }

    //! Delete an existing visualizer
    //!
    //! Deletes visualizer and all of its renderers
    //!
    //! \param[in] name handle to existing visualizer returned by
    //! NewVisualizer(). This method is a no-op if a Visualizer named
    //! \p name doesn't exist
    //!
    //! \param[in] hasOpenGLContext If true it is the callers job to ensure
    //! that the OpenGL Context for the window \p winName is active. In
    //! this case the renderer is destroyed immediately. If false the
    //! renderer is queue'd for later destruction
    //! when \p winName has an active OpenGL context.
    //!
    //
    void RemoveVisualizer(string name, bool hasOpenGLContext = false);

    //! Return tag pushed to the undo stack when RemoveVisualizer() is called
    //
    string GetRemoveVisualizerUndoTag() const { return ("RemoveVisualizer"); }

    //! Perform OpenGL initialization of specified visualizer
    //!
    //! \param[in] name handle to existing visualizer returned by
    //! NewVisualizer(). This method is a no-op if a Visualizer named
    //!
    //! This method should be called by the UI once before any
    //! rendering is performed.
    //! The UI should make the OGL context associated with \p viz
    //! current prior to calling this method.
    //
    int InitializeViz(string name, GLManager *glManager);

    //! Notify the control executive that a drawing object has
    //! changed size.
    //!
    //! \param[in] name handle to existing visualizer returned by
    //! NewVisualizer().
    //! \param[in] width Width of visualizer
    //! \param[in] height Height of visualizer
    //!
    //! This method should be called by the UI whenever the drawing
    //! object (e.g. window) associated with \p viz has changed size.
    //! The UI should make the OGL context associated with \p viz
    //! current prior to calling this method.
    //
    int ResizeViz(string name, int width, int height);

    GLManager::Vendor GetGPUVendor() const;

    //! Determine how many visualizer windows are present
    //! \return number of visualizers
    //!
    int GetNumVisualizers() const { return (int)_visualizers.size(); }

    //! Return the names of all of the defined visualizers
    //!
    vector<string> GetVisualizerNames() const;

    //! Render the contents of a drawable
    //!
    //! Tells the control executive to invoke all active renderers associated
    //! with the visualizer \p name. The control executive may elect to
    //! ignore this method if it believes the rendering state to be current,
    //! unless \p force is true.
    //!
    //! The UI should make the OGL context associated with \p viz
    //! current prior to calling this method.
    //!
    //! \param[in] name handle to existing visualizer returned by
    //! NewVisualizer().
    //!	\param[in] force If true all active renderers will rerender
    //! their scenes. If false, rendering will only be performed if
    //! the params objects associated with any of the active renderers
    //! on this visualizer have changed state.
    //! \return rc is 0 if actual painting occurred, -1 if not.
    //!
    //!
    int Paint(string name, bool force = false);

    //! Activate or Deactivate a renderer

    //!
    //! Create and activate a new renderer instance.
    //! This renderer instance is inserted in the queue of active renderers
    //! for the visualizer.
    //! To deactivate a renderer, the associated Renderer instance is removed
    //! from the queue of active renderers in the Visualizer, and then deleted.
    //!
    //! \param[in] winName The visualizer associated with this renderer
    //! \param[in] type The type of renderer; i.e. the tag for the RenderParams.
    //! \param[in] renderName The instance name associated with this
    //! renderer.
    //! \param[in] on A boolean indicating if the renderer is to be made
    //! active (true) or inactive (off)
    //!
    //! \return status A negative int is returned on failure, indicating that
    //! the renderer cannot be activated
    //
    int ActivateRender(string winName, string dataSetName, string renderType, string renderName, bool on);

    int ActivateRender(string winName, string dataSetName, const RenderParams *rp, string renderName, bool on);

    //! Return tag pushed to the undo stack when ActivateRender() is called
    //
    string GetActivateRendererUndoTag() const { return ("ActivateRenderer"); }

    //! Remove (destroy) the indicated renderer
    //!
    //! \param[in] hasOpenGLContext If true it is the callers job to ensure that the
    //! OpenGL Context for the window \p winName is active. In this case the renderer
    //! is destroyed immediately. If false the renderer is queue'd for later destruction
    //! when \p winName has an active OpenGL context.
    //!
    void RemoveRenderer(string winName, string dataSetName, string renderType, string renderName, bool hasOpenGLContext);

    //! Return tag pushed to the undo stack when RemoveRenderer() is called
    //
    string GetRemoveRendererUndoTag() const { return ("RemoveRenderer"); }

    //! Remove (destroy) all renderers on this window
    //!
    //! \param[in] hasOpenGLContext If true it is the callers job to ensure that the
    //! OpenGL Context for the window \p winName is active. In this case the renderer
    //! is destroyed immediately. If false the renderer is queue'd for later destruction
    //! when \p winName has an active OpenGL context.
    //!
    void RemoveAllRenderers(string winName, bool hasOpenGLContext = false);

    //! Obtain the ParamsMgr, for use in accessing the Params instances.
    //! \return ParamsMgr*
    //
    ParamsMgr *GetParamsMgr() const { return _paramsMgr; }

    //! Save the current session state to a file
    //!
    //! This method saves all current session state information
    //! to the file specified by path \p file. All session state information
    //! is stored in Params objects and their derivatives
    //!
    //! \param[in] file	Path to the output file
    //!
    //! \return status A negative int indicates failure
    //!
    //! \sa RestoreSession()
    //
    int SaveSession(string file);

#ifdef VAPOR3_0_0_ALPHA
    //! Save user preferences to a file
    //!
    //! This method saves all preference information
    //! to the file specified by path \p file.
    //!
    //! \param[in] file	Path to the output file
    //!
    //! \return status A negative int indicates failure
    //!
    //! \sa RestorePreferences()
    //
    int SavePreferences(string file);

    //!	Restore the preferences from a preference file
    //!
    //! This method sets the preferences based on the contents of the
    //! preferences file specified by \p file.
    //!
    //! \param[in] file	Path to the input file
    //!
    //! \return status A negative int indicates failure. If the method
    //! fails the preferences remain unchanged (is it possible to
    //! guarantee this? )
    //!
    //! \sa SavePreferences()
    //
    int RestorePreferences(string file);
#endif

    //! Load a data set into the current session
    //!
    //! Loads a data set specified by the list of files in \p files
    //!
    //! \param[in] files A vector of data file paths. For data sets
    //! not containing explicit time information the ordering of
    //! time varying data will be determined by the order of the files
    //! in \p files.
    //!
    //!
    //! \note The proposed DataMgr API doesn't provide methods to easily
    //! query some of the information that will be required by the UI, for
    //! example, the coordinate extents of a variable. These methods
    //! should either be added to the DataMgr, or the current DataStatus
    //! class might be cleaned up. Hence, DataInfo might be an enhanced
    //! DataMgr, or a class object designed specifically for returning
    //! metadata to the UI.
    //! \note (AN) It would be much better to incorporate the
    //! DataStatus methods into
    //! the DataMgr class, rather than keeping them separate.
    //
    int OpenData(const std::vector<string> &files, const std::vector<string> &options, string dataSetName, string type = "vdc");

    //! Unloads the specified data set
    //!
    void CloseData(string dataSetName);

    //! Return list of currently open data set names
    //!
    //! \sa OpenData(), CloseData()
    //
    std::vector<string> GetDataNames() const { return (_paramsMgr->GetDataMgrNames()); }

    //! Obtain the current DataStatus
    //! Needed to store in GUI when the DataStatus changes.
    //! \return DataStatus*
    DataStatus *GetDataStatus() const { return _dataStatus; }

    //! Get RenderParams for an active renderer
    //!
    //! Return the RenderParams for a render of type \p renderType, associated
    //! the visualizer \p winName, and named \p instName
    //! with the specified
    //
    RenderParams *GetRenderParams(string winName, string dataSetName, string renderType, string instName) const;

    //! Get all activated render class names
    //!
    //! Get a list of all render class names currently active for
    //! the visualizer named by \p winName
    //!
    std::vector<string> GetRenderClassNames(string winName) const;

    //! Get all activated render class instance names
    //!
    //! Get a list of all render class instance names currently active for
    //! the visualizer named by \p winName of type \p renderType
    //!
    std::vector<string> GetRenderInstances(string winName, string renderType) const;

    //! Get all available render class types
    //!
    //! Return a vector of all availble render class type names
    //!
    static vector<string> GetAllRenderClasses();

    //! Lookup window, data set, and class name from a render instance name
    //!
    //! This method returns the window name \p winName, data set name
    //! \p dataSetName, and render type \p rendererType that are associated
    //! with the render instance name \p instName.
    //!
    //! \retval status True on success, false if \p instName is not a previously
    //! defined render instance name
    //!
    //! \sa ActivateRender
    //
    bool RenderLookup(string instName, string &winName, string &dataSetName, string &rendererType) const;

    //! Draw 2D text on the screen
    //!
    //! This method provides a simple interface for rendering text on
    //! the screen. No text will actually be rendered until after Paint()
    //! is called. Text rendering will occur after all active renderers
    //! associated with \p viz have completed rendering.
    //!
    //! \param[in] winName A visualizer handle returned by NewVisualizer()
    //! \param[in] text The text to render
    //! \param[in] x X coordinate in pixels coordinates of lower left
    //! corner of rectangle bounding the text.
    //! \param[in] y Y coordinate in pixels coordinates of lower left
    //! corner of rectangle bounding the text.
    //! \params[in] size Font size to be used
    //! \params[in] color Three tuple describing rgj values for the billboard
    //
    int DrawText(string winName, string text, int x, int y, int size, float color[3], int type = 0);
    int DrawTextNormalizedCoords(string winName, string text, float x, float y, int size, float color[3], int type = 0);

    //! Draw 2D text to all visualizers
    //!
    //! This method provides a simple interface for rendering text on
    //! all Visualizers. No text will actually be rendered until after Paint()
    //! is called. Text rendering will occur after all active renderers
    //! associated with \p viz have completed rendering.
    //!
    //! \param[in] text The text to render
    //! \param[in] x X coordinate in pixels coordinates of lower left
    //! corner of rectangle bounding the text.
    //! \param[in] y Y coordinate in pixels coordinates of lower left
    //! corner of rectangle bounding the text.
    //! \params[in] size Font size to be used
    //! \params[in] color Three tuple describing rgj values for the billboard
    //
    int DrawText(string text, int x, int y, int size, float color[3], int type = 0);
    int DrawTextNormalizedCoords(string text, float x, float y, int size, float color[3], int type = 0);

    int ClearText();
    int ClearText(string winName);

    //! Undo the last session state change
    //! Restores the state of the session to what it was prior to the
    //! last change made via a Params object, or prior to the last call
    //! to Undo() or Redo(), whichever happened last. I.e. Undo() can
    //! be called repeatedly to undo multiple state changes.
    //!
    //! State changes do not trigger rendering. It is the UI's responsibility
    //! to call Paint() after Undo(), and to make any UI internal changes
    //! necessary to reflect the new state.
    //!
    //! \return status A boolean indicating whether state was succefully
    //! restored to previous state.
    //!
    //! \sa Redo()
    //!
    bool Undo();

    //! Redo the next session state change
    //! Restores the state of the session to what it was before the
    //! last change made via Undo,Redo() can
    //! be called repeatedly to undo multiple state changes.
    //!
    //! State changes do not trigger rendering. It is the UI's responsibility
    //! to call Paint() after Redo(), and to make any UI internal changes
    //! necessary to reflect the new state.
    //!
    //! \return status A boolean indicating whether state was succefully
    //! restored to previous state.
    //!
    //! \sa UnDo()
    //
    bool Redo();

    //! Clear history from the Undo & Redo stack
    //!
    void UndoRedoClear();

    //! Return number of elements in Undo history
    //! \sa Undo()
    //!
    size_t UndoSize() const { return (_paramsMgr->UndoSize()); }

    //! Return number of elements in Redo history
    //! \sa Redo()
    //!
    size_t RedoSize() const { return (_paramsMgr->RedoSize()); }

    //! Enable or disable state saving
    //!
    //! Enable or disable session state saving. When enabled all state
    //! changes are recorded and it is possible to undo previous changes,
    //! or save current session state to a file
    //!
    //! State saving is disabled by default
    //!
    //! \sa Undo(), Redo(), SaveSession()
    //
    void SetSaveStateEnabled(bool enabled) { _paramsMgr->SetSaveStateEnabled(enabled); }

    bool GetSaveStateEnabled() const { return (_paramsMgr->GetSaveStateEnabled()); }

    //! Re-base state saving
    //!
    //! Set's the base state to the current node
    //!
    void RebaseStateSave() { _paramsMgr->RebaseStateSave(); }

    //! Capture the next rendered image to a file
    //! When this method is called, the next time Paint() is called for
    //! the specified visualizer, the rendered image
    //! will be written to the specified (.jpg, .tif, ...) file.
    //! The UI must call Paint(viz, true) after this method is called.
    //! If this is called concurrently with a call to Paint(), the
    //! image will not be captured until that rendering completes
    //! and another Paint() is initiated.
    //! Only one image will be captured.
    //! \param[in] filename is either .jpg, .tif, or .tiff file name to capture
    //! \param[in] viz Valid visualizer handle
    int EnableImageCapture(string filename, string winName);

    //! Start or stop capturing a sequence of rendered images
    //! When this method is called, the next time Paint() is called for
    //! the specified visualizer, the rendered image
    //! will be written to the specified (.jpg, .tif, ...) file.
    //! The UI must call Paint(viz, true) after this method is called.
    //! Subsequent renders in the same visualizer will result in capture to a file
    //! and the filename will be incremented by 1.
    //! The starting filename should terminate with digits to permit incrementing.
    //! filename is ignored if capture is being disabled

    //! \param[in] viz Valid visualizer handle
    //! \param[in] doEnable true to start capture, false to end.
    //! \param[in] filestart is either .jpg, .tif, or .tiff file
    //! name for first capture.  Ignored if doEnable = false.
    //!
    int EnableAnimationCapture(string winName, bool doEnable, string filename = "");

    //! Make string conformant for library
    //!
    //! Many of the methods provided by the API accept string arguments
    //! defining names of objects. These user-defined names must conform
    //! to Xml element name requirements:
    //!
    //! \li Element names are case-sensitive
    //! \li Element names must start with a letter or underscore
    //! \li Element names cannot start with the letters xml (or XML, or Xml, etc)
    //! \li Element names can contain letters, digits, hyphens,
    //! underscores, and periods
    //! \li Element names cannot contain spaces
    //!
    //! This method ensures \p s is conformant, returning a possibly
    //! modified string meeting the above requirements
    //!
    //! \param[in] s a string
    //! \retval Returns \p s, possibly modified to be XML element conformant
    //
    static string MakeStringConformant(string s);

    //! Add a variable calculation function
    //!
    //! This method adds one or more derived variables to the data set
    //! specified by \p dataSetName.
    //! Each derived variable is calculated on-the-fly as needed
    //! by executing
    //! the script \p script.
    //!
    //! \param[in] scriptName A string identifier for the collection
    //! of derived variables
    //! computed by \p script
    //!
    //! \param[in] scriptType The language the script contained in \p script
    //! is implemented in. Currently the only accepted value is "Python".
    //!
    //! \param[in] dataSetName The name of the data set for which this
    //! script is to be run. See DataStatus::GetDataMgrNames()
    //!
    //! \param[in] script A script that will be invoked each time
    //! one of the variables listed in \p outputs is accessed. The scope of
    //! the script will contain NumPy Array (numpy.array) variables named
    //! in \p inputs.
    //!
    //! \param[in] inputVarNames A list of input variable names. The named
    //! variables will be made available in the scope of the Python
    //! script as NumPy Arrays. The variables must exist as native variables
    //! of the dataset named by \p dataSetName. I.e. Input variables cannot
    //! be derived variables themselves.
    //!
    //! \param[in] outputVarNames A list of derived variable names. The named
    //! variables are expected to be computed by \p script as NumPy Arrays
    //! and will appear as derived variables in the dataset named by
    //! \p dataSetName.
    //!
    //! \param[in] outVarMeshes A list of output mesh names, one for each output
    //! variable listed in \p outputVarNames. The output mesh names must be known
    //! to the dataset. Each output variable created by \p script is expected
    //! to be sampled by the named mesh. See DataMgr::GetMesh()
    //!
    //! \retval status A negative integer is returned on failure and an error
    //! message is reported with MyBase::SetErrMsg(). AddFunction() will fail
    //! if any of the output variables named in \p outputVarNames already exist
    //! in the dataset as returned by DataMgr::GetDataVarNames(), or if any of
    //! the output mesh names in \p outVarMeshes are not known to the
    //! DataMgr (see DataMgr::GetMeshNames())
    //!
    //! \note The Python script \p script is executed when one of the output
    //! variables is read. Depending on the region requested only a subset
    //! of the native variable may be provided to \p script as a NumPy
    //! array. Currently this occurs if all of the input variables named
    //! by \p inputs and the requested output variable are sampled on the
    //! same mesh.
    //!
    //! \sa PyEngine::AddFunction(), CalcEngineMgr::AddFunction()
    //
    int AddFunction(string scriptType, string dataSetName, string scriptName, string script, const vector<string> &inputVarNames, const vector<string> &outputVarNames,
                    const vector<string> &outputVarMeshes, bool coordFlag = false);

    //! Remove a previously defined function
    //!
    //! This method removes the function previously created by AddFunction()
    //! All of the associated derived variables are
    //! removed from the dataset. The method is a no-op if \p scriptName is not
    //! an active function.
    //!
    //! \param[in] scriptType Language of script. Currently only "Python" is
    //! supported
    //!
    //! \param[in] dataSetName Name of data set for which \p scriptName is to
    //! be removed.
    //!
    //! \param[in] scriptName Name of script to remove
    //!
    //! \sa PyEngineMgr::RemoveFunction()
    //
    void RemoveFunction(string scriptType, string dataSetName, string scriptName);

    //! Return the script for a named function
    //!
    //! This method returns as a string the script associated with the
    //! function named by \p name. It also returns the input and output
    //! variable names, and the output variable mesh names.
    //!
    //! \param[in] scriptType Type of script
    //! \param[in] dataSetName Name of data set
    //! \param[in] scriptName Name of script
    //!
    //! \param[out] script Script returned as a string
    //! \param[out] inputVarNames Vector of input variable names to script
    //! \param[out] outputVarNames Vector of output variable names generated
    //! by script
    //! \param[out] outputVarMeshes Vector of output variable mesh names
    //! associated with \p outputVarNames
    //!
    //! \retval status Returns true upon success. If the tupple \p scriptName
    //! \p dataSetName, \p scriptName is invalid false is returned.
    //!
    //! \sa AddFunction(), RemoveFunction()
    //
    bool GetFunction(string scriptType, string dataSetName, string scriptName, string &script, vector<string> &inputVarNames, vector<string> &outputVarNames, vector<string> &outputVarMeshes,
                     bool &coordFlag) const;

    //! Return any standard output from the last invocation of a script
    //!
    //! This method returns as a string any standard output from the last
    //! (most recent) invocation of the named script
    //
    string GetFunctionStdout(string scriptType, string dataSetName, string scriptName) const;

    //! Return a list of all active function names
    //!
    //! This method returns a vector of names for all active (not previously
    //! removed) functions created with AddFunction() for the specified
    //! dataset \p dataSetName and the specifed script language type \p scriptType
    //!
    //! \sa AddFunction();
    //!
    std::vector<string> GetFunctionNames(string scriptType, string dataSetName) const;

private:
    ParamsMgr *                    _paramsMgr;
    DataStatus *                   _dataStatus;
    CalcEngineMgr *                _calcEngineMgr;
    std::map<string, Visualizer *> _visualizers;

    GLManager::Vendor _cachedVendor = GLManager::Vendor::Unknown;

    //! obtain an existing visualizer
    //! \param[in] viz Handle of desired visualizer
    //! \return pointer to specified visualizer
    //!
    Visualizer *getVisualizer(string winName) const
    {
        std::map<string, Visualizer *>::const_iterator it;
        it = _visualizers.find(winName);
        if (it == _visualizers.end()) return NULL;
        return it->second;
    }

    int  openDataHelper(bool reportErrs);
    bool _undoRedoRenderer(bool undoFlag);
    bool _undoRedoVisualizer(bool undoFlag);

    int activateClassRenderers(string vizName, string dataSetName, string pClassName, vector<string> instNames, bool reportErrs);

    void _removeRendererHelper(string winName, string dataSetName, string paramsType, string renderName, bool removeFromParamsFlag, bool hasOpenGLContext);
};
};    // namespace VAPoR

#endif    // ControlExec_h
