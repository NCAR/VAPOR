#ifndef TRANSFORM_H
#define TRANSFORM_H
/*
* This class describes a viewpoint
*/
#include <vapor/ParamsBase.h>

namespace VAPoR {

//! \class Transform
//! \ingroup Public_Params
//! \brief class that indicates location and direction of view
//! \author Scott Pearse
//! \version 3.0
//! \date October 2017

//! \par
//! This class contains all the parameters associated with dataset transforms,
//! including the scaling, rotation, and translation of that dataset.  
//! ddshould be accessed through the TransformParams class.

class PARAMS_API Transform : public ParamsBase {
 
public: 

 enum Flags {
	VIEWPOINT = (1u << 0),
	RENDERER = (1u << 1)
 };

 Transform( ParamsBase::StateSave *ssave);
    
 Transform( ParamsBase::StateSave *ssave, XmlNode *node);

 virtual ~Transform();

 vector<double> GetRotations() const {
	vector <double> defaultv(3,0.0);
	vector<double> rotation = GetValueDoubleVec(_rotationTag, 
		defaultv
	);
	return rotation;
 }

 void SetRotations(const vector<double> rotation) {
	SetValueDoubleVec(_rotationTag, "Set rotation transform", rotation);
 }
 
 vector<double> GetTranslations() const {
	vector <double> defaultv(3,0.0);
	vector<double> translation = GetValueDoubleVec(_translationTag,
		defaultv
	);
	return translation;
 }

 void SetTranslations(const vector<double> translation) {
	SetValueDoubleVec(_translationTag, "Set translation transform", translation);
 }
 
 vector<double> GetScales() const {
	vector <double> defaultv(3,1.0);
	vector<double> scale = GetValueDoubleVec(_scaleTag, defaultv);
	return scale;
 }

 void SetScales(const vector<double> scale) {
	SetValueDoubleVec(_scaleTag, "Set scale transform", scale);
 }

 vector<double> GetOrigin() const;
 void SetOrigin(const vector<double> origin);

 bool IsOriginInitialized() const;
 void SetOriginInitialized(bool value);

 static string GetClassType() {
  return("Transform");
 }

private:

 static const string _translationTag;
 static const string _rotationTag;
 static const string _scaleTag;
 static const string _originTag;
 static const string _originInitializedTag;


};
};

#endif //TRANSFORM_H 

