#ifndef _UnstructuredGrid2D_
#define _UnstructuredGrid2D_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/UnstructuredGrid.h>
#include <vapor/UnstructuredGridCoordless.h>
#include <vapor/QuadTreeRectangle.hpp>

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
                       size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                       const UnstructuredGridCoordless &zug, std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr);

    UnstructuredGrid2D() = default;
    virtual ~UnstructuredGrid2D()
    {
        if (_qtr) _qtr = nullptr;
    }

    std::shared_ptr<const QuadTreeRectangle<float, size_t>> GetQuadTreeRectangle() const { return (_qtr); }

    virtual std::vector<size_t> GetCoordDimensions(size_t dim) const override;

    virtual size_t GetGeometryDim() const override;

    static std::string GetClassType() { return ("Unstructured2D"); }
    std::string        GetType() const override { return (GetClassType()); }

    virtual void GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const override;

    bool GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const override;

    virtual void GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const override;

    bool GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const override
    {
        std::vector<double>              lambda;
        std::vector<std::vector<size_t>> nodes;
        return (GetIndicesCell(coords, indices, nodes, lambda));
    }
    // For grandparent inheritance of
    // Grid::GetIndicesCell(const double coords[3], size_t indices[3])
    //
    using Grid::GetIndicesCell;

    //! \copydoc Grid::GetIndicesCell()
    //!
    //! \param[out] nodes Indices of vertices of the cell identified by \p indices
    //! \param[out] lambda Interpolation weights that may be applied to values
    //! at nodes identified by \p nodes
    //!
    bool GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices, std::vector<std::vector<size_t>> &nodes, std::vector<double> &lambda) const;

    bool InsideGrid(const DblArr3 &coords) const override;

    float GetValueNearestNeighbor(const DblArr3 &coords) const override;

    float GetValueLinear(const DblArr3 &coords) const override;

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

protected:
    virtual void GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const override;

private:
    UnstructuredGridCoordless                               _xug;
    UnstructuredGridCoordless                               _yug;
    UnstructuredGridCoordless                               _zug;
    std::shared_ptr<const QuadTreeRectangle<float, size_t>> _qtr;

    bool _insideGrid(const DblArr3 &coords, size_t &face, std::vector<size_t> &nodes, double *lambda, int &nlambda) const;

    bool _insideGridNodeCentered(const DblArr3 &coords, size_t &face, std::vector<size_t> &nodes, double *lambda, int &nlambda) const;

    bool _insideGridFaceCentered(const DblArr3 &coords, size_t &face, std::vector<size_t> &nodes, double *lambda, int &nlambda) const;

    bool _pointInsideBoundingRectangle(const double pt[], const double verts[], int n) const;

    bool _insideFace(size_t face, double pt[2], std::vector<size_t> &node_indices, double *lambda, int &nlambda) const;

    std::shared_ptr<QuadTreeRectangle<float, size_t>> _makeQuadTreeRectangle() const;
};
};    // namespace VAPoR

#endif
