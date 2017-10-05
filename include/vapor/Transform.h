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
    Transform(ParamsBase::StateSave *ssave);

    Transform(ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~Transform();

    vector<double> GetRotations() const
    {
        vector<double> rotation = GetValueDoubleVec(_rotationTag, _defaultRotation);
        return rotation;
    }

    void SetRotations(const vector<double> rotation) { SetValueDoubleVec(_rotationTag, "Set dataset rotation", rotation); }

    vector<double> GetTranslations() const
    {
        vector<double> translation = GetValueDoubleVec(_translationTag, _defaultTranslation);
        return translation;
    }

    void SetTranslations(const vector<double> translation) { SetValueDoubleVec(_translationTag, "Set dataset translation", translation); }

    vector<double> GetScales() const
    {
        vector<double> scale = GetValueDoubleVec(_scaleTag, _defaultScale);
        return scale;
    }

    void SetScales(const vector<double> scale) { SetValueDoubleVec(_scaleTag, "Set dataset scale", scale); }

    static string GetClassType() { return ("Transform"); }

private:
    static const string _translationTag;
    static const string _rotationTag;
    static const string _scaleTag;

    vector<double> _defaultTranslation;
    vector<double> _defaultRotation;
    vector<double> _defaultScale;

    void _init();
};
};    // namespace VAPoR

#endif    // TRANSFORM_H
