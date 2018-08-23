#ifndef VIEWPOINT_H
#define VIEWPOINT_H
/*
 * This class describes a viewpoint
 */
#include <vapor/ParamsBase.h>

namespace VAPoR {

//! \class Viewpoint
//! \ingroup Public_Params
//! \brief class that indicates location and direction of view
//! \author Alan Norton
//! \version 3.0
//! \date February 2014

//! \par
//! This class contains all the parameters associated with viewpoint, including
//! camera position, direction, and rotation center.  Most of its methods
//! should be accessed through the ViewpointParams class.

class PARAMS_API Viewpoint : public ParamsBase {
public:
    Viewpoint(ParamsBase::StateSave *ssave);

    Viewpoint(ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~Viewpoint();

    //! Return the current 4x4 model-view matrix
    //
    void GetModelViewMatrix(double m[16]) const;
    void SetModelViewMatrix(const double m[16]);

    void GetProjectionMatrix(double m[16]) const;
    void SetProjectionMatrix(const double m[16]);

    bool ReconstructCamera(const double m[16], double position[3], double upVec[3], double viewDir[3]) const;

    void SetRotationCenter(const std::vector<double> &v)
    {
        assert(v.size() == 3);
        SetValueDoubleVec(_rotationCenterTag, "Camera rotation center", v);
    }

    std::vector<double> GetRotationCenter() const
    {
        vector<double> defaultv(3, 0.0);
        return (GetValueDoubleVec(_rotationCenterTag, defaultv));
    }

    static string GetClassType() { return ("Viewpoint"); }

private:
    // ParamsContainer *_VPs;

    static const string _modelViewMatrixTag;
    static const string _projectionMatrixTag;
    static const string _rotationCenterTag;

    static double _defaultModelViewMatrix[16];
    static double _defaultProjectionMatrix[16];

    void _init();
};
};    // namespace VAPoR

#endif    // VIEWPOINT_H
