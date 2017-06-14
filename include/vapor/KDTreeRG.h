#ifndef _KDTreeRG_
#define _KDTreeRG_

#include <ostream>
#include <vector>
#include <vapor/RegularGrid.h>

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
    KDTreeRG();

    //! Construct a 2D k-d tree for a structured grid
    //!
    //! Creates a 2D k-d space partitioning tree for a structured grid
    //! defined by a pair of RegularGrid instances. The data values of  the
    //! \p xrg and \p yrg
    //! RegularGrid instances provide the coordinates of all of the points
    //! to be inserted into the k-d tree.
    //!
    //! \param[in] xrg A RegularGrid instance giving the X user coordinates
    //! for each point in the k-d tree.
    //! \param[in] yrg A RegularGrid instance giving the Y user coordinates
    //! for each point in the k-d tree. The \p xrg and \p yrg RegularGrid
    //! instances must have identical configurations, differing only in their
    //! data values.
    //!
    //! \sa RegularGrid()
    //
    KDTreeRG(
        const RegularGrid &xrg,
        const RegularGrid &yrg);

    //! Construct a 3D k-d tree for a structured grid
    //!
    //! Creates a 3D k-d space partitioning tree for a structured grid
    //!
    //! \param[in] xrg A RegularGrid instance giving the X user coordinates
    //! for each point in the k-d tree.
    //! \param[in] yrg A RegularGrid instance giving the Y user coordinates
    //! for each point in the k-d tree.
    //! \param[in] zrg A RegularGrid instance giving the Z user coordinates
    //! for each point in the k-d tree. The \p xrg, \p yrg, and \p yrg RegularGrid
    //! instances must have identical configurations, differing only in their
    //! data values.
    //!
    //! \sa KDTreeRG(const RegularGrid, const RegularGrid)
    //
    KDTreeRG(
        const RegularGrid &xrg,
        const RegularGrid &yrg,
        const RegularGrid &zrg);
    virtual ~KDTreeRG();

    //! Return indecies of nearest point
    //!
    //! This method returns the \a ijk indeces of the grid vertex nearest, by
    //! measure of Cartesian distance, a specified point. The returned
    //! indeces may be used to access the \p xrg, \p yrg, and \p zrg
    //! RegularGrid instances passed into the constructor.
    //!
    //! \param[in] coordu A 2D or 3D vector of user coordinates specifying
    //! the location of a point in space.
    //!
    //! \param[out] index The \a ijk indecies of the grid vertex nearest
    //! \p coordu.
    //
    void Nearest(
        const std::vector<float> &coordu, std::vector<size_t> &index) const;

    //! Returns the dimesionality of the structured grids passed to the
    //! constructor.
    //!
    //! This method returns a two or three element vector containing the
    //! dimensions of the RegularGrid class instances passed to the
    //! constructor.
    //!
    //! \retval vector
    std::vector<size_t> GetDimensions() const {
        return (_dims);
    }

  private:
    kdtree *_kdtree;
    size_t *_offsets;
    std::vector<size_t> _dims;
};

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
    //! of the first grid point defined by \a xrg, \a yrg, and \a zrg.
    //! \param[in] max A two or three element vector of \a ijk coordinate indeces
    //! of the last grid point defined by \a xrg, \a yrg, and \a zrg.
    //!
    //! \note \p kdtreerg is shallow copied and the referenced contents
    //! should remain valid
    //! until this class instance is destroyed.
    //
    KDTreeRGSubset(
        const KDTreeRG *kdtreerg,
        const std::vector<size_t> &min,
        const std::vector<size_t> &max);
    ~KDTreeRGSubset() {}

    //! Return indecies of nearest point
    //!
    //! This method returns the \a ijk index of the grid vertex nearest, by
    //! measure of Cartesian distance, a specified point. The returned
    //! indecies may be used to access the \p xrg, \p yrg, and \p zrg
    //! RegularGrid instances passed into the constructor used to
    //! create \p kdtreerg.
    //!
    //! \param[in] coordu A 2D or 3D vector of user coordinates specifying
    //! the location of a point in space.
    //!
    //! \param[out] index The \a ijk indecies of the grid vertex nearest
    //! \p coordu.
    //
    void Nearest(
        const std::vector<float> &coordu, std::vector<size_t> &coord) const;

  private:
    const KDTreeRG *_kdtree;
    std::vector<size_t> _min;
    std::vector<size_t> _max;
};

}; // namespace VAPoR

#endif
