
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

    //! Shift a container of longitude values so that all
    //! values are in the range [-180.0..180.0)
    //
    static void ShiftLon(vector <float>::iterator first, vector <float>::iterator last);
    static void ShiftLon(vector <double>::iterator first, vector <double>::iterator last);
    static void ShiftLon(float *first, float *last);

    //! Unwrap any wrapped longitued values
    //!
    //! Iteratively adds 360.0 to each value in a container that is less than
    //! the first value until the new value is greater than the first.
    //
    static void UnwrapLongitude(vector <float>::iterator first, vector <float>::iterator last);
    static void UnwrapLongitude(vector <double>::iterator first, vector <double>::iterator last);
    static void UnwrapLongitude(float *first, float *last);

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
    static void ExtractBoundary(const float *a, int nx, int ny, float *bdry);
    static void ExtractBoundary(const double *a, int nx, int ny, double *bdry);

private:
};
};    // namespace VAPoR

#endif    //	_GeoUtil_h_
