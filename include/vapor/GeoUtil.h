
//      $Id$
//

#ifndef _GeoUtil_h_
#define _GeoUtil_h_

#include <vector>
#include <vapor/MyBase.h>

namespace VAPoR {

//
//! \class GeoUtil
//! \brief Misc. utilities for operating on geographic coordinates
//!
//!
//! \author John Clyne
//!
class VDF_API GeoUtil : public Wasp::MyBase {
  public:
    static void ShiftLon(
        const float *srclon, int nx, float *dstlon);
    static void ShiftLon(
        const double *srclon, int nx, double *dstlon);
    static void ShiftLon(
        const float *srclon, int nx, int ny, float *dstlon);
    static void ShiftLon(
        const double *srclon, int nx, int ny, double *dstlon);

    //! Calculate west-most and east-most extents for a grid of
    //! longitudinal values
    //!
    //! \note If the input values wrap (cross 0 if coordinates run from 0 to 360,
    //! or cross 180 if coordinates run from -180 to 180) the returned
    //! values may be shifted by 360 degrees such that \p lonwest < \p loneast
    //!
    //! \param[in] lon	A structured 2D grid of longitude coordintes in degrees
    //! \param[in] nx	dimension of fastest moving coordinate
    //! \param[in] ny	dimension of slowest moving coordinate
    //! \param[out] lonwest west-most longitude coordinate
    //! \param[out] loneast east-most longitude coordinate
    //
    static void LonExtents(
        const float *lon, int nx, int ny, float &lonwest, float &loneast);
    static void LonExtents(
        const double *lon, int nx, int ny, double &lonwest, double &loneast);

    //! Calculate west-most and east-most extents for a grid of
    //! longitudinal values
    //!
    //! \note If the input values wrap (cross 0 if coordinates run from 0 to 360,
    //! or cross 180 if coordinates run from -180 to 180) the returned
    //! values may be shifted by 360 degrees such that \p lonwest < \p loneast
    //!
    //! \param[in] lon	A 1D grid of longitude coordintes in degrees
    //! \param[in] nx	dimension of grid
    //! \param[out] lonwest west-most longitude coordinate
    //! \param[out] loneast east-most longitude coordinate
    //
    static void LonExtents(
        const float *lon, int nx, float &lonwest, float &loneast);
    static void LonExtents(
        const double *lon, int nx, double &lonwest, double &loneast);

    //! Calculate south-most and nort-most extents for a grid of
    //! latitude values
    //!
    //! \note Assumes valid coordinates are in the range -180 to 180
    //!
    //! \param[in] lon	A structured 2D grid of latitude coordintes in degrees
    //! \param[in] nx	dimension of fastest moving coordinate
    //! \param[in] ny	dimension of slowest moving coordinate
    //! \param[out] latsouth south-most latitude coordinate
    //! \param[out] latnorth north-most latitude coordinate
    //
    static void LatExtents(
        const float *lon, int nx, int ny, float &latsouth, float &latnorth);
    static void LatExtents(
        const double *lon, int nx, int ny, double &latsouth, double &latnorth);

    static void LatExtents(
        const float *lon, int ny, float &latsouth, float &latnorth);
    static void LatExtents(
        const double *lon, int ny, double &latsouth, double &latnorth);

    //! Extract boundary points from a 2D grid
    //!
    //! This method walks a 2D array, \p a, in counter-clockwise order, visiting
    //! each boundary grid point exactly once, copying the value
    //! to the array \p bdry. A total of 2*nx + 2*ny - 4 grid points are
    //! copied.
    //!
    //! \param[in] a An 2D array dimensioned \p nx by \p ny
    //! \param[in] nx	dimension of fastest moving coordinate
    //! \param[in] ny	dimension of slowest moving coordinate
    //! \param[output] bdry Output array containing the boundary values
    //! of \p a. The number of elements copied to \p bdry is
    //! 2 * \p nx + 2 * \p ny - 4.
    //
    static void ExtractBoundary(
        const float *a, int nx, int ny, float *bdry);
    static void ExtractBoundary(
        const double *a, int nx, int ny, double *bdry);

  private:
};
}; // namespace VAPoR

#endif //	_GeoUtil_h_
