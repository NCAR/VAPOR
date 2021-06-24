#ifndef _KDTreeRG_
#define _KDTreeRG_

#include <ostream>
#include <vector>
#include <numeric>       // std::accumulate
#include <functional>    // std::multiplies<>
#include <vapor/Grid.h>

#include "nanoflann.hpp"

struct kdtree;

namespace VAPoR {
//
//! \class KDTreeRG
//! \brief This class implements a k-d space partitioning tree.
//!
//! This class provides an object-oriented interface to kdtree.c, which
//! implements a k-d space partitioning tree.
//!
//! \sa https://en.wikipedia.org/wiki/K-d_tree
//
class VDF_API KDTreeRG {
public:
    //! Construct a 2D k-d tree for a structured grid
    //!
    //! Creates a 2D k-d space partitioning tree for a structured grid
    //! defined by a pair of Grid instances. The data values of  the
    //! \p xg and \p yg
    //! Grid instances provide the coordinates of all of the points
    //! to be inserted into the k-d tree.
    //!
    //! \param[in] xg A Grid instance giving the X user coordinates
    //! for each point in the k-d tree.
    //! \param[in] yg A Grid instance giving the Y user coordinates
    //! for each point in the k-d tree. The \p xg and \p yg Grid
    //! instances must have identical configurations, differing only in their
    //! data values.
    //!
    //! \sa Grid()
    //
    KDTreeRG(const Grid &xg, const Grid &yg);

    //! Construct a 3D k-d tree for a structured grid
    //!
    //! Creates a 3D k-d space partitioning tree for a structured grid
    //!
    //! \param[in] xg A Grid instance giving the X user coordinates
    //! for each point in the k-d tree.
    //! \param[in] yg A Grid instance giving the Y user coordinates
    //! for each point in the k-d tree.
    //! \param[in] zg A Grid instance giving the Z user coordinates
    //! for each point in the k-d tree. The \p xg, \p yg, and \p yg Grid
    //! instances must have identical configurations, differing only in their
    //! data values.
    //!
    //! \sa KDTreeRG(const Grid, const Grid)
    //
    // KDTreeRG( const Grid &xg, const Grid &yg, const Grid &zg );

    virtual ~KDTreeRG();

    //! Return indecies of nearest point
    //!
    //! This method returns the \a ijk indeces of the grid vertex nearest, by
    //! measure of Cartesian distance, a specified point. The returned
    //! indeces may be used to access the \p xg, \p yg, and \p zg
    //! Grid instances passed into the constructor.
    //!
    //! \param[in] coordu A 2D or 3D vector of user coordinates specifying
    //! the location of a point in space.
    //!
    //! \param[out] index The \a ijk indecies of the grid vertex nearest
    //! \p coordu.
    //
    void Nearest(const std::vector<float> &coordu, std::vector<size_t> &index) const;

    void Nearest(const std::vector<double> &coordu, std::vector<size_t> &index) const
    {
        std::vector<float> coordu_f;
        for (int i = 0; i < coordu.size(); i++) coordu_f.push_back(coordu[i]);
        this->Nearest(coordu_f, index);
    }

    //! Returns the dimesionality of the structured grids passed to the
    //! constructor.
    //!
    //! This method returns a two or three element vector containing the
    //! dimensions of the Grid class instances passed to the
    //! constructor.
    //!
    //! \retval vector
    std::vector<size_t> GetDimensions() const { return (_dims); }

private:
    class PointCloud2D {
    public:
        // Constructor
        PointCloud2D(const Grid &xg, const Grid &yg)
        {
            VAssert(xg.GetDimensions() == yg.GetDimensions());
            VAssert(xg.GetNumDimensions() <= 2);

            // number of elements
            auto   dims = xg.GetDimensions();
            size_t nelem = std::accumulate(dims.begin(), dims.end(), 1ul, std::multiplies<size_t>());
            this->X.resize(nelem);
            this->Y.resize(nelem);

            // Store the point coordinates in the k-d tree
            Grid::ConstIterator xitr = xg.cbegin();
            Grid::ConstIterator yitr = yg.cbegin();

            for (size_t i = 0; i < nelem; ++i, ++xitr, ++yitr) {
                this->X[i] = *xitr;
                this->Y[i] = *yitr;
            }
        }    // end of the Constructor

        // Must return the number of data points
        inline size_t kdtree_get_point_count() const
        {
            VAssert(X.size() == Y.size());
            return X.size();
        }

        // Returns the dim'th component of the idx'th point in the class:
        // Since this is inlined and the "dim" argument is typically an immediate value, the
        //  "if/else's" are actually solved at compile time.
        inline float kdtree_get_pt(const size_t idx, int dim) const
        {
            if (dim == 0)
                return X[idx];
            else
                return Y[idx];
        }

        // Optional bounding-box computation: return false to default to a standard bbox computation loop.
        //   Return true if the BBOX was already computed by the class and returned in "bb"
        //   so it can be avoided to redo it again.
        //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
        template<class BBOX> bool kdtree_get_bbox(BBOX & /* bb */) const { return false; }

    private:
        std::vector<float> X, Y;

    };    // end of class PointCloud2D

    typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, PointCloud2D>, PointCloud2D, 2 /* dimension */> KDTreeType;

    PointCloud2D        _points;
    KDTreeType          _kdtree;
    std::vector<size_t> _dims;
};    // end of class KDTreeRG.

//! class KDTreeRGSubset
//! \brief This class implements a k-d tree for a structured grid over
//! a reduced region-of-interest (ROI)
//!
//! This class provides a k-d tree for a structured grid over an ROI. The
//! class constructor is passed a pointer to a KDTreeRG instance that
//! defines a k-d tree over a structured grid. This class can be used
//! to cull out points outside of an axis-aligned region of interest
//! specified by \p min and \p max. The advantage of using this class
//! over simply creating a new instance of KDTreeRG is the avoidance
//! of the cost of rebuilding the k-d tree from scratch.
//!
class VDF_API KDTreeRGSubset {
public:
    KDTreeRGSubset();

    //! Construct a KDTreeRGSubset instance.
    //!
    //! Construct a KDTreeRGSubset instance.
    //!
    //! \param[in] kdtreerg A pointer to a KDTreeRG instance. The pointer
    //! is shallow copied and the referenced contents should remain valid
    //! until this class instance is destroyed.
    //!
    //! \param[in] min A two or three element vector of \a ijk coordinate indeces
    //! of the first grid point defined by \a xg, \a yg, and \a zg.
    //! \param[in] max A two or three element vector of \a ijk coordinate indeces
    //! of the last grid point defined by \a xg, \a yg, and \a zg.
    //!
    //! \note \p kdtreerg is shallow copied and the referenced contents
    //! should remain valid
    //! until this class instance is destroyed.
    //
    KDTreeRGSubset(const KDTreeRG *kdtreerg, const std::vector<size_t> &min, const std::vector<size_t> &max);
    ~KDTreeRGSubset() {}

    //! Return indecies of nearest point
    //!
    //! This method returns the \a ijk index of the grid vertex nearest, by
    //! measure of Cartesian distance, a specified point. The returned
    //! indecies may be used to access the \p xg, \p yg, and \p zg
    //! Grid instances passed into the constructor used to
    //! create \p kdtreerg.
    //!
    //! \param[in] coordu A 2D or 3D vector of user coordinates specifying
    //! the location of a point in space.
    //!
    //! \param[out] index The \a ijk indecies of the grid vertex nearest
    //! \p coordu.
    //
    void Nearest(const std::vector<float> &coordu, std::vector<size_t> &coord) const;

    void Nearest(const std::vector<double> &coordu, std::vector<size_t> &index) const
    {
        std::vector<float> coordu_f;
        for (int i = 0; i < coordu.size(); i++) coordu_f.push_back(coordu[i]);
        Nearest(coordu_f, index);
    }

    std::vector<size_t> GetDimensions() const
    {
        std::vector<std::size_t> dims;
        for (int i = 0; i < _min.size(); i++) { dims.push_back(_max[i] - _min[i] + 1); }
        return (dims);
    }

private:
    const KDTreeRG *    _kdtree;
    std::vector<size_t> _min;
    std::vector<size_t> _max;
};

};    // namespace VAPoR

#endif
