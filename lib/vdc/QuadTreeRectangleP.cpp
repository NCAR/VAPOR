#include <iostream>
#include <vapor/VAssert.h>
#include <vapor/utils.h>
#include <cstdint>
#include <vapor/QuadTreeRectangleP.h>
#include <vapor/OpenMPSupport.h>

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
printf("num threads = %d\n", nthreads);
    }

    // We split the quadtree along the X-axis to create one subtree for
    // each thread. This way there are no dependencies between the regions
    // covered by each subtree and we can process each subtree in parallel
    //
    float bin_width = ((float)right - (float)left) / ((float)nthreads);
    float binLeft = left;
    for (int i = 0; i < nthreads; i++) {
        float binRight = binLeft + bin_width;

        if (i == nthreads - 1) binRight = right;

        _qtrs.push_back(new QuadTreeRectangle<float, pType>(binLeft, top, binRight, bottom, max_depth, reserve_size / nthreads));
        binLeft = binRight;
    }
}

QuadTreeRectangleP::QuadTreeRectangleP(size_t max_depth, size_t reserve_size) : _left(0.0), _right(1.0)
{
    int nthreads = 1;
#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
printf("num threads = %d\n", nthreads);
    }

    float bin_width = (1.0 - 0.0) / ((float)nthreads);
    float binLeft = 0.0;
    for (int i = 0; i < nthreads; i++) {
        float binRight = binLeft + bin_width;

        if (i == nthreads - 1) binRight = 1.0;

        _qtrs.push_back(new QuadTreeRectangle<float, pType>(binLeft, 0.0, binRight, 1.0, max_depth, reserve_size / nthreads));

        binLeft = binRight;
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
    // Serial insertion of a single element
    //
    bool  status = true;
    float bin_width = (_right - _left) / ((float)_qtrs.size());
    float binLeft = _left;
    for (int i = 0; i < _qtrs.size(); i++) {
        float binRight = binLeft + bin_width;

        if (i == _qtrs.size() - 1) binRight = right;

        if (left <= binRight && right >= binLeft) {
            pType p = {(uint32_t)payload[0], (uint32_t)payload[1]};
            status &= _qtrs[i]->Insert(left, top, right, bottom, p);
        }

        binLeft = binRight;
    }
    return (status);
}

bool QuadTreeRectangleP::Insert(std::vector<class QuadTreeRectangle<float, pType>::rectangle_t> rectangles, std::vector<pType> payloads)
{
    VAssert(rectangles.size() == payloads.size());

    bool status = true;

    vector<vector<class QuadTreeRectangle<float, pType>::rectangle_t>> parRectangles(_qtrs.size());
    vector<vector<pType>>                                              parPayloads(_qtrs.size());

    // Pre-allocate space for each tree
    //
    for (int i = 0; i < _qtrs.size(); i++) {
        parRectangles[i].reserve(rectangles.size() / _qtrs.size());
        parPayloads[i].reserve(payloads.size() / _qtrs.size());
    }

    // parRectangles and parPayloads will contain the regions that
    // need to be inserted into each subtree
    //
    float bin_width = (_right - _left) / ((float)_qtrs.size());
    float binLeft = _left;
    for (size_t j = 0; j < rectangles.size(); j++) {
        for (int i = 0; i < _qtrs.size(); i++) {
            float binRight = binLeft + bin_width;

            if (i == _qtrs.size() - 1) binRight = _right;

            if ((rectangles[j]._left <= binRight) && (rectangles[j]._right >= binLeft)) {
                parRectangles[i].push_back(rectangles[j]);
                parPayloads[i].push_back(payloads[j]);
            }

            binLeft = binRight;
        }
    }

// Perform parallel construction of tree
//
#pragma omp parallel
#pragma omp for
    for (int i = 0; i < _qtrs.size(); i++) {
        for (size_t j = 0; j < parRectangles[i].size(); j++) { status &= _qtrs[i]->Insert(parRectangles[i][j], parPayloads[i][j]); }
    }

    return (status);
}

bool QuadTreeRectangleP::Insert(const Grid *grid, size_t ncells)
{
    if (ncells == 0) { ncells = Wasp::VProduct(grid->GetCellDimensions()); }

    // parRectangles and parPayloads will contain the rectangles and their
    // payloads that need to be inserted into each substree
    //
    vector<vector<class QuadTreeRectangle<float, pType>::rectangle_t>> parRectangles(_qtrs.size());
    vector<vector<pType>>                                              parPayloads(_qtrs.size());
    for (int i = 0; i < _qtrs.size(); i++) {
        parRectangles[i].reserve(ncells / _qtrs.size());
        parPayloads[i].reserve(ncells / _qtrs.size());
    }

    int ncellindices = grid->GetCellDimensions().size();

// Populate parRectangles and parPayloads with the data that will
// be used to contruct the quadtree
//
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

                if (coords[0] < left) left = (float)coords[0];
                if (coords[0] > right) right = (float)coords[0];
                if (coords[1] < top) top = (float)coords[1];
                if (coords[1] > bottom) bottom = (float)coords[1];
            }

            // Figure out which subtree(s) contain the rectangle. In general,
            // a rectangle can span multiple subtrees, in which case it
            // will be inserted into both.
            //
            float bin_width = (_right - _left) / ((float)_qtrs.size());
            float binLeft = _left;
            for (int j = 0; j < _qtrs.size(); j++) {
                float binRight = binLeft + bin_width;

                if (j == _qtrs.size() - 1) binRight = _right;

                if ((left <= binRight) && (right >= binLeft)) {
                    class QuadTreeRectangle<float, pType>::rectangle_t r(left, top, right, bottom);

                    pType p = {(uint32_t)cell[0], (uint32_t)cell[1]};
#pragma omp critical
                    {
                        parRectangles[j].push_back(r);
                        parPayloads[j].push_back(p);
                    }
                }

                binLeft = binRight;
            }
        }
    }

    // At this point parRectangles and parPayloads contain a vector of
    // rectangles and their payloads that intersect each of the subtrees that
    // make up the tree. Since there is no shared data between each of
    // the subtrees we can populate each subtree in parallel without any need
    // for synchronization (mutex, etc.)
    //
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

    int   bin = 0;
    float bin_width = ((float)_right - (float)_left) / ((float)_qtrs.size());
    float binLeft = _left;
    for (int i = 0; i < _qtrs.size(); i++) {
        float binRight = binLeft + bin_width;
        if (i == _qtrs.size() - 1) binRight = _right;

        if (x >= binLeft && x <= binRight) {
            bin = i;
            break;
        }
        binLeft = binRight;
    }

    std::vector<pType> p;
    _qtrs[bin]->GetPayloadContained(x, y, p);

    for (auto itr = p.begin(); itr != p.end(); ++itr) { payloads.push_back(Size_tArr3{(*itr)[0], (*itr)[1], 0}); }
}

void QuadTreeRectangleP::GetStats(std::vector<size_t> &payload_histo, std::vector<size_t> &level_histo) const
{
    payload_histo.clear();
    level_histo.clear();

    for (int i = 0; i < _qtrs.size(); i++) {
        std::vector<size_t> p;
        std::vector<size_t> l;

        _qtrs[i]->GetStats(p, l);
        payload_histo.insert(payload_histo.end(), p.begin(), p.end());
        level_histo.insert(level_histo.end(), l.begin(), l.end());
    }
}
