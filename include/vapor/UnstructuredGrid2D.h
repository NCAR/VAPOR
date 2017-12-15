#ifndef _UnstructuredGrid2D_
#define _UnstructuredGrid2D_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/UnstructuredGrid.h>
#include <vapor/UnstructuredGridCoordless.h>
#include <vapor/KDTreeRG.h>

#ifdef WIN32
    #pragma warning(disable : 4661 4251)    // needed for template class
#endif

namespace VAPoR {

//! \class UnstructuredGrid2D
//! \brief class for a 2D unstructured grid.
//! \author John Clyne
//!
//
class VDF_API UnstructuredGrid2D : public UnstructuredGrid {
public:
    //! Construct a unstructured grid sampling 2D scalar function
    //!
    //
    UnstructuredGrid2D(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs, const std::vector<float *> &blks,
                       const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                       Location location,    // node,face, edge
                       size_t maxVertexPerFace, size_t maxFacePerVertex, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug, const UnstructuredGridCoordless &zug,
                       const KDTreeRG *kdtree);

    UnstructuredGrid2D() = default;
    virtual ~UnstructuredGrid2D() = default;

    virtual size_t GetNumCoordinates() const override;

    virtual void GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const override;

    virtual void GetBoundingBox(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<double> &minu, std::vector<double> &maxu) const override;

    void GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const override;

    void GetUserCoordinates(const std::vector<size_t> &indices, std::vector<double> &coords) const override;

    void GetIndices(const std::vector<double> &coords, std::vector<size_t> &indices) const override;

    bool GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const override;

    bool InsideGrid(const std::vector<double> &coords) const override;

    float GetValueNearestNeighbor(const std::vector<double> &coords) const override;

    float GetValueLinear(const std::vector<double> &coords) const override;

    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    class ConstCoordItrU2D : public Grid::ConstCoordItrAbstract {
    public:
        ConstCoordItrU2D(const UnstructuredGrid2D *ug, bool begin);
        ConstCoordItrU2D(const ConstCoordItrU2D &rhs);

        ConstCoordItrU2D();
        virtual ~ConstCoordItrU2D() {}

        virtual void            next();
        virtual void            next(const long &offset);
        virtual ConstCoordType &deref() const { return (_coords); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const
        {
            const ConstCoordItrU2D *itrptr = static_cast<const ConstCoordItrU2D *>(rhs);

            return (_xCoordItr == itrptr->_xCoordItr && _yCoordItr == itrptr->_yCoordItr && _zCoordItr == itrptr->_zCoordItr);
        }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrU2D(*this)); };

    private:
        int                 _ncoords;
        ConstIterator       _xCoordItr;
        ConstIterator       _yCoordItr;
        ConstIterator       _zCoordItr;
        std::vector<double> _coords;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrU2D(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrU2D(this, false))); }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGrid2D &sg);

private:
    UnstructuredGridCoordless _xug;
    UnstructuredGridCoordless _yug;
    UnstructuredGridCoordless _zug;
    const KDTreeRG *          _kdtree;

    bool _insideGrid(const std::vector<double> &coords, std::vector<size_t> &face_indices, double *lambda, int &nlambda, double zwgt[2]) const;

    bool _insideGridNodeCentered(const std::vector<double> &coords, std::vector<size_t> &face_indices, double *lambda, int &nlambda, double zwgt[2]) const;

    bool _insideGridFaceCentered(const std::vector<double> &coords, std::vector<size_t> &face_indices, double *lambda, int &nlambda, double zwgt[2]) const;

    bool _insideFace(size_t face, double pt[2], double *lambda, int &nlambda, double zwgt[2]) const;
};
};    // namespace VAPoR

#endif
