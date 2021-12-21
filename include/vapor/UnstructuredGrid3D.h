#pragma once

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/UnstructuredGrid2D.h>
#include <vapor/QuadTreeRectangle.hpp>


#ifdef WIN32
    #pragma warning(disable : 4661 4251)    // needed for template class
#endif

namespace VAPoR {

//! \class UnstructuredGrid3D
//! \brief class for a Layered unstructured grid.
//!
//
class VDF_API UnstructuredGrid3D : public UnstructuredGrid {
public:
    //! Construct a unstructured grid sampling Layered scalar function
    //!
    //
    UnstructuredGrid3D(const DimsType &vertexDims, const DimsType &faceDims, const DimsType &edgeDims, const DimsType &bs, const std::vector<float *> &blks, const int *vertexOnFace,
                       const int *faceOnVertex, const int *faceOnFace,
                       Location location,    // node,face, edge
                       size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                       const UnstructuredGridCoordless &zug);

    UnstructuredGrid3D(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs, const std::vector<float *> &blks,
                       const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                       Location location,    // node,face, edge
                       size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                       const UnstructuredGridCoordless &zug);

    UnstructuredGrid3D() = default;
    virtual ~UnstructuredGrid3D() = default;

    virtual std::vector<size_t> GetCoordDimensions(size_t dim) const override;

    virtual size_t GetGeometryDim() const override;

    static std::string GetClassType() { return ("Unstructured3D"); }
    std::string        GetType() const override { return (GetClassType()); }


    virtual void GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const override;
    bool         GetEnclosingRegion(const CoordType &minu, const CoordType &maxu, DimsType &min, DimsType &max) const override;
    virtual void GetUserCoordinates(const DimsType &indices, CoordType &coords) const override;
    bool         GetIndicesCell(const CoordType &coords, DimsType &indices) const override;
    bool         InsideGrid(const CoordType &coords) const override;
    float        GetValueNearestNeighbor(const CoordType &coords) const override;
    float        GetValueLinear(const CoordType &coords) const override;


    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    class ConstCoordItrU3D : public Grid::ConstCoordItrAbstract {
    public:
        ConstCoordItrU3D(const UnstructuredGrid3D *ug, bool begin);
        ConstCoordItrU3D(const ConstCoordItrU3D &rhs);


        ConstCoordItrU3D();
        virtual ~ConstCoordItrU3D() {}

        virtual void            next() override;
        virtual void            next(const long &offset) override;
        virtual ConstCoordType &deref() const override { return (_coords); }
        virtual const void *    address() const override { return this; };

        virtual bool equal(const void *rhs) const override
        {
            const ConstCoordItrU3D *itrptr = static_cast<const ConstCoordItrU3D *>(rhs);

            return (_xCoordItr == itrptr->_xCoordItr && _yCoordItr == itrptr->_yCoordItr && _zCoordItr == itrptr->_zCoordItr);
        }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const override { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrU3D(*this)); };

    private:
        const UnstructuredGrid3D *_ug;
        ConstIterator             _xCoordItr;
        ConstIterator             _yCoordItr;
        ConstIterator             _zCoordItr;
        CoordType                 _coords;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrU3D(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrU3D(this, false))); }



    VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGrid3D &sg);

protected:
    virtual void GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const override;

private:
    UnstructuredGridCoordless _xug;
    UnstructuredGridCoordless _yug;
    UnstructuredGridCoordless _zug;

    bool _insideGrid(const CoordType &coords, DimsType &cindices, std::vector<size_t> &nodes2D, std::vector<double> &lambda, float zwgt[2]) const;
};
};    // namespace VAPoR
