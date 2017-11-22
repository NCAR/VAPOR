//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		ParamsMgr.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		February 2016 
//
//	Description:	Defines the ParamsMgr  class.
//		This provides an API for manipulating the Params Objects that are used in VAPOR applications
//
#ifndef PARAMSMGR_H
#define PARAMSMGR_H

#include <cassert>
#include <map>
#include <deque>
#include <stack>
#include <utility>
#include <functional>

#include <vapor/DataMgr.h>
#include <vapor/ParamsBase.h>
#include <vapor/RenderParams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/regionparams.h>
#include <vapor/VizFeatureParams.h>

namespace VAPoR{

//!
//!

//! \class ParamsMgr
//! \ingroup Public_Params
//! \brief A singleton class for managing Params instances
//! 
//! \author Alan Norton
//! \version 3.0
//! \date    February 2016
//!

//!
class PARAMS_API ParamsMgr : public MyBase {
	
public: 

 //! ParamsMgr constructor
 //!
 //! \param[in] appParamsNames A vector of unique ParamsBase class 
 //! names previously
 //! registered with ParamsRegistrar(). The ParamsMgr will construct
 //! these application-defined classes as needed.
 //! If any of the class names in \p appParamsNames were not 
 //! previously registered via ParamsRegistrar() they will be ignored.
 //!
 //! \param[in] appRenderParamsNames A vector of unique RenderParams class 
 //! names previously
 //! registered with RenParamsRegistrar(). The ParamsMgr will construct
 //! these application-defined render params classes as needed.
 //! If any of the class names in \p appRenderParamsNames were not 
 //! previously registered via RenParamsRegistrar() they will be ignored.
 //!
 //! \sa ParamsRegistrar()
 //
 ParamsMgr(
	std::vector <string> appParamNames = std::vector <string> (),
	std::vector <string> appRenderParamNames = std::vector <string> ()
 );

 //! Destroy object
 //!
 virtual ~ParamsMgr();

 //! Load the default state
 //!
 //! This method resets the entire parameter state to the default values.
 //
 virtual void LoadState();

 //! Load the parameter state from an XmlNode tree
 //!
 //! This method resets the entire parameter state to the state specified
 //! by the XmlNode tree whose root is \p node. Any unrecognized nodes
 //! in \p node will be ignored.
 //
 virtual void LoadState(const XmlNode *node);

 //! Load the parameter state from a file
 //!
 //! This method resets the entire parameter state to the state specified
 //! by the file named by \p stateFile. Any unrecognized nodes
 //! in \p stateFile will be ignored.
 //
 virtual int LoadState(string stateFile);

 //! Add a DataMgr to the ParamsMgr class
 //!
 //! This method associates a data set name \p dataSetName with a 
 //! DataMgr \p dataMgr. Methods on this class that take a data set
 //! name as an argument can not be invoked until the data set name is
 //! bound to a DataMgr instance.
 //!
 void AddDataMgr(string dataSetName, DataMgr *dataMgr);

 //! Remove a DataMgr instance previously.
 //!
 //! This method removes the association of a DataMgr instance with
 //! the data set name \p dataSetName, previously made by AddDataMgr()
 //!
 void RemoveDataMgr(string dataSetName);

 //! Return list of all DataMgr names
 //!
 //! Return a list of all DataMgr names bound with AddDataMgr()
 //
 vector <string> GetDataMgrNames() const;



 //! Create a new ViewpointParams instances
 //!
 //! This method will create a new ViewpointParams instance
 //!
 //! \param [in] winName Window name to associate the new RenderParams
 //! object with.
 //!
 //! \retval ptr Returns a pointer to the newly created object on success,
 //! and NULL on failure. 
 //!
 //! \sa RenParamsRegistrar, RenParamsFactory
 //! GetRenderParams()
 //
 ViewpointParams *CreateVisualizerParamsInstance(string winName);

 //! Create a new rendering params instances
 //!
 //! This method will create a new instance of an object derived
 //! from the RenderParams class. The object will be created with
 //! the RenParamsFactory and its type must have been 
 //! registered with RenParamsRegistrar. The new RenderParams 
 //! instance will be associated with the window named by \p winName.
 //!
 //! \param [in] winName Window name to associate the new RenderParams
 //! object with.
 //! \param [in] classType Class name used to register the derived RenderParams
 //! object with.
 //! \param [in] instName Name to associate with the new object.
 //!
 //! \retval ptr Returns a pointer to the newly created object on success,
 //! and NULL on failure. This method will fail if the data set 
 //! \p dataSetName was not previously bound with AddDataMgr()
 //! exist, or if \p classType does not refer to a valid RenderParams
 //! derived class.
 //!
 //! \sa RenParamsRegistrar, RenParamsFactory, AddDataMgr(), 
 //! GetRenderParams(), AddDataMgr()
 //! \sa RemoveRenderParamsInstance()
 //
 RenderParams *CreateRenderParamsInstance(
	string winName, string dataSetName, string classType, string instName
 );

 //! Create a render params instance from an existing one
 //!
 //! Copies the RenderParams instance \p rp into the hierarchy and
 //! associates it with the visualizer \p winName and gives it the name
 //! \p instName. If a RenderParams instance is already associated with
 //! \p winName and \p instName the existing RenderParams instance is
 //! destroyed.
 //!
 //! \param[in] winName Window name to associate the new RenderParams
 //! object with.
 //! \param[in] instName Name to associate with the new object.
 //! \param[in] rp Pointer to a valid RenderParams instance to insert
 //!
 //! \retval pointer Pointer to the newly created RenderParams instance
 //
 RenderParams *CreateRenderParamsInstance(
	string winName, string dataSetName, string instName, const RenderParams *rp
 );

 //! Remove a previously created RenderParams instance
 //!
 //! This method removes from the session state a previously created
 //! RenderParams instance. If the identified RenderParams instance
 //! does not exist this method is a no-op.
 //!
 //! \params [in] winName Window name that the RenderParams instance is
 //! associated with.
 //! \param [in] classType Class name used to register the derived RenderParams
 //! object with.
 //! \param [in] instName Name associated with the object to destroy.
 //!
 //! \sa RenParamsRegistrar, RenParamsFactory 
 //! GetRenderParams()
 //! \sa CreateRenderParamsInstance()
 //
 void RemoveRenderParamsInstance(
    string winName, string dataSetName, string classType, string instName
 );


 //! Return a previously created RenderParams instance
 //!
 //! This method returns from the session state a previously created
 //! RenderParams instance. If the identified RenderParams instance
 //! does not exist this A NULL is returned, but no error is generated.
 //!
 //! \params [in] winName Window name that the RenderParams instance is
 //! associated with.
 //! \param [in] classType Class name used to register the derived RenderParams
 //! object with.
 //! \param [in] instName Name associated with the object to destroy.
 //!
 //! \sa RenParamsRegistrar, RenParamsFactory, AddDataMgr(), 
 //! GetRenderParams()
 //! \sa CreateRenderParamsInstance()
 //!
 //! \retval ptr If the identified RenderParams instance exists in the 
 //! session state it is returned, otherwise NULL is returned. The latter
 //! case does not generate an error
 //
 RenderParams *GetRenderParams(
	string winName, string dataSetName, string classType, string instName
 ) const;

 void GetRenderParams(
	string winName, string dataSetName, vector <RenderParams *> &rParams
 ) const;

 void GetRenderParams(
	string winName, vector <RenderParams *> &rParams
 ) const;

 void GetRenderParams(vector <RenderParams *> &rParams) const;

 //! Return all render param instance names
 //!
 //! Return all of the RenderParam instance names associated with
 //! the visualizer (window) named \p winName, the data set named 
 //! \p dataSetName, and the Params class type \p classType
 //!
 //! The returned names are guaranteed to be unique.
 //!
 void GetRenderParamNames(
	string winName, string dataSetName, string classType,
	vector <string> &instNames
 ) const;

 void GetRenderParamNames(
	string winName, string dataSetName, 
	vector <string> &instNames
 ) const;

 void GetRenderParamNames(string winName, vector <string> &instNames) const;

 void GetRenderParamNames(vector <string> &instNames) const;

 //! Lookup window, data set, and class name from a render instance name
 //!
 //! This method returns the window name \p winName, data set name 
 //! \p dataSetName, and render params type \p className that are associated
 //! with the render instance name \p instName. 
 //!
 //! \retval status True on success, false if \p instName is not a previously
 //! defined render instance name
 //!
 //! \sa CreateRenderParamsInstance
 //
 bool RenderParamsLookup(
	string instName, string &winName, string &dataSetName, string &className
 ) const;


 //! Returns all defined window (aka visualizer names).
 //!
 //! This method will return all defined window names in the session state
 //!
 //! \retval names A vector of window names, possibly an empty vector if
 //! none exist.
 //!
 //! \sa CreateRenderParamsInstance()
 //
 vector <string> GetVisualizerNames() const;

 //! Returns renderer types (aka class names) defined for window \p winName
 //!
 //! This method returns a list of all RenderParams types associated
 //! with the window \p winName in the session state
 //!
 //! \sa CreateRenderParamsInstance()
 //! \sa RenParamsRegistrar, RenParamsFactory
 //
 vector <string> GetRenderParamsClassNames(string winName) const;

 //! Returns renderer types (aka class names) defined for window \p winName
 //! associated with a data set named by \p dataSetName.
 //! This method returns a list of all RenderParams types associated
 //! with the window \p winName in the session state
 //!
 //! \sa CreateRenderParamsInstance()
 //! \sa RenParamsRegistrar, RenParamsFactory
 //
 vector <string> GetRenderParamsClassNames(
	string winName, string dataSetName
 ) const;

 //! Returns available renderer types (aka class names) 
 //!
 //! This method returns a list of all registered RenderParams types 
 //! (class names)
 //!
 //! \sa CreateRenderParamsInstance()
 //! \sa RenParamsRegistrar, RenParamsFactory
 //
 static vector <string> GetRenderParamsClassNamesAvail() {
	return(RenParamsFactory::Instance()->GetFactoryNames());
 }

 //! Returns renderer instance names defined on window \p winName for
 //! renderer type \p classname
 //!
 //! This method returns the instance names of all RenderParams of 
 //! type \p classType that are associated with the window \p winName
 //! in the current session state.
 //!
 //! \sa CreateRenderParamsInstance()
 //! \sa RenParamsRegistrar, RenParamsFactory
 //
 vector <string> GetRenderParamInstances(
	string winName, string dataSetName, string classType
 ) const;

 vector <string> GetRenderParamInstances(
	string winName, string classType
 ) const;

 //! Obtain the ViewpointParams that are applicable in a particular Visualizer
 //! window.
 //!
 //! Return the ViewpointParams instance associated with the
 //! window named by \p winName in the current session state
 //!
 //! \retval ptr ViewpointParams instance that is applicable.
 //
 ViewpointParams* GetViewpointParams(string winName) const ;

 //! Obtain the RegionParams that are applicable in a particular Visualizer
 //! window.
 //!
 //! Return the RegionParams instance associated with the
 //! window named by \p winName in the current session state
 //!
 //! \retval ptr RegionParams instance that is applicable.
 //
 RegionParams* GetRegionParams(string winName) const {
	return((RegionParams *) 
		_otherParams->GetParams(RegionParams::GetClassType())
	);
 };

 //! Obtain the VizFeatureParams that are applicable in a particular Visualizer
 //! window.
 //!
 //! Return the VizFeatureParams instance associated with the
 //! window named by \p winName in the current session state
 //!
 //! \retval ptr VizFeatureParams instance that is applicable.
 //
 VizFeatureParams* GetVizFeatureParams(string winName) const {
	return((VizFeatureParams *) 
		_otherParams->GetParams(VizFeatureParams::GetClassType())
	);
 };


 //! Optain any paramers registered by the application
 //!
 //! This method returns params that have been registered on the
 //! ParamsMgr via the constructor
 //!
 //! \retval params Pointer to requested params on success, or NULL
 //! on failure. Fails if \p classType was not registred with 
 //! the constructor or ParamsRegistrar
 //!
 //! \sa ParamsMgr()
 //
 ParamsBase *GetParams(string classType) {
	return(_otherParams->GetParams(classType));
 }

 //! Optain any render paramers registered by the application
 //!
 //! This method returns params that have been registered on the
 //! ParamsMgr via RegisterAppParams() for the data set named
 //! by \p dataSetName;
 //!
 //! \param[in] dataSetName  
 //! \param[in] classType 
 //!
 //! \retval params Pointer to requested params on success, or NULL
 //! on failure. Fails if \p classType was not registred with 
 //! the constructor or RenParamsRegistrar
 //! 
 //!
 //! \sa ParamsMgr()
 //
 RenderParams *GetAppRenderParams(string dataSetName, string classType) {
	std::map <string, RenParamsContainer *>::const_iterator itr;
	itr = _otherRenParams.find(dataSetName);
	return(itr != _otherRenParams.cend() ? itr->second->GetParams(classType) : NULL);
 }

 
 //! Save current state to a file
 //!
 //! Save the current state of the parameter database to an XML file
 //!
 //! \param[in] path Path to file
 //
 int SaveToFile(string path) const;

 //const DataMgr *GetDataMgr() const {return (_dataMgr);}

 //! Begin state save group
 //!
 //! Changes in state can be undone (redone) one at a time using
 //! Undo() and Redo(), or can be grouped to together into a collection.
 //! This method announces the start of such a collection group. The group
 //! will be completed when EndSaveStateGroup() is called. 
 //! When a subsequent call
 //! to Undo() or Redo() is made all of the state changes made within the 
 //! group are undone (redone) at once. Groups may be nested, in
 //! which case the outermost group prevails.
 //!
 //! \param[in] description A descriptive name for the group
 //!
 //! \sa EndSaveStateGroup()
 //!
 void BeginSaveStateGroup(string description) {
	_ssave.BeginGroup(description);
 }

 //! End state save group
 //! \sa BeginSaveStateGroup()
 //
 void EndSaveStateGroup() {
	_ssave.EndGroup();
 };

 void SetSaveStateEnabled( bool enabled) {
	_ssave.SetEnabled(enabled);
 }

 bool GetSaveStateEnabled() const { return (_ssave.GetEnabled()); }

 //! Restore state to previously saved state
 //!
 //! \retval status Returns true on success, false if the state is unchanged
 //! \sa BeginSaveStateGroup()
 //
 bool Undo();

 //! Restore state to state that existed prior to the last Undo()
 //!
 //! \retval status Returns true on success, false if the state is unchanged
 //! \sa BeginSaveStateGroup()
 //
 bool Redo();

 void UndoRedoClear();

 //! Return number states saved that can be undone with Undo()
 //!
 size_t UndoSize() const {
	return(_ssave.UndoSize());
 }

 size_t RedoSize() const {
	return(_ssave.RedoSize());
 }

 //! Register a boolean flag to capture state changes
 //!
 //! This method registers the address of boolean flag whose value
 //! will be set whenever the parameter state changes. It is the user's
 //! responsbility to clear (set to false) the flag. Note, for changes
 //! grouped tegoer with BeginSaveStateGroup() the flag will not be set
 //! until after EndSaveStateGroup() is called, and in the case of 
 //! nested groups, not until the last EndSaveStateGroup() invocation.
 //
 void RegisterStateChangeFlag(bool *flag) {
	_ssave.RegisterStateChangeFlag(flag);
 }

 //! Register a state change callback
 //!
 //! This method is similar to RegisterStateChangeFlag(). However, instead of
 //! setting a boolean flag, the function specified by \p callback
 //! will be invoked on state changes
 //
 void RegisterStateChangeCB(std::function<void()> callback) {
	_ssave.RegisterStateChangeCB(callback);
 }

 //! Reinit state saving
 //!
 //! 
 void RebaseStateSave() {
	_ssave.Rebase();
 }

 const XmlNode *GetXMLRoot() const {
	return(_rootSeparator->GetNode());
 }

 //! Return true if any state changes made since last call
 //!
 //! This method returns a boolean indicating whether any changes
 //! have been made to the parameter state since the last time
 //! the method was called. The first time StateChanged() is called
 //! it will return true;
 //
 bool StateChanged() { 
	if (*(_rootSeparator->GetNode()) == _prevState) return(false);

	_prevState = *(_rootSeparator->GetNode());
	return(true);
 }



private:

 class PARAMS_API PMgrStateSave : public ParamsBase::StateSave {
 public:

  PMgrStateSave(int stackSize = 100);
  ~PMgrStateSave();
  
  
  void Reinit(const XmlNode *rootNode) {
 	_rootNode = rootNode;
	emitStateChange();
  }

  void Rebase() {
	if (_state0) delete _state0;
	_state0 = new XmlNode(*_rootNode);
  }
  void Save(const XmlNode *node, string description);
  void BeginGroup(string descripion);
  void EndGroup();

  void SetEnabled(bool onOff) {
	if (! _groups.empty()) return;	// Can't change inside group
	_enabled = onOff;
  }

  bool GetEnabled() const { return (_enabled); }

  const XmlNode *GetTop(string &description) const;
  const XmlNode *GetBase() const {
	return(_state0);
  }

  bool Undo();
  bool Redo();
  void Clear();

  size_t UndoSize() const {
	return(_undoStack.size());
  }

  size_t RedoSize() const {
	return(_redoStack.size());
  }

  void RegisterStateChangeFlag(bool *flag) {
	_stateChangeFlags.push_back(flag);
  }
  void RegisterStateChangeCB(std::function<void()> callback) {
	_stateChangeCBs.push_back(callback);
  }

 private:

  bool _enabled;
  int _stackSize;
  const XmlNode *_rootNode;
  const XmlNode *_state0;

  std::stack <string>  _groups;
  std::deque <std::pair <string, XmlNode *>> _undoStack;
  std::deque <std::pair <string, XmlNode *>> _redoStack;

  std::vector <bool *> _stateChangeFlags;
  std::vector <std::function<void()> >_stateChangeCBs;

  void cleanStack(int maxN, std::deque <std::pair <string, XmlNode *>> &s);
  void emitStateChange();
   
 };
 
 map <string, DataMgr *> _dataMgrMap;
 ParamsSeparator *_rootSeparator;
 XmlNode _prevState;
 std::vector <string> _appParamNames;
 std::vector <string> _appRenderParamNames;

 // Map of RenParamsContainers referenced by Window Name, then by
 // data set name, and finally Renderer Name.
 //
 map <string, map <string, map <string,RenParamsContainer *>>> _renderParamsMap;

 // Map of ViewpointParams referenced by Window Name
 //
 map <string, ViewpointParams *> _viewpointParamsMap;

 ParamsContainer *_otherParams;
 std::map <string, RenParamsContainer *> _otherRenParams;

 PMgrStateSave _ssave;

 static const string _rootTag;
 static const string _globalTag;
 static const string _renderersTag;
 static const string _appRenderersTag;
 static const string _windowsTag;

 void _init(std::vector <string> appParamNames, XmlNode *node);
 void _initAppRenParams(string dataSetName);
 void _destroy();

 const map <string, map <string, RenParamsContainer *>> *getWinMap3(
	const map <string, map <string, map <string, RenParamsContainer *>>> &m3,
	string key
 ) const;

 const map <string, RenParamsContainer *> *getWinMap3(
	const map <string, map <string, map <string, RenParamsContainer *>>> &m3,
	string key1, string key2
 ) const;
 
 const map <string, RenParamsContainer *> *getWinMap2(
	const map <string, map <string, RenParamsContainer *>> &m2,
	string key
 ) const;

 RenParamsContainer *get_ren_container(
	string winName, string dataSetName, string renderName
 ) const;

 map <string, map <string, RenParamsContainer *>> *getWinMap3(
	map <string, map <string, map <string, RenParamsContainer *>>> &m3,
	string key
 ) const;

 map <string, RenParamsContainer *> *getWinMap3(
	map <string, map <string, map <string, RenParamsContainer *>>> &m3,
	string key1, string key2
 ) const;
 
 map <string, RenParamsContainer *> *getWinMap2(
	map <string, map <string, RenParamsContainer *>> &m2,
	string key
 ) const;

 void delete_ren_container(
	string winName, string dataSetName, string renderName
 );

 void delete_ren_containers(string winName, string dataSetName);
 void delete_ren_containers(string winName);
 void delete_ren_containers();

 RenParamsContainer *make_ren_container(
	string winName, string dataSetName, string renderName
 );

 ViewpointParams *get_vp_params(string winName) const;

 void delete_vp_params(string winName);

 ViewpointParams *make_vp_params(string winName);

 void addDataMgrNew();
 void addDataMgrMerge(string dataSetName);

 bool undoRedoHelper();

 RenParamsContainer *createRenderParamsHelper(
	string winName, string dataSetName, string className, string instName
 ) ;

};


}; //End namespace VAPoR
#endif //PARAMSMGR_H 
