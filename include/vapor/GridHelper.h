#include <vector>
#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>
#include <vapor/DC.h>
#include <vapor/MyBase.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/RegularGrid.h>
#include <vapor/StretchedGrid.h>
#include <vapor/UnstructuredGrid2D.h>
#include <vapor/UnstructuredGridLayered.h>

#ifndef GRIDMGR_H
    #define GRIDMGR_H

namespace VAPoR {

class UnstructuredGrid3D;

class VDF_API GridHelper : public Wasp::MyBase {
public:
    GridHelper(size_t max_size = 10) : _qtrCache(max_size) {}

    ~GridHelper();

    string GetGridType(const DC::Mesh &m, const std::vector<DC::CoordVar> &cvarsinfo, const std::vector<vector<string>> &cdimnames) const;

    bool IsUnstructured(std::string gridType) const;
    bool IsStructured(std::string gridType) const;

    //	var: variable info
    //  roi_dims: spatial dimensions of ROI
    //	dims: spatial dimensions of full variable domain in voxels
    //	blkvec: data blocks, and coordinate blocks
    //	bsvec: data block dimensions, and coordinate block dimensions
    //  bminvec: ROI offsets in blocks, full domain, data and coordinates
    //  bmaxvec: ROI offsets in blocks, full domain, data and coordinates
    //
    StructuredGrid *MakeGridStructured(string gridType, size_t ts, int level, int lod, const DC::DataVar &var, const std::vector<DC::CoordVar> &cvarsinfo, const std::vector<size_t> &roi_dims,
                                       const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<std::vector<size_t>> &bsvec,
                                       const std::vector<std::vector<size_t>> &bminvec, const std::vector<std::vector<size_t>> &bmaxvec);

    UnstructuredGrid *MakeGridUnstructured(string gridType, size_t ts, int level, int lod, const DC::DataVar &var, const std::vector<DC::CoordVar> &cvarsinfo, const std::vector<size_t> &roi_dims,
                                           const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<std::vector<size_t>> &bsvec,
                                           const std::vector<std::vector<size_t>> &bminvec, const std::vector<std::vector<size_t>> &bmaxvec, const std::vector<int *> &conn_blkvec,
                                           const std::vector<std::vector<size_t>> &conn_bsvec, const std::vector<std::vector<size_t>> &conn_bminvec,
                                           const std::vector<std::vector<size_t>> &conn_bmaxvec, const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims,
                                           const std::vector<size_t> &edgeDims, UnstructuredGrid::Location location, size_t maxVertexPerFace, size_t maxFacePerVertex, long vertexOffset,
                                           long faceOffset);

private:
    template<typename key_t, typename value_t> class lru_cache {
    public:
        typedef typename std::pair<key_t, value_t>             key_value_pair_t;
        typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

        lru_cache() : _max_size(10) {}

        lru_cache(size_t max_size) : _max_size(max_size) {}

        value_t put(const key_t &key, value_t value)
        {
            value_t rvalue = NULL;
            auto    it = _cache_items_map.find(key);
            _cache_items_list.push_front(key_value_pair_t(key, value));
            if (it != _cache_items_map.end()) {
                rvalue = it->second->second;
                _cache_items_list.erase(it->second);
                _cache_items_map.erase(it);
            }
            _cache_items_map[key] = _cache_items_list.begin();

            if (_cache_items_map.size() > _max_size) {
                auto last = _cache_items_list.end();
                last--;
                rvalue = last->second;
                _cache_items_map.erase(last->first);
                _cache_items_list.pop_back();
            }
            return (rvalue);
        }

        value_t get(const key_t &key)
        {
            auto it = _cache_items_map.find(key);
            if (it == _cache_items_map.end()) return (NULL);

            _cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
            return it->second->second;
        }

        value_t remove_lru()
        {
            if (!_cache_items_map.size()) return (NULL);

            auto last = _cache_items_list.end();
            last--;
            value_t rvalue = last->second;
            last->second = nullptr;    // necessary for delete?
            _cache_items_map.erase(last->first);
            _cache_items_list.pop_back();
            return (rvalue);
        }

        bool exists(const key_t &key) const;

        size_t size() const { return _cache_items_map.size(); }

    private:
        std::list<key_value_pair_t>                _cache_items_list;
        std::unordered_map<key_t, list_iterator_t> _cache_items_map;
        size_t                                     _max_size;
    };

    lru_cache<string, std::shared_ptr<const QuadTreeRectangle<float, size_t>>> _qtrCache;

    RegularGrid *_make_grid_regular(const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax

    ) const;

    StretchedGrid *_make_grid_stretched(const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin,
                                        const std::vector<size_t> &bmax) const;

    LayeredGrid *_make_grid_layered(const std::vector<size_t> &dims, const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin,
                                    const std::vector<size_t> &bmax) const;

    CurvilinearGrid *_make_grid_curvilinear(size_t ts, int level, int lod, const std::vector<DC::CoordVar> &cvarsinfo, const std::vector<size_t> &dims, const std::vector<float *> &blkvec,
                                            const std::vector<size_t> &bs, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax);

    UnstructuredGrid2D *_make_grid_unstructured2d(size_t ts, int level, int lod, const DC::DataVar &var, const std::vector<DC::CoordVar> &cvarsinfo, const std::vector<size_t> &dims,
                                                  const std::vector<float *> &blkvec, const std::vector<size_t> &bs, const std::vector<size_t> &bmin, const std::vector<size_t> &bmax,
                                                  const std::vector<int *> &conn_blkvec, const std::vector<size_t> &conn_bs, const std::vector<size_t> &conn_bmin, const std::vector<size_t> &conn_bmax,
                                                  const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, UnstructuredGrid::Location location,
                                                  size_t maxVertexPerFace, size_t maxFacePerVertex, long vertexOffset, long faceOffset);

    UnstructuredGridLayered *_make_grid_unstructured_layered(size_t ts, int level, int lod, const DC::DataVar &var, const vector<DC::CoordVar> &cvarsinfo, const vector<size_t> &dims,
                                                             const vector<float *> &blkvec, const vector<size_t> &bs, const vector<size_t> &bmin, const vector<size_t> &bmax,
                                                             const vector<int *> &conn_blkvec, const vector<size_t> &conn_bs, const vector<size_t> &conn_bmin, const vector<size_t> &conn_bmax,
                                                             const vector<size_t> &vertexDims, const vector<size_t> &faceDims, const vector<size_t> &edgeDims, UnstructuredGrid::Location location,
                                                             size_t maxVertexPerFace, size_t maxFacePerVertex, long vertexOffset, long faceOffset);

    UnstructuredGrid3D *_make_grid_unstructured_3d(size_t ts, int level, int lod, const DC::DataVar &var, const vector<DC::CoordVar> &cvarsinfo, const vector<size_t> &dims,
                                                   const vector<float *> &blkvec, const vector<size_t> &bs, const vector<size_t> &bmin, const vector<size_t> &bmax, const vector<int *> &conn_blkvec,
                                                   const vector<size_t> &conn_bs, const vector<size_t> &conn_bmin, const vector<size_t> &conn_bmax, const vector<size_t> &vertexDims,
                                                   const vector<size_t> &faceDims, const vector<size_t> &edgeDims, UnstructuredGrid::Location location, size_t maxVertexPerFace,
                                                   size_t maxFacePerVertex, long vertexOffset, long faceOffset);

    void _makeGridHelper(const DC::DataVar &var, const std::vector<size_t> &roi_dims, const std::vector<size_t> &dims, Grid *g) const;

    string _getQuadTreeRectangleKey(size_t ts, int level, int lod, const vector<DC::CoordVar> &cvarsinfo, const vector<size_t> &bmin, const vector<size_t> &bmax) const;
};

};    // namespace VAPoR
#endif
