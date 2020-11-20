#include <iostream>
#include <omp.h>
#include <vapor/VAssert.h>
#include <vapor/utils.h>
#include <cstdint>
#include <vapor/QuadTreeRectangleP.h>

using namespace VAPoR;
using namespace std;

using UInt32_tArr2 = std::array<uint32_t, 2>;
using pType = UInt32_tArr2;

QuadTreeRectangleP::QuadTreeRectangleP(float left, float top, float right, float bottom, size_t max_depth, size_t reserve_size) : _left(left), _right(right)
{
    VAssert(left <= right);
    VAssert(top <= bottom);

    int nthreads = 1;
#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
    }

    for (int i = 0; i < nthreads; i++) {
        float bin_width = (right - left) / ((float)nthreads);
        float l = left + (i * bin_width);
        float r = l + bin_width;
        _qtrs.push_back(new QuadTreeRectangle<float, pType>(l, top, r, bottom, max_depth, reserve_size));
    }
}

QuadTreeRectangleP::QuadTreeRectangleP(size_t max_depth, size_t reserve_size) : _left(0.0), _right(1.0)
{
    int nthreads = 1;
#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
    }

    for (int i = 0; i < nthreads; i++) {
        float bin_width = (1.0 - 0.0) / ((float)nthreads);
        float l = 0.0 + (i * bin_width);
        float r = l + bin_width;
        _qtrs.push_back(new QuadTreeRectangle<float, pType>(l, 0.0, r, 1.0, max_depth, reserve_size));
    }
}

QuadTreeRectangleP::QuadTreeRectangleP(const QuadTreeRectangleP &rhs)
{
    _left = rhs._left;
    _right = rhs._right;

    _qtrs.resize(rhs._qtrs.size());
    for (int i = 0; i < _qtrs.size(); i++) { _qtrs[i] = new QuadTreeRectangle<float, pType>(*(rhs._qtrs[i])); }
}

QuadTreeRectangleP &QuadTreeRectangleP::operator=(const QuadTreeRectangleP &rhs)
{
    //	if (*this == rhs) return *this;

    _left = rhs._left;
    _right = rhs._right;
    for (int i = 0; i < _qtrs.size(); i++) {
        if (_qtrs[i]) delete _qtrs[i];
    }

    _qtrs.resize(rhs._qtrs.size());
    for (size_t i = 0; i < rhs._qtrs.size(); i++) { _qtrs[i] = new QuadTreeRectangle<float, pType>(*(rhs._qtrs[i])); }
    return *this;
}

QuadTreeRectangleP::~QuadTreeRectangleP()
{
    for (size_t i = 0; i < _qtrs.size(); i++) {
        if (_qtrs[i]) delete _qtrs[i];
    }
    _qtrs.clear();
}

bool QuadTreeRectangleP::Insert(float left, float top, float right, float bottom, Size_tArr3 payload)
{
    bool status = true;
    for (int i = 0; i < _qtrs.size(); i++) {
        float bin_width = (_right - _left) / ((float)_qtrs.size());
        float l = _left + (i * bin_width);
        float r = l + bin_width;

        if (left <= r && right > l) {
            pType p = {(uint32_t)payload[0], (uint32_t)payload[1]};
            status &= _qtrs[i]->Insert(left, top, right, bottom, p);
        }
    }
    return (status);
}

bool QuadTreeRectangleP::Insert(std::vector<class QuadTreeRectangle<float, pType>::rectangle_t> rectangles, std::vector<pType> payloads)
{
    VAssert(rectangles.size() == payloads.size());

    bool status = true;

    vector<vector<class QuadTreeRectangle<float, pType>::rectangle_t>> parRectangles(_qtrs.size());
    vector<vector<pType>>                                              parPayloads(_qtrs.size());
    for (int i = 0; i < _qtrs.size(); i++) {
        parRectangles[i].reserve(rectangles.size() / _qtrs.size());
        parPayloads[i].reserve(payloads.size() / _qtrs.size());
    }

    float bin_width = (_right - _left) / ((float)_qtrs.size());
    for (size_t j = 0; j < rectangles.size(); j++) {
        for (int i = 0; i < _qtrs.size(); i++) {
            float l = _left + (i * bin_width);
            float r = l + bin_width;

            if ((rectangles[j]._left <= r) && (rectangles[j]._right > l)) {
                parRectangles[i].push_back(rectangles[j]);
                parPayloads[i].push_back(payloads[j]);
            }
        }
    }

#pragma omp parallel
#pragma omp for
    for (int i = 0; i < _qtrs.size(); i++) {
        for (size_t j = 0; j < parRectangles[i].size(); j++) { status &= _qtrs[i]->Insert(parRectangles[i][j], parPayloads[i][j]); }
    }

    return (status);
}

bool QuadTreeRectangleP::Insert(const Grid *grid)
{
    size_t ncells = Wasp::VProduct(grid->GetCellDimensions());

    vector<vector<class QuadTreeRectangle<float, pType>::rectangle_t>> parRectangles(_qtrs.size());
    vector<vector<pType>>                                              parPayloads(_qtrs.size());
    for (int i = 0; i < _qtrs.size(); i++) {
        parRectangles[i].reserve(ncells / _qtrs.size());
        parPayloads[i].reserve(ncells / _qtrs.size());
    }

    int ncellindices = grid->GetCellDimensions().size();

#pragma omp parallel
    {
        size_t             maxNodes = grid->GetMaxVertexPerCell();
        vector<Size_tArr3> nodes(maxNodes);

        int    id = omp_get_thread_num();
        int    nthreads = omp_get_num_threads();
        size_t istart = id * ncells / nthreads;
        size_t iend = (id + 1) * ncells / nthreads;
        if (id == nthreads - 1) iend = ncells;

        DblArr3                 coords;
        Grid::ConstCellIterator itr = grid->ConstCellBegin() + istart;

        for (size_t i = istart; i < iend; i++, ++itr) {
            Size_tArr3 cell = {0, 0, 0};
            Grid::CopyToArr3((*itr).data(), ncellindices, cell);

            grid->GetCellNodes(cell, nodes);
            if (nodes.size() < 2) continue;

            grid->GetUserCoordinates(nodes[0], coords);

            float left = (float)coords[0];
            float right = (float)coords[0];
            float top = (float)coords[1];
            float bottom = (float)coords[1];

            for (int j = 1; j < nodes.size(); j++) {
                grid->GetUserCoordinates(nodes[j], coords);

                if (coords[0] < left) left = coords[0];
                if (coords[0] > right) right = coords[0];
                if (coords[1] < top) top = coords[1];
                if (coords[1] > bottom) bottom = coords[1];
            }

            float bin_width = (_right - _left) / ((float)_qtrs.size());

#pragma omp critical
            for (int j = 0; j < _qtrs.size(); j++) {
                float l = _left + (j * bin_width);
                float r = l + bin_width;

                if ((left <= r) && (right > l)) {
                    class QuadTreeRectangle<float, pType>::rectangle_t r(left, top, right, bottom);

                    pType p = {(uint32_t)cell[0], (uint32_t)cell[1]};
                    parRectangles[j].push_back(r);
                    parPayloads[j].push_back(p);
                }
            }
        }
    }

    bool status = true;

#pragma omp parallel
#pragma omp for
    for (int i = 0; i < _qtrs.size(); i++) {
        for (size_t j = 0; j < parRectangles[i].size(); j++) { status &= _qtrs[i]->Insert(parRectangles[i][j], parPayloads[i][j]); }
    }

    return (status);
}

void QuadTreeRectangleP::GetPayloadContained(float x, float y, std::vector<Size_tArr3> &payloads) const
{
    payloads.clear();

    float bin_width = (_right - _left) / ((float)_qtrs.size());
    int   bin = (x - _left) / bin_width;

    if (bin < 0 || bin >= _qtrs.size()) return;

    std::vector<pType> p;
    _qtrs[bin]->GetPayloadContained(x, y, p);

    for (auto itr = p.begin(); itr != p.end(); ++itr) { payloads.push_back(Size_tArr3{(*itr)[0], (*itr)[1], 0}); }
}

void QuadTreeRectangleP::GetStats(std::vector<size_t> &payload_histo, std::vector<size_t> &level_histo) const
{
    payload_histo.clear();
    level_histo.clear();
}
