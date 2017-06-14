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

    //! Get current camera position
    //! \param[out] position 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraPos(double position[3]) const {
        double upVec[3], viewDir[3];
        reconstructCamera(position, upVec, viewDir);
    }

    //! Get current camera normalized view direction
    //! \param[out] viewDir 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraViewDir(double viewDir[3]) const {
        double upVec[3], position[3];
        reconstructCamera(position, upVec, viewDir);
    }

    //! Get current camera normalized up direction
    //! \param[out] upVec 3-element vector with camera position in
    //! world coordinates
    //
    void GetCameraUpVec(double upVec[3]) const {
        double viewDir[3], position[3];
        reconstructCamera(position, upVec, viewDir);
    }

    //! Obtain rotation center in local coordinates
    //! \param[out] center 3-element vector with center of rotation
    //
    void GetRotationCenter(double center[3]) const {
        vector<double> defaultv(3, 0.0);
        vector<double> v = GetValueDoubleVec(_rotCenterTag, defaultv);
        assert(v.size() == 3);
        center[0] = v[0];
        center[1] = v[1];
        center[2] = v[2];
    }

    //! Specify rotation center in local coordinates
    //! \param[in] vector<double>& rotation center in local coordinates
    //! \retval int 0 if successful
    void SetRotationCenter(const double center[3]) {
        vector<double> val;
        val.push_back(center[0]);
        val.push_back(center[1]);
        val.push_back(center[2]);
        SetValueDoubleVec(_rotCenterTag, "Set rotation center", val);
    }

    //! Return the current 4x4 model-view matrix
    //
    void GetModelViewMatrix(double m[16]) const;
    void SetModelViewMatrix(const double m[16]);

    void GetProjectionMatrix(double m[16]) const;
    void SetProjectionMatrix(const double m[16]);

    static string GetClassType() {
        return ("Viewpoint");
    }

  private:
    //ParamsContainer *m_VPs;

    static const string _rotCenterTag;
    static const string m_modelViewMatrixTag;
    static const string m_projectionMatrixTag;

    static double m_defaultModelViewMatrix[16];
    static double m_defaultProjectionMatrix[16];

    void _init();

    void reconstructCamera(
        double position[3], double upVec[3], double viewDir[3]) const;
};
}; // namespace VAPoR

#endif //VIEWPOINT_H
