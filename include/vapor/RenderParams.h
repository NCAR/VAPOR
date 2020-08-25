//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		RenderParams.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2014
//
//	Description:	Defines the RendererParams class.
//		This is an abstract class for all the tabbed panel render params classes.
//		Supports functionality common to all the tabbed panel render params.

//
#ifndef RENDERPARAMS_H
#define RENDERPARAMS_H

#include <map>
#include <vapor/common.h>
#include <vapor/ParamsBase.h>
#include <vapor/MapperFunction.h>
#include <vapor/Box.h>
#include <vapor/ColorbarPbase.h>
#include <vapor/Transform.h>
#include <vapor/DataMgr.h>

namespace VAPoR{

//! \class RenderParams
//! \ingroup Public_Params
//! \brief A Params subclass for managing parameters used by Renderers
//! \author Alan Norton
//! \version 3.0
//! \date    February 2014
//!
class PARAMS_API RenderParams : public ParamsBase {
public: 

//! Standard RenderParams constructor.
//! \param[in] name  std::string name, can be the tag
	RenderParams(
		DataMgr *dataMgr, ParamsBase::StateSave *ssave, const string &classname,
		int maxdim = 3
	); 

	RenderParams(
		DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node,
		int maxdim = 3
	);

	RenderParams(const RenderParams &rhs);

	RenderParams &operator=( const RenderParams& rhs );

	virtual ~RenderParams();

	//! Initialize the class. 
	//!
	//! Must be called immediately after the constructor:
	//!
	//!  RenderParams(DataMgr *, ParamsBase::StateSave *, const string &, int maxdim);
	//!
	//! The results of 
	//! calling any other methods before calling Initialize() are 
	//! undefined.
	//!
	//! Subsequent calls to Initialize() after the first call are a no-op.
	//!
	//! \retval returns integer >= 0 on success, otherwise failure
	//
	virtual int Initialize();
	

	//! Determine if this params has been enabled for rendering
	//!
	//! Default is false.
	//!
	//! \retval bool true if enabled
	virtual bool IsEnabled() const {
		return ((bool) GetValueLong(_EnabledTag, (int) false));
	}

	//! Enable or disable this params for rendering
	//!
	//! This should be executed between start and end capture
	//! which provides the appropriate undo/redo support
	//! Accordingly this will not make an entry in the undo/redo queue.
	//!
	//! Default is false.
	//!
	//! \param[in] bool true to enable, false to disable.
	virtual void SetEnabled(bool val);


	//! Specify primary variable name; e.g. used in color mapping or rendering.
	//! The default is the empty string, which indicates no variable.
	//! \param[in] string varName. If "0" \p varName will be quietly 
	//! set to the empty string, "".
	//
	virtual void SetVariableName(string varName);

	//! Get the primary variable name, e.g. used in color mapping or rendering.
	//! The default is the empty string, which indicates a no variable.
	//! \retval string variable name
	string GetVariableName() const ;

	//! Specify auxiliary variable name; e.g. "Position along Flow"
	//! The default is a vector of length containing the empty string.
	//! \param[in] string varNames. If any element is "0" the element
	//! will be quietly 
	//! set to the empty string, "".
	virtual void SetAuxVariableNames(vector<string> varName);


	//! Get the auxiliary variable names, e.g. "position along flow"
	//!
	//! The default is a vector of length containing the empty string.
	//!
	//! \retval string variable name
	vector <string> GetAuxVariableNames() const ;

	//! Determine if auxiliary variable name is used
	//! \retval bool true if using auxiliary variable
	bool UseAuxVariable()  const {
		vector<string> names = GetAuxVariableNames();
		for (int i = 0; i<names.size(); i++){
			if (GetVariableName() == names[i]) return true;
		}
		return false;
	}

	//! Specify field variable names; e.g. used in flow integration
	//! can be 0 or 3 strings
	//! \param[in] string varNames. If any element is "0" the element
	//! will be quietly 
	//! set to the empty string, "".
	//
	virtual void SetFieldVariableNames(vector <string> varNames);

	//! Get the field variable names, e.g. used in flow integration.
	//! \retval vector<string> variable names. A vector of length 3 
	//! containing variable names. The default is 3 empty variable names.
	//
	vector<string> GetFieldVariableNames() const ;


	//! Virtual method sets current number of refinements of this Params.
	//! \param[in] int refinements
	//!
	virtual void SetRefinementLevel(int numrefinements);

	//! Virtual method indicates current number of refinements of this Params.
	//! \retval integer number of refinements
	//!
	virtual int GetRefinementLevel() const ;

	//! virtual method indicates current Compression level.
	//! \retval integer compression level, 0 is most compressed
	//!
	virtual int GetCompressionLevel() const ;

	//! Virtual method sets current Compression level.
	//! \param[in] val  compression level, 0 is most compressed
	//!
	virtual void SetCompressionLevel(int val);

	//! Specify a stretch factor used in displaying histograms in 
	//! mapper functions.
	//! Can be ignored if there is no mapper function in the params.
	//! Default value is 1.0.
	//!
	//! \param[in] factor positive multiplier that applies to the 
	//! histogram height.
	void SetHistoStretch(float factor);

	//! Obtain the stretch factor used in displaying histograms in mapper 
	//! functions.
	//!
	//! Default value is 1.0.
	//! \return multiplier that applies to the histogram height.
	float GetHistoStretch() const ;

	//! Obtain ColorbarPBase (used to specify Color Bar properties) from RenderParams.
	virtual ColorbarPbase* GetColorbarPbase() const {
		return _Colorbar;
	}

	//! Set the ColorbarPbase that specifies properties of the Color bar.
	//! \sa GetColorbarPbase
	//! By default the ColorbarPbase is a child of the root node of this RenderParams.
	//! \param[in] pb ColorbarPbase to set.
	//! \return 0 if successful.
	virtual void SetColorbarPbase(ColorbarPbase* pb);


	//! Obtain current MapperFunction 
	//!
	//! Get a MapperFunction for the variable \p varname.
	//! If one does not exist it is created.
	//
	virtual MapperFunction* GetMapperFunc(string varname);

	//! Remove the mapper function for the named variable.
	//
	virtual void RemoveMapperFunc(string varname);


	//! Set current MapperFunction 
	//!
	//! \param[in] varname Name of variable associated with mapping
	//! \param[in] tf
	//
	virtual void SetMapperFunc(string varname, MapperFunction* tf);

	//! Virtual method to return the Box associated with a Params class.
	//! By default returns NULL.
	//! All params classes that use a box to define data extents should reimplement this method.
	//! Needed to support manipulators.
	//! \retval Box* returns pointer to the Box associated with this Params.
	//
	virtual Box* GetBox()  const {
		return(_Box);
	}

	//! Specify the 2D cursor coordinates when associated with this RenderParams
	//! \param[in] coords
	void SetCursorCoords(const float coords[2]);
	
	//! Obtain the 2D cursor coordinates if associated with this RenderParams
	//! \return 2D cursor coordinates
	void GetCursorCoords(float coords[2]) const ;
	

	//! Specify the variable being used for height
	//! Overrides method on RenderParams
	//! \param[in] string varName. If any \p varName is "0" it
	//! will be quietly 
	//! set to the empty string, "".
	//! \retval int 0 if successful;
	virtual void SetHeightVariableName(string varname); 

	//! Determine variable name being used for terrain height (above or below sea level)
	//! \retval const string& variable name
	virtual string GetHeightVariableName() const ;
    
	//! Indicate if a single (constant) color is being used
	//! \return true if constant single color is used
    bool UseSingleColor() const;

	//! Specify the variable being used for color mapping
	//! \param[in] string varName. If any \p varName is "0" it
	//! will be quietly 
	//! set to the empty string, "".
	//
	virtual void SetColorMapVariableName(string varname); 

	//! Get the color mapping variable name if any
	//! \retval string variable name
	//!
	virtual string GetColorMapVariableName() const ;


	//! Turn on or off the use of single constant color (versus color map)
	//! \param[in] val true will enable constant color
    void SetUseSingleColor(bool val);

	//! Specify a constant color  
	//!
	//! Specify a constant color is in rgb values between 0 and 1.
	//! The constant color is used to color objects when UseSingleColor()
	//! returns true.
	//!
	//! \param[in] const float rgb[3]
	//!
	//! \sa GetConstantColor()
	//
	void SetConstantColor(const float rgb[3]);

	//! Get the constant color (in r,g,b)
	//!
	//! \param[out] rgb A 3-element array use values are in the range 0.0..1.0
	//!
	//! \sa SetConstantColor(), GetConstantOpacity(), UseSingleColor()
	//
	void GetConstantColor(float rgb[3]) const ;

	vector <float> GetConstantColor() const {
		float rgb[3];
		GetConstantColor(rgb);
		vector <float> v = {rgb[0], rgb[1], rgb[2]};
		return(v);
	}

	//! Specify a constant opacity.  Color is n the between 0 and 1.
	//!
	//! \param[in] const float rgb[3]
	//! \retval 0 if successful
	//!
	void SetConstantOpacity(float o);

	//! Get the constant opacity
	//!
	//! \retval opacity
	//!
	//! \sa SetConstantColor(), GetConstantOpacity()
	//
	float GetConstantOpacity() const ;

 //! Get the current data timestep 
 //! \retval ts current time step
 //
 size_t GetCurrentTimestep() const {
	return (size_t) GetValueLong(_currentTimestepTag, 0);
 }

 //! Set the current data timestep being used
 //! \param[in] ts current time step
 //
 void SetCurrentTimestep(size_t ts) {
	SetValueLong(_currentTimestepTag,"Set timestep", (long) ts);
 }

 //! Access the transform used by the renderer
 //
 virtual Transform* GetTransform() const {
	return _transform;
 }

	void initializeBypassFlags();

 //! Set reasonable default variables
 //! \param[in] The dimension of the variables being set
 //! \param[in] Indicates whether we're using color mapped variables
 void SetDefaultVariables(
    int dim,
    bool secondaryColormapVariable
);

 //! Return the renderer's current dimension
 //!
 //! For renderers that are only capable of operating on variables of a fixed
 //! dimensionality (e.g. 2D or 3D) this function will return a constant value:
 //! the number of dimensions. For renderers that can operate on a variable of
 //! varying dimension this method returns the current dimensionality. The
 //! returned value will be between 0 and 3. A value of zero will be
 //! returned if the current dimensionality cannot be determined.
 //!
 //!
 virtual size_t GetRenderDim() const = 0;
    
    //! This should be overriden by params for renderes that support iso values to return true.
    virtual bool HasIsoValues() const { return false; }
    virtual vector<double> GetIsoValues(const string &variable) { VAssert(0); return {}; }
    virtual void SetIsoValues(const string &variable, const vector<double> &values) { VAssert(0); }
    
    vector<double> GetIsoValues() { return GetIsoValues(GetVariableName()); }
    void SetIsoValues(const vector<double> &values) { SetIsoValues(GetVariableName(), values); }
	
protected:
	DataMgr *_dataMgr;
    int _maxDim;
    
    virtual bool GetUseSingleColorDefault() const { return false; }
	
private:

 void _init();
 void _calculateStride(string varName);
 int _stride;
 ParamsContainer *_TFs; 
 Box *_Box;
 ColorbarPbase *_Colorbar;
 Transform *_transform;
 bool _classInitialized;	//

 static const string _EnabledTag;
 static const string _histoScaleTag;
 static const string _editBoundsTag;
 static const string _histoBoundsTag;
 static const string _cursorCoordsTag;
 static const string _terrainMapTag;
 static const string _fieldVariableNamesTag;
 static const string _auxVariableNamesTag;
 static const string _constantOpacityTag;
 static const string _CompressionLevelTag;
 static const string _RefinementLevelTag;
 static const string _transferFunctionsTag;
 static const string _stretchFactorsTag;
 static const string _currentTimestepTag;

 string _findVarStartingWithLetter(
	std::vector<string> searchVars,
	char letter
 );
    
public:
    static const string _variableNameTag;
    static const string _colorMapVariableNameTag;
    static const string _heightVariableNameTag;
    static const string _useSingleColorTag;
    static const string _constantColorTag;
};


//////////////////////////////////////////////////////////////////////////
//
// RenParamsFactory Class
//
/////////////////////////////////////////////////////////////////////////


class PARAMS_API RenParamsFactory {
public:
 static RenParamsFactory *Instance() {
	static RenParamsFactory instance;
	return &instance;
 }

 void RegisterFactoryFunction(
	string name,
	function<RenderParams *(DataMgr *, ParamsBase::StateSave *,  XmlNode *)> classFactoryFunction) 
 {

	// register the class factory function
	_factoryFunctionRegistry[name] = classFactoryFunction;
 }

 RenderParams *(
	CreateInstance(string classType, DataMgr *, ParamsBase::StateSave *,  XmlNode *
 ));

 vector <string> GetFactoryNames() const;

private:
 map<string, function<RenderParams * (DataMgr *, ParamsBase::StateSave *, XmlNode *)>> 
	_factoryFunctionRegistry;

 RenParamsFactory() {}
 RenParamsFactory(const RenParamsFactory &) { }
 RenParamsFactory &operator=(const RenParamsFactory &) { return *this; }

};


//////////////////////////////////////////////////////////////////////////
//
// RenParamsRegistrar Class
//
// Register RenParamsBase derived class with:
//
//	static RenParamsRegistrar<RenParamsClass> registrar("myclassname");
//
// where 'RenParamsClass' is a class derived from 'RenderParams', and 
// "myclassname" is the name of the class
//
/////////////////////////////////////////////////////////////////////////

template<class T>
class RenParamsRegistrar {
public:
 RenParamsRegistrar(string classType) {

	// register the class factory function 
	//
	RenParamsFactory::Instance()->RegisterFactoryFunction(
		classType, [](DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) -> RenderParams * { 
			if (node) return new T(dataMgr, ssave, node);
			else return new T(dataMgr, ssave);
		}
	);
 }
};

//////////////////////////////////////////////////////////////////////////
//
// RenParamsContainer Class
//
/////////////////////////////////////////////////////////////////////////

//
// The RenParamsContainer class constructs an XML tree as depicted below,
// where 'Container Name' is the root of XML tree, and is the name
// passed into the constructor as 'myname'; 'Class Name' is the name of
// the derived RenParamBase class used to construct new instances of
// the derived class; and 'ele name x' is the unique name of the element
// contained in the container.
//
/*
            |----------------|
            | Container Name |
            |----------------|
                    |
                   \|/
            |----------------|
            |   Class Name   |
            |----------------|
                    |         \
                   \|/         \
            |----------------|  \ |----------------|
            |   ele name 1   |....|   ele name n   |
            |----------------|    |----------------|
*/
class PARAMS_API RenParamsContainer : public Wasp::MyBase {
public: 
 RenParamsContainer(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, const string &myname
 ); 

 RenParamsContainer(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
 ); 

 //! Copy constructor.  
 RenParamsContainer(const RenParamsContainer &rhs);

 RenParamsContainer &operator=( const RenParamsContainer& rhs );

 ~RenParamsContainer();

 RenderParams *Insert(const RenderParams *rp, string name) ;

 RenderParams *Create(string classType, string name) ;

 void Remove(string name) ;

 RenderParams *GetParams(string name) const;

 vector <string> GetNames() const;


 size_t Size() const {
	return(_elements.size());
 }

 XmlNode *GetNode() const {return _separator->GetNode(); }

 ParamsSeparator *GetSeparator() const {return _separator; }

 string const GetName() const {
	return(_separator->GetName());
 }

private:
 DataMgr *_dataMgr;
 ParamsBase::StateSave *_ssave; 
 ParamsSeparator *_separator;
 map <string, RenderParams *> _elements;


};

}; //End namespace VAPoR
#endif //RENDERPARAMS_H 
