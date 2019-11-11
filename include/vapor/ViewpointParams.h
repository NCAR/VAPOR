//************************************************************************
//         *
//       Copyright (C)  2004    *
//     University Corporation for Atmospheric Research   *
//       All Rights Reserved    *
//         *
//************************************************************************/
//
// File:  ViewpointParams.h
//
// Author:  Alan Norton
//   National Center for Atmospheric Research
//   PO 3000, Boulder, Colorado
//
// Date:  August 2004
//
// Description: Defines the ViewpointParams class
//  This class contains the parameters associated with viewpoint and lights
//
#ifndef VIEWPOINTPARAMS_H
#define VIEWPOINTPARAMS_H

#include <vapor/ParamsBase.h>
#include <vapor/Viewpoint.h>
#include <vapor/Transform.h>

namespace VAPoR {

//! \class ViewpointParams
//! \ingroup Public_Params
//! \brief A class for describing the viewpoint and lights in a 3D VAPOR scene
//! \author Alan Norton
//! \version 3.0
//! \date    February 2014

//! This class describes the state of viewpoints and
//! and the properties of light sources.  If the ViewpointParams is shared (i.e. not local), all windows can
//! use the same viewpoint and lights.  Local viewpoints are
//! just applicable in one visualizer.
class PARAMS_API ViewpointParams : public ParamsBase {
public:
    ViewpointParams(ParamsBase::StateSave *ssave);

    ViewpointParams(ParamsBase::StateSave *ssave, XmlNode *node);

    ViewpointParams(const ViewpointParams &rhs);

    ViewpointParams &operator=(const ViewpointParams &rhs);

    virtual ~ViewpointParams();

    //! Rescale viewing parameters, e.g. when the scene stretch factors change
    //! \param[in] vector<double> scaleFac scale factors to be applied, relative to previous scaling.
    void rescale(vector<double> scaleFac);

    //! This method tells how many lights are specified and whether
    //! lighting is on or not (i.e. if there are more than 0 lights).
    //! Note that only the first (light 0) is used in DVR and Isosurface rendering.
    //! \retval int number of lights (0,1,2)
    int getNumLights() const
    {
        size_t n = (size_t)GetValueLong(_numLightsTag, _defaultNumLights);
        if (n > 2) n = 2;
        return (n);
    }

    //! Obtain the current specular exponent.
    //! This value should be used in setting the material properties
    //! of all geometry being rendered.
    //! \retval float Specular exponent
    double getExponent() const { return (GetValueDouble(_specularExpTag, _defaultSpecularExp)); }

    //! Set the number of directional light sources
    //! \param[in] int number of lights (0,1,2)
    //! \retval 0 on success
    void setNumLights(size_t nlights)
    {
        if (nlights > 2) nlights = 2;
        SetValueLong(_numLightsTag, "Set number of lights", nlights);
    }

    //! get one component of a light direction vector
    //! \param[in] int lightNum identifies which light source
    //! \param[in] int dir coordinate of direction vector (0..3)
    //! \retval double requested component of light direction vector
    //
    double getLightDirection(int lightNum, int dir) const;

    //! Set one component of a light direction vector
    //! \param[in] int lightNum identifies which light source
    //! \param[in] int dir coordinate of direction vector (0..3)
    //! \param[in] double value to be set
    //! \retval int 0 on success
    void setLightDirection(int lightNum, int dir, double val);

    //! Optain the diffuse lighting coefficient of a light source
    //! \param[in] int light number (0..2)
    //! \retval double diffuse coefficient
    double getDiffuseCoeff(int lightNum) const;

    //! Optain the specular lighting coefficient of a light source
    //! \param[in] int light number (0..2)
    //! \retval double specular coefficient
    double getSpecularCoeff(int lightNum) const;

    //! Optain the ambient lighting coefficient of the lights
    //! \retval double ambient coefficient
    double getAmbientCoeff() const { return GetValueDouble(_ambientCoeffTag, _defaultAmbientCoeff); }

    //! Set the diffuse lighting coefficient of a light source
    //! \param[in] int light number (0..2)
    //! \param[in] double diffuse coefficent
    //! \retval int 0 if successful
    void setDiffuseCoeff(int lightNum, double val);

    //! Set the specular lighting coefficient of a light source
    //! \param[in] int light number (0..2)
    //! \param[in] double specular coefficent
    //! \retval int 0 if successful
    void setSpecularCoeff(int lightNum, double val);

    //! Set the specular lighting exponent of all light sources
    //! \param[in] double specular exponent
    //! \retval int 0 if successful
    void setExponent(double val) { SetValueDouble(_specularExpTag, "Set specular lighting", val); }

    //! Set the ambient lighting coefficient
    //! \param[in] double ambient coefficient
    //! \retval int 0 if successful
    void setAmbientCoeff(double val) { SetValueDouble(_ambientCoeffTag, "Set ambient lighting", val); }

    //! Set the current viewpoint to another viewpoint
    //! \param[in] Viewpoint* viewpoint to be set
    //! \retval int 0 if successful
    //! \sa Viewpoint
    void SetCurrentViewpoint(Viewpoint *newVP);

    //! Set widow width and height
    //!
    //! \param[in] width width of window in pixels
    //! \param[in] height height of window in pixels
    //!
    void SetWindowSize(size_t width, size_t height);

    //! Get widow width and height
    //!
    //! \param[out] width width of window in pixels
    //! \param[out] height height of window in pixels
    //!
    void GetWindowSize(size_t &width, size_t &height) const;

    void   SetFOV(float v);
    double GetFOV() const;
    void   SetOrthoProjectionSize(float f);
    double GetOrthoProjectionSize() const;

    enum ProjectionType { Perspective, Orthographic, MapOrthographic };
    void           SetProjectionType(ProjectionType type);
    ProjectionType GetProjectionType() const;

    //! Method to get stretch factors
    vector<double> GetStretchFactors() const;

    //! method to set stretch factors
    //! \param[in] factors 3-vector of stretch factors
    void SetStretchFactors(vector<double> factors);

    //! Obtain the current viewpoint
    //! \sa Viewpoint
    //! \retval Viewpoint* current viewpoint.
    virtual Viewpoint *getCurrentViewpoint() const
    {
        Viewpoint *v = (Viewpoint *)m_VPs->GetParams(_currentViewTag);
        VAssert(v != NULL);
        return (v);
    }

    //! Return the current 4x4 model-view matrix
    //
    void                GetModelViewMatrix(double m[16]) const { getCurrentViewpoint()->GetModelViewMatrix(m); }
    std::vector<double> GetModelViewMatrix() const
    {
        double m[16];
        getCurrentViewpoint()->GetModelViewMatrix(m);
        vector<double> vec(m, m + sizeof(m) / sizeof(m[0]));
        return (vec);
    }
    void SetModelViewMatrix(const double matrix[16]) { getCurrentViewpoint()->SetModelViewMatrix(matrix); }
    void SetModelViewMatrix(const std::vector<double> mvec)
    {
        VAssert(mvec.size() == 16);
        double m[16];
        for (int i = 0; i < mvec.size(); i++) m[i] = mvec[i];
        getCurrentViewpoint()->SetModelViewMatrix(m);
    }

    void GetProjectionMatrix(double m[16]) const { getCurrentViewpoint()->GetProjectionMatrix(m); }
    void SetProjectionMatrix(const double m[16]) { getCurrentViewpoint()->SetProjectionMatrix(m); }

    bool ReconstructCamera(const double m[16], double position[3], double upVec[3], double viewDir[3]) const { return (getCurrentViewpoint()->ReconstructCamera(m, position, upVec, viewDir)); }

    std::vector<double> GetRotationCenter() const { return (getCurrentViewpoint()->GetRotationCenter()); }

    void GetRotationCenter(double c[3]) const
    {
        vector<double> val = GetRotationCenter();
        VAssert(val.size() == 3);
        for (int i = 0; i < val.size(); i++) c[i] = val[i];
    }

    void SetRotationCenter(vector<double> c) { getCurrentViewpoint()->SetRotationCenter(c); }
    void SetRotationCenter(const double c[3])
    {
        std::vector<double> vec = {c[0], c[1], c[2]};
        SetRotationCenter(vec);
    }

    //! Access the transform for a data set
    //!
    //! Access the transform for data set \p dataSetName
    //!
    //! \retval Returns a new transform if one does not exist
    //
    virtual Transform *GetTransform(string dataSetName);

    //! Return list of transform names.
    //!
    //! Return the list of transform names added with AddDatasetTransform()
    //
    vector<string> GetTransformNames() const { return (_transforms->GetNames()); }

#ifdef VAPOR3_0_0_ALPHA
    //! Determine the current diameter of the visible scene.
    //! Calculated as 2*Dist*tan(theta*.5) where theta is the camera angle,
    //! Dist is the distance from the camera to the rotation center.
    double GetCurrentViewDiameter(vector<double> stretchFactors) const;
#endif

#ifndef DOXYGEN_SKIP_THIS

    static const double *getDefaultLightDirection(int lightNum) { return _defaultLightDirection[lightNum]; }
    static double        getDefaultAmbientCoeff() { return _defaultAmbientCoeff; }
    static double        getDefaultSpecularExp() { return _defaultSpecularExp; }
    static int           getDefaultNumLights() { return _defaultNumLights; }
    static const double *getDefaultDiffuseCoeff() { return _defaultDiffuseCoeff; }
    static const double *getDefaultSpecularCoeff() { return _defaultSpecularCoeff; }
    static void          setDefaultLightDirection(int lightNum, double val[3])
    {
        for (int i = 0; i < 3; i++) _defaultLightDirection[lightNum][i] = val[i];
    }
    static void setDefaultSpecularCoeff(double val[3])
    {
        for (int i = 0; i < 3; i++) _defaultSpecularCoeff[i] = val[i];
    }
    static void setDefaultDiffuseCoeff(double val[3])
    {
        for (int i = 0; i < 3; i++) _defaultDiffuseCoeff[i] = val[i];
    }
    static void setDefaultAmbientCoeff(double val) { _defaultAmbientCoeff = val; }
    static void setDefaultSpecularExp(double val) { _defaultSpecularExp = val; }
    static void setDefaultNumLights(int val) { _defaultNumLights = val; }

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("ViewpointParams"); }

    static const string UseCustomFramebufferTag;
    static const string CustomFramebufferWidthTag;
    static const string CustomFramebufferHeightTag;

private:
    ParamsContainer *m_VPs;
    ParamsContainer *_transforms;

    static const string _viewPointsTag;
    static const string _transformsTag;
    static const string _currentViewTag;
    static const string _lightDirectionsTag;
    static const string _diffuseCoeffTag;
    static const string _specularCoeffTag;
    static const string _specularExpTag;
    static const string _ambientCoeffTag;
    static const string _numLightsTag;
    static const string m_windowSizeTag;
    static const string m_stretchFactorsTag;
    static const string m_fieldOfView;
    static const string _orthoProjectionSizeTag;
    static const string _projectionTypeTag;

    // defaults:
    static double _defaultLightDirection[3][4];
    static double _defaultDiffuseCoeff[3];
    static double _defaultSpecularCoeff[3];
    static double _defaultAmbientCoeff;
    static double _defaultSpecularExp;
    static int    _defaultNumLights;

    void _init();

#endif    // DOXYGEN_SKIP_THIS
};
};        // namespace VAPoR
#endif    // VIEWPOINTPARAMS_H
