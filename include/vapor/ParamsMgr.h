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

#include <vapor/ParamsBase.h>
#include <vapor/DataStatus.h>
#include <vapor/RenderParams.h>
#include <vapor/AnimationParams.h>
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

 ParamsMgr();

 //! ParamsMgr constructor
 //!
 //! \param[in] appParamsNames A vector of unique ParamsBase class 
 //! names previously
 //! registered with ParamsRegistrar(). The ParamsMgr will construct
 //! these application-defined classes as needed.
 //! If any of the class names in \p appParamsNames were not 
 //! previously registered via ParamsRegistrar() they will be ignored.
 //!
 //! \sa ParamsRegistrar()
 //
 ParamsMgr(std::vector <string> appParamNames);

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

 //! Set the data manager for the current session
 //!
 //! Set the data manager for the active session. A 
 //! DataStatus must be present before any RenderParams 
 //! objects can be created. 
 //!
 //! \sa CreateRenderParamsInstance(), GetRenderParams()
 //
 void SetDataStatus(DataStatus *dataStatus);


 //! Create a new ViewpointParams instances
 //!
 //! This method will create a new ViewpointParams instance
 //!
 //! \param [in] winName Window name to associate the new RenderParams
 //! object with.
 //!
 //! This method will fail if a valid DataStatus has not be established
 //! with SetDataStatus()
 //!
 //! \retval ptr Returns a pointer to the newly created object on success,
 //! and NULL on failure. This method will fail if a DataStatus does not
 //! exist.
 //!
 //! \sa RenParamsRegistrar, RenParamsFactory, SetDataStatus(), 
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
 //! This method will fail if a valid DataStatus has not be established
 //! with SetDataStatus()
 //!
 //! \retval ptr Returns a pointer to the newly created object on success,
 //! and NULL on failure. This method will fail if a DataStatus does not
 //! exist, or if \p classType does not refer to a valid RenderParams
 //! derived class.
 //!
 //! \sa RenParamsRegistrar, RenParamsFactory, SetDataStatus(), 
 //! GetRenderParams()
 //! \sa RemoveRenderParamsInstance()
 //
 RenderParams *CreateRenderParamsInstance(
	string winName, string classType, string instName
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
	string winName, string instName, const RenderParams *rp
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
 //! \sa RenParamsRegistrar, RenParamsFactory, SetDataStatus(), 
 //! GetRenderParams()
 //! \sa CreateRenderParamsInstance()
 //
 void RemoveRenderParamsInstance(
    string winName, string classType, string instName
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
 //! \sa RenParamsRegistrar, RenParamsFactory, SetDataStatus(), 
 //! GetRenderParams()
 //! \sa CreateRenderParamsInstance()
 //!
 //! \retval ptr If the identified RenderParams instance exists in the 
 //! session state it is returned, otherwise NULL is returned. The latter
 //! case does not generate an error
 //
 RenderParams *GetRenderParams(
	string winName, string classType, string instName
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
	string winName, string classType
 ) const;

 //! Obtain the AnimationParams that are applicable in a particular Visualizer
 //! window.
 //!
 //! Return the AnimationParams instance 
 //!
 //! \retval ptr AnimationParams instance that is applicable.
 //
 AnimationParams* GetAnimationParams() const {
	return((AnimationParams *) 
		_otherParams->GetParams(AnimationParams::GetClassType())
	);
 }

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
 //! ParamsMgr via RegisterAppParams();
 //!
 //! \retval params Pointer to requested params on success, or NULL
 //! on failure. Fails if \p classType was not registred with 
 //! RegisterAppParams()
 //!
 //! \sa RegisterAppParams()
 //
 ParamsBase *GetParams(string classType) {
	return(_otherParams->GetParams(classType));
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

 void RegisterStateChangeFlag(bool *flag) {
	_ssave.RegisterStateChangeFlag(flag);
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

 class PMgrStateSave : public ParamsBase::StateSave {
 public:

  PMgrStateSave(int stackSize = 100);
  ~PMgrStateSave();
  
  
  void Reinit(const XmlNode *rootNode) {
 	_rootNode = rootNode;
	for (int i=0; i<_stateChangeFlags.size(); i++) {
		*(_stateChangeFlags[i]) = true;
	}
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

 private:

  bool _enabled;
  int _stackSize;
  const XmlNode *_rootNode;

  std::stack <string>  _groups;
  std::deque <std::pair <string, XmlNode *>> _undoStack;
  std::deque <std::pair <string, XmlNode *>> _redoStack;

  void cleanStack(int maxN, std::deque <std::pair <string, XmlNode *>> &s);

  std::vector <bool *> _stateChangeFlags;
   
 };
 
 DataStatus *_dataStatus;
 ParamsSeparator *_rootSeparator;
 XmlNode _prevState;
 std::vector <string> _appParamNames;

 // Map of RenParamsContainers referenced by Window Name, and then by
 // Renderer Name.
 //
 map <string, map <string, RenParamsContainer *>> _renderParamsMap;

 // Map of ViewpointParams referenced by Window Name
 //
 map <string, ViewpointParams *> _viewpointParamsMap;

 ParamsContainer *_otherParams;
 PMgrStateSave _ssave;

 static const string _rootTag;
 static const string _globalTag;
 static const string _renderersTag;
 static const string _windowsTag;

 void _init(std::vector <string> appParamNames, XmlNode *node);
 void _destroy();

 RenParamsContainer *get_ren_container(string winName, string renderName) const;

 void delete_ren_container(string winName, string renderName);
 void delete_ren_containers(string winName);
 void delete_ren_containers();

 RenParamsContainer *make_ren_container(string winName, string renderName);

 ViewpointParams *get_vp_params(string winName) const;

 void delete_vp_params(string winName);

 ViewpointParams *make_vp_params(string winName);

 void setDataStatusNew(DataStatus *dataStatus);
 void setDataStatusMerge(DataStatus *dataStatus);

 bool undoRedoHelper();

};


#ifdef	DEAD

//! method that specifies the instance that is current in the identified window.
//! For non-Render Params, the current instance should always be 0;
//! \param[in] pType ParamsBase TypeId of the params class
//! \param[in] winnum index of identified window
//! \param[in] instance index of instance to be made current
	void SetCurrentParamsInstanceIndex(int pType, int winnum, int instance) {
		_currentParamsInstance[make_pair(pType,winnum)] = instance;
	}
	

//! method that returns the default Params instance.
//! Useful for application developers.
//! Based on XML tag (name) of Params class.
//! With non-render params this is the global Params instance.
//! \param[in] tag XML tag of the Params class
//! \retval Pointer to specified Params instance
	Params* GetDefaultParams(const string& tag) const {
		ParamsBase::ParamsBaseType ptype = GetTypeFromTag(tag);
		return GetDefaultParams(ptype);
	}

//! method that sets the default Params instance.
//! Useful for application developers.
//! With non-render params this is the global Params instance.
//! \param[in] pType ParamsBase TypeId of the params class
//! \param[in] p Pointer to default Params instance
	void SetDefaultParams(int pType, Params* p) {
		_defaultParamsInstance[pType] = p;
	}

//! method that specifies the default Params instance
//! Based on Xml Tag of Params class.
//! With non-render params this is the global Params instance.
//! Useful for application developers.
//! \param[in] tag XML Tag of the params class
//! \param[in] p Pointer to default Params instance
	void SetDefaultParams(const string& tag, Params* p) {
		_defaultParamsInstance[GetTypeFromTag(tag)] = p;
	}

//! Method that constructs a default Params instance.
//! Useful for application developers.
//! \param[in] tag name of the params class
//! \retval Pointer to new default Params instance, Null on failure
	Params* CreateDefaultParams(string tag);
		
//! method that tells how many instances of a Params class
//! exist for a particular visualizer.
//! Useful for application developers.
//! \param[in] pType ParamsBase TypeId of the params class
//! \param[in] winnum index of specified visualizer window
//! \retval number of instances that exist 
	int GetNumParamsInstances(int pType, int winnum) const ;

//! method that tells how many instances of a Params class
//! exist for a particular visualizer.
//! Based on the XML tag of the Params class.
//! Useful for application developers.
//! \param[in] tag XML tag associated with Params class
//! \param[in] winnum index of specified visualizer window
//! \retval number of instances that exist 
	int GetNumParamsInstances(const std::string tag, int winnum) const {
		return GetNumParamsInstances(GetTypeFromTag(tag), winnum);
	}
	
//! method that appends a new instance to the list of existing 
//! Params instances for a particular visualizer.
//! Useful for application developers.
//! \param[in] pType ParamsBase TypeId of the params class
//! \param[in] winnum index of specified visualizer window
//! \param[in] p pointer to Params instance being appended 
//! \return 0 if successful
	int AppendParamsInstance(int pType, int winnum, Params* p){
		_paramsInstances[make_pair(pType,winnum)].push_back(p);
		if (pType == 3 && _paramsInstances[make_pair(pType,winnum)].size() > 1){
			SetErrMsg("Error appending params instance");
			return -1;
		}
		return 0;
	}

//! method that appends a new instance to the list of existing 
//! Params instances for a particular visualizer.
//! Based on the XML tag of the Params class.
//! Useful for application developers.
//! \param[in] tag XML tag associated with Params class
//! \param[in] winnum index of specified visualizer window
//! \param[in] p pointer to Params instance being appended 
//! \return 0 if successful
	int AppendParamsInstance(const std::string tag, int winnum, Params* p){
		return AppendParamsInstance(GetTypeFromTag(tag),winnum, p);
	}

//! method that removes an instance from the list of existing 
//! Params instances for a particular visualizer.
//! Useful for application developers.
//! \param[in] tag ParamsBase tag of the params class
//! \param[in] winnum index of specified visualizer window
//! \param[in] instance index of instance to remove
//! \return zero if successful, -1 if error
	int RemoveParamsInstance(string tag, int winnum, int instance);
	


//! method that produces a list of all the Params instances of a type
//! for any visualizer (but not the default params)
//! Useful for application developers.
//! \param[in] pType ParamsBase TypeId of the params class.
//! \param[out] vector<Params*> vector of all params of the specified type
//! \retval int indicates size of result vector.
	int GetAllParamsInstances(string tag, vector<Params*>&) const ;
		
//! method that deletes all the Params instances for a particular visualizer
//! of any type.
//! \param[in] viznum window number associated with the visualizer
//! \retval number of Params instances that were deleted.
	int DeleteVisualizerParams(int viznum);

//! Determine if a tag is a valid Params tag
//! \params[in] tag to be checked
//! \return true if tag is a params tag
	bool IsParamsTag(const string&tag) const {return (GetTypeFromTag(tag) > 0);}

//!
//! method for converting a ParamsBase typeID to a Tag
//! \retval string Tag (Name) associated with ParamsBase TypeID, empty string if typeID invalid
//!
const string GetTagFromType(ParamsBase::ParamsBaseType t) const;


//! method that produces a vector of all the Params instances 
//! for a particular visualizer,
//! based on the XML Tag of the Params class.
//! Useful for application developers.
//! \param[in] tag XML tag associated with Params class
//! \param[in] winnum index of specified visualizer window
//! \retval vector of the Params pointers associated with the window 
	vector<Params*>& GetAllParamsInstances(const std::string tag, int winnum) {
		return GetAllParamsInstances(GetTypeFromTag(tag),winnum);
	}


//!
//! method for constructing a default instance of a ParamsBase 
//! class based on the Tag.
//! \param[in] tag XML tag of the ParamsBase instance to be created.
//! \retval instance newly created ParamsBase instance, or NULL on failure
//!
ParamsBase* CreateDefaultParamsBase(const string&tag);
//! method for registering a tag for an already registered ParamsBaseClass.
//! This is needed for backwards compatibility when a tag is changed.
//! The class must first be registered with the new tag.
//! \param[in] tag  Tag of class to be registered
//! \param[in] newtag  Previously registered tag (new name of class)
//! \param[in] isParams set true if the ParamsBase class is derived from Params
//! \retval classID Returns the ParamsBaseClassId, or 0 on failure 
//!
	int ReregisterParamsBaseClass(const string& tag, const string& newtag, bool isParams);
	


//! Copy the color bar settings from one RenderParams to all other RenderParams with ColorBars
//! \param[in] rp RenderParams instance which will have its colorbar settings copied
	void CopyColorBarSettings(RenderParams* rp);

#ifndef DOXYGEN_SKIP_THIS
private:
//! method that identifies the instance that is current in the identified window.

#endif //DOXYGEN_SKIP_THIS
};
#endif

}; //End namespace VAPoR
#endif //PARAMSMGR_H 
