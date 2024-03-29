#ifndef _UnstructuredGridCoordless_
#define _UnstructuredGridCoordless_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/UnstructuredGrid.h>

#ifdef WIN32
    #pragma warning(disable : 4661 4251)    // needed for template class
#endif

namespace VAPoR {

//! \class UnstructuredGridCoordless
//! \brief class for a 2D unstructured grid.
//! \author John Clyne
//!
//
class VDF_API UnstructuredGridCoordless : public UnstructuredGrid {
public:
    //! Construct a unstructured grid sampling 2D scalar function
    //!
    //
    UnstructuredGridCoordless(const DimsType &vertexDims, const DimsType &faceDims, const DimsType &edgeDims, const DimsType &bs, const std::vector<float *> &blks, size_t topology_dimension,
                              const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                              Location location,    // node,face, edge
                              size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset)
    : UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, topology_dimension, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset)
    {
    }
    UnstructuredGridCoordless(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs,
                              const std::vector<float *> &blks, size_t topology_dimension, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                              Location location,    // node,face, edge
                              size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset)
    : UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, topology_dimension, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset)
    {
    }

    UnstructuredGridCoordless() = default;
    virtual ~UnstructuredGridCoordless() = default;

    virtual DimsType GetCoordDimensions(size_t dim) const override { return (DimsType{1, 1, 1}); }

    static std::string GetClassType() { return ("UnstructuredCoordless"); }
    std::string        GetType() const override { return (GetClassType()); }

    virtual void GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const override
    {
        minu = {0.0, 0.0, 0.0};
        maxu = {0.0, 0.0, 0.0};
    }

    bool GetEnclosingRegion(const CoordType &minu, const CoordType &maxu, DimsType &min, DimsType &max) const override { return (false); }

    virtual void GetUserCoordinates(const DimsType &indices, CoordType &coords) const override {}

    bool GetIndicesCell(const CoordType &, DimsType &) const override { return (false); }

    bool InsideGrid(const CoordType &coords) const override { return (false); }

    float GetValueNearestNeighbor(const CoordType &coords) const override { return (0.0); }

    float GetValueLinear(const CoordType &coords) const override { return (0.0); }

    virtual size_t GetGeometryDim() const override { return (0); }

    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    class ConstCoordItrUCoordless : public Grid::ConstCoordItrAbstract {
    public:
        ConstCoordItrUCoordless(const UnstructuredGridCoordless *ug, bool begin) {}
        ConstCoordItrUCoordless(const ConstCoordItrUCoordless &rhs) {}

        ConstCoordItrUCoordless();
        virtual ~ConstCoordItrUCoordless() {}

        virtual void            next() {}
        virtual void            next(const long &offset) {}
        virtual ConstCoordType &deref() const { return (_coords); }
        virtual const void *    address() const { return this; };

        virtual bool equal(const void *rhs) const { return (true); }

        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const { return std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrUCoordless(*this)); };

    private:
        CoordType _coords;
    };

    virtual ConstCoordItr ConstCoordBegin() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrUCoordless(this, true))); }
    virtual ConstCoordItr ConstCoordEnd() const override { return ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract>(new ConstCoordItrUCoordless(this, false))); }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGridCoordless &sg);

protected:
    virtual void GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const override
    {
        minu = {0.0, 0.0, 0.0};
        maxu = {0.0, 0.0, 0.0};
    }

private:
};
};    // namespace VAPoR

#endif
