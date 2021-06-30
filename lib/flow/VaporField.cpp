#include "vapor/VaporField.h"
#include "vapor/ConstantGrid.h"

using namespace flow;

//
// Class GridKey
//
void GridKey::Reset(uint64_t ts, int32_t ref, int32_t comp, std::string var, const std::vector<double> &min, const std::vector<double> &max)
{
    _timestep = ts;
    _refLev = ref;
    _compLev = comp;

    _ext_min = {0.0, 0.0, 0.0};
    _ext_max = {0.0, 0.0, 0.0};

    for (int i = 0; i < min.size(); i++) _ext_min[i] = min[i];
    for (int i = 0; i < max.size(); i++) _ext_max[i] = max[i];

    _varName = std::move(var);
}

bool GridKey::operator==(const GridKey &other) const
{
    // String comparison seems to occur most frequently, so put it at the first.
    if (this->_varName != other._varName) return false;
    if (this->_timestep != other._timestep) return false;
    if (this->_refLev != other._refLev) return false;
    if (this->_compLev != other._compLev) return false;
    if (this->_ext_min != other._ext_min) return false;
    if (this->_ext_max != other._ext_max) return false;

    return true;
}

bool GridKey::emptyVar() const { return _varName.empty(); }

//
// Class GridWrapper
//
GridWrapper::GridWrapper(const VAPoR::Grid *gp, VAPoR::DataMgr *mp) : _gridPtr(gp), _mgr(mp) {}

GridWrapper::~GridWrapper()
{
    if (_gridPtr && _mgr) {
        _mgr->UnlockGrid(_gridPtr);
        delete _gridPtr;
    }
}

const VAPoR::Grid *GridWrapper::grid() const { return _gridPtr; }

//
// Class VaporField
//
VaporField::VaporField(size_t cache_limit) : _recentGrids(cache_limit, false) {}

auto VaporField::LockParams() -> int
{
    if (!_isReady()) return 1;

    // Retrieve param values and put them in the local cache.
    _c_currentTS = _params->GetCurrentTimestep();
    _c_refLev = _params->GetRefinementLevel();
    _c_compLev = _params->GetCompressionLevel();
    _c_vel_mult = _params->GetVelocityMultiplier();

    _params->GetBox()->GetExtents(_c_ext_min, _c_ext_max);

    for (int i = 0; i < 3; i++) { _c_velocity_grids[i] = _getAGrid(_c_currentTS, this->VelocityNames[i]); }
    _c_scalar_grid = _getAGrid(_c_currentTS, this->ScalarName);

    _params_locked = true;
    return 0;
}

auto VaporField::UnlockParams() -> int
{
    _c_currentTS = 0;
    _c_refLev = -2;
    _c_compLev = -2;
    _c_vel_mult = 0.0;
    _c_ext_min.clear();
    _c_ext_max.clear();

    for (int i = 0; i < 3; i++) { _c_velocity_grids[i] = nullptr; }
    _c_scalar_grid = nullptr;

    _params_locked = false;
    return 0;
}

bool VaporField::InsideVolumeVelocity(double time, const glm::vec3 &pos) const
{
    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *         grid = nullptr;
    VAssert(_isReady());

    // In case of steady field, we only check a specific time step
    if (IsSteady) {
        for (int i = 0; i < 3; i++) {
            const auto &v = VelocityNames[i];
            if (_params_locked) {
                grid = _c_velocity_grids[i];
            } else {
                auto currentTS = _params->GetCurrentTimestep();
                grid = _getAGrid(currentTS, v);
            }

            if (grid == nullptr) return false;
            if (!grid->InsideGrid(coords)) return false;
        }
    } else    // In case of unsteady, we check two time steps
    {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back()) return false;

        // Then locate the 2 time steps
        size_t floor = 0;
        int    rv = LocateTimestamp(time, floor);
        if (rv != 0) return false;

        // Then test if pos is inside of time step "floor"
        for (auto &v : VelocityNames) {
            grid = _getAGrid(floor, v);
            if (grid == nullptr) return false;
            if (!grid->InsideGrid(coords)) return false;
        }

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if (time > _timestamps[floor]) {
            for (auto &v : VelocityNames) {
                grid = _getAGrid(floor + 1, v);
                if (grid == nullptr) return false;
                if (!grid->InsideGrid(coords)) return false;
            }
        }
    }

    return true;
}

bool VaporField::InsideVolumeScalar(double time, const glm::vec3 &pos) const
{
    // When this variable doesn't exist, it doesn't make sense to say if
    // a position is inside of the volume, so simply return true.
    if (ScalarName.empty()) return true;

    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *         grid = nullptr;
    VAssert(_isReady());

    // In case of steady field, we only check a specific time step
    if (IsSteady) {
        if (_params_locked) {
            grid = _c_scalar_grid;
        } else {
            auto currentTS = _params->GetCurrentTimestep();
            grid = _getAGrid(currentTS, ScalarName);
        }
        if (grid == nullptr) return false;
        return grid->InsideGrid(coords);
    } else    // In case of unsteady field, we check two time steps
    {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back()) return false;

        // Then locate the 2 time steps
        size_t floor = 0;
        int    rv = LocateTimestamp(time, floor);
        if (rv != 0) return false;

        // Then test if pos is inside of time step "floor"
        grid = _getAGrid(floor, ScalarName);
        if (grid == nullptr) return false;
        if (!grid->InsideGrid(coords)) return false;

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if (time > _timestamps[floor]) {
            grid = _getAGrid(floor + 1, ScalarName);
            if (grid == nullptr) return false;
            if (!grid->InsideGrid(coords)) return false;
        }

        return true;
    }
}

int VaporField::GetVelocityIntersection(size_t ts, glm::vec3 &minxyz, glm::vec3 &maxxyz) const
{
    const VAPoR::Grid *   grid = nullptr;
    std::array<double, 3> min[3], max[3];

    // For each velocity variables
    for (int i = 0; i < 3; i++) {
        grid = _getAGrid(ts, VelocityNames[i]);
        if (grid == nullptr) {
            Wasp::MyBase::SetErrMsg("Vector field not available at requested time step!");
            return GRID_ERROR;
        } else
            grid->GetUserExtents(min[i], max[i]);
    }

    minxyz = glm::vec3(min[0][0], min[0][1], min[0][2]);
    maxxyz = glm::vec3(max[0][0], max[0][1], max[0][2]);

    for (int i = 1; i < 3; i++) {
        glm::vec3 xyz(min[i][0], min[i][1], min[i][2]);
        minxyz = glm::max(minxyz, xyz);
        xyz = glm::vec3(max[i][0], max[i][1], max[i][2]);
        maxxyz = glm::min(maxxyz, xyz);
    }

    return 0;
}

int VaporField::GetVelocity(double time, const glm::vec3 &pos, glm::vec3 &velocity) const
{
    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *         grid = nullptr;

    // Retrieve the missing value and velocity multiplier
    glm::vec3 missingV(0.0f);    // stores missing values for 3 velocity variables
    velocity = glm::vec3(0.0f);

    if (IsSteady) {
        for (int i = 0; i < 3; i++) {
            if (_params_locked) {
                grid = _c_velocity_grids[i];
            } else {
                auto currentTS = _params->GetCurrentTimestep();
                grid = _getAGrid(currentTS, VelocityNames[i]);
            }
            if (grid == nullptr) return GRID_ERROR;
            velocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto  hasMissing = glm::equal(velocity, missingV);
        float mult = _params_locked ? _c_vel_mult : _params->GetVelocityMultiplier();
        if (glm::any(hasMissing))
            return MISSING_VAL;
        else {
            velocity *= mult;
            return 0;
        }
    } else {
        float mult = _params->GetVelocityMultiplier();

        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back()) return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS = 0;
        int    rv = LocateTimestamp(time, floorTS);
        VAssert(rv == 0);

        // Find the velocity values at floor time step
        glm::vec3 floorVelocity(0.f, 0.f, 0.f);
        glm::vec3 ceilingVelocity(0.f, 0.f, 0.f);
        for (int i = 0; i < 3; i++) {
            grid = _getAGrid(floorTS, VelocityNames[i]);
            if (grid == nullptr) return GRID_ERROR;
            floorVelocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal(floorVelocity, missingV);
        if (glm::any(hasMissing)) { return MISSING_VAL; }

        if (time == _timestamps[floorTS]) {
            velocity = floorVelocity * mult;
            return 0;
        } else    // Find the velocity values at the ceiling time step
        {
            // We need to make sure there aren't duplicate time stamps
            VAssert(_timestamps[floorTS + 1] > _timestamps[floorTS]);
            for (int i = 0; i < 3; i++) {
                grid = _getAGrid(floorTS + 1, VelocityNames[i]);
                if (grid == nullptr) return GRID_ERROR;
                ceilingVelocity[i] = grid->GetValue(coords);
                missingV[i] = grid->GetMissingValue();
            }
            hasMissing = glm::equal(ceilingVelocity, missingV);
            if (glm::any(hasMissing)) { return MISSING_VAL; }

            float weight = (time - _timestamps[floorTS]) / (_timestamps[floorTS + 1] - _timestamps[floorTS]);
            velocity = glm::mix(floorVelocity, ceilingVelocity, weight) * mult;
            return 0;
        }
    }    // end of unsteady condition
}

int VaporField::GetScalar(double time, const glm::vec3 &pos, float &scalar) const
{
    // When this variable doesn't exist, it doesn't make sense to get a scalar value
    // from it, so just return that fact.
    if (ScalarName.empty()) return NO_FIELD_YET;

    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *         grid = nullptr;

    if (IsSteady) {
        if (_params_locked) {
            grid = _c_scalar_grid;
        } else {
            auto currentTS = _params->GetCurrentTimestep();
            grid = _getAGrid(currentTS, ScalarName);
        }
        if (grid == nullptr) return GRID_ERROR;
        scalar = grid->GetValue(coords);
        if (scalar == grid->GetMissingValue()) {
            return MISSING_VAL;
        } else {
            return 0;
        }
    } else {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back()) return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS = 0;
        int    rv = LocateTimestamp(time, floorTS);
        VAssert(rv == 0);
        grid = _getAGrid(floorTS, ScalarName);
        if (grid == nullptr) return GRID_ERROR;
        float floorScalar = grid->GetValue(coords);
        if (floorScalar == grid->GetMissingValue()) { return MISSING_VAL; }

        if (time == _timestamps[floorTS]) {
            scalar = floorScalar;
            return 0;
        } else {
            grid = _getAGrid(floorTS + 1, ScalarName);
            if (grid == nullptr) return GRID_ERROR;

            float ceilingScalar = grid->GetValue(coords);
            if (ceilingScalar == grid->GetMissingValue()) {
                return MISSING_VAL;
            } else {
                float weight = (time - _timestamps[floorTS]) / (_timestamps[floorTS + 1] - _timestamps[floorTS]);
                scalar = glm::mix(floorScalar, ceilingScalar, weight);
                return 0;
            }
        }
    }    // end of unsteady condition
}

bool VaporField::_isReady() const
{
    if (!_datamgr) return false;
    if (!_params) return false;

    return true;
}

void VaporField::AssignDataManager(VAPoR::DataMgr *dmgr)
{
    _datamgr = dmgr;

    // Make a copy of the timestamps from the new data manager
    const auto &timeCoords = dmgr->GetTimeCoordinates();
    _timestamps.resize(timeCoords.size());
    for (size_t i = 0; i < timeCoords.size(); i++) _timestamps[i] = timeCoords[i];
}

void VaporField::UpdateParams(const VAPoR::FlowParams *p)
{
    _params = p;
    IsSteady = p->GetIsSteady();
}

int VaporField::LocateTimestamp(double time, size_t &floor) const
{
    if (_timestamps.empty()) return TIME_ERROR;
    if (_timestamps.size() == 1) {
        if (_timestamps[0] != time)
            return TIME_ERROR;
        else {
            floor = 0;
            return 0;
        }
    }

    if (time < _timestamps.front() || time > _timestamps.back())
        return TIME_ERROR;
    else {
        auto it = std::upper_bound(_timestamps.cbegin(), _timestamps.cend(), time);
        floor = --it - _timestamps.cbegin();
        return 0;
    }
}

int VaporField::GetNumberOfTimesteps() const { return _timestamps.size(); }

int VaporField::CalcDeltaTFromCurrentTimeStep(double &delT) const
{
    VAssert(_isReady());

    // This function is called only one-time before advection starts,
    // so don't worry about parameter locking here.
    const auto currentTS = _params->GetCurrentTimestep();
    const auto timestamp = _timestamps.at(currentTS);

    // Let's find the intersection of 3 velocity components.
    glm::vec3 minxyz(0.0f), maxxyz(0.0f);
    int       rv = this->GetVelocityIntersection(currentTS, minxyz, maxxyz);
    if (rv != 0) return rv;

    // Let's make sure the max is greater than or equal to min
    const auto invalid = glm::lessThan(maxxyz, minxyz);
    if (glm::any(invalid)) {
        Wasp::MyBase::SetErrMsg("One of the selected volume dimension is zero!");
        return GRID_ERROR;
    }

    // Let's sample some locations along each dimension.
    const long      N = 10;    // Num of samples along each axis
    const long      totalSamples = N * N * N;
    const glm::vec3 numOfSteps(float(N + 1));
    const glm::vec3 stepSizes = (maxxyz - minxyz) / numOfSteps;
    glm::vec3       samples[totalSamples];
    long            counter = 0;
    for (long z = 1; z <= N; z++)
        for (long y = 1; y <= N; y++)
            for (long x = 1; x <= N; x++) {
                samples[counter].x = minxyz.x + stepSizes.x * float(x);
                samples[counter].y = minxyz.y + stepSizes.y * float(y);
                samples[counter].z = minxyz.z + stepSizes.z * float(z);
                counter++;
            }

    float     maxmag = 0.0f;
    glm::vec3 vel;
    for (long i = 0; i < totalSamples; i++) {
        int rv = this->GetVelocity(timestamp, samples[i], vel);
        // Among possible return values, 0 is good, and MISSING_VAL isn't too bad,
        // we just need to ignore those values.
        if (rv == flow::MISSING_VAL)
            continue;
        else if (rv != 0)
            return rv;
        else {
            auto mag = glm::length(vel);
            if (mag > maxmag) maxmag = mag;
        }
    }

    // If all sampled locations are missing values or zero values,
    //   we give deltaT an arbitrary value and return a special value.
    if (maxmag == 0.0 || std::isinf(maxmag)) {
        delT = glm::distance(minxyz, maxxyz) / 1000.0;
        return flow::FIELD_ALL_ZERO;
    }

    // Let's dictate that using the maximum velocity FROM OUR SAMPLES
    //   a particle needs 1000 steps (in case of unstructured grids) or
    //   twice the domain dimension (in case of structured grids)
    //   to travel the entire space.
    double desiredNum = 1000.0;    // pre-defined value for unstructured grids

    const auto *grid = _getAGrid(currentTS, VelocityNames[0]);
    const auto *structuredGrid = dynamic_cast<const VAPoR::StructuredGrid *>(grid);
    if (structuredGrid) {
        auto   dims = structuredGrid->GetDimensions();
        double numCellsDiagnal = std::sqrt(double(dims[0] * dims[0] + dims[1] * dims[1] + dims[2] * dims[2]));
        desiredNum = 2.0 * numCellsDiagnal;
    }

    const double actualNum = double(glm::distance(minxyz, maxxyz)) / double(maxmag);
    delT = actualNum / desiredNum;

    return 0;
}

const VAPoR::Grid *VaporField::_getAGrid(size_t timestep, const std::string &varName) const
{
    GridKey key;
    if (_params_locked) {
        // Because in unsteady case, both currentTS and currentTS will be queried,
        // so we do a sanity check here. The assertion will be gone in release mode.
        assert(timestep == _c_currentTS);
        key.Reset(_c_currentTS, _c_refLev, _c_compLev, varName, _c_ext_min, _c_ext_max);
    } else {
        std::vector<double> extMin, extMax;
        _params->GetBox()->GetExtents(extMin, extMax);
        int refLevel = _params->GetRefinementLevel();
        int compLevel = _params->GetCompressionLevel();
        key.Reset(timestep, refLevel, compLevel, varName, extMin, extMax);
    }

    // First check if we have the requested grid in our cache.
    // If it exists, return the grid directly.
    const auto &grid_wrapper = _recentGrids.query(key);
    if (grid_wrapper != nullptr) { return grid_wrapper->grid(); }

    //
    // There's no such grid in our cache!
    // Let's create a new grid by doing one of the three things, and then put it in the cache.
    // 1) create it by ourselves if a ConstantGrid is required, or
    // 2) ask for it from the data manager,
    //

    // Note that we use a lock here, so no two threads querying _datamgr simultaneously.
    const std::lock_guard<std::mutex> lock_gd(_grid_operation_mutex);

    VAPoR::Grid *grid = nullptr;
    if (key.emptyVar()) {
        // In case of an empty variable name, we generate a constantGrid with zeros.
        grid = new VAPoR::ConstantGrid(0.0f, 3);
    } else {
        if (_params_locked) {
            grid = _datamgr->GetVariable(_c_currentTS, varName, _c_refLev, _c_compLev, _c_ext_min, _c_ext_max, true);
        } else {
            std::vector<double> extMin, extMax;
            _params->GetBox()->GetExtents(extMin, extMax);
            int refLevel = _params->GetRefinementLevel();
            int compLevel = _params->GetCompressionLevel();
            grid = _datamgr->GetVariable(timestep, varName, refLevel, compLevel, extMin, extMax, true);
        }
    }

    if (grid == nullptr) {
        Wasp::MyBase::SetErrMsg("Not able to get a grid!");
        return nullptr;
    }

    // Now we have this grid, but also put it in a GridWrapper so
    // 1) it will be properly deleted, and
    // 2) it is stored in our cache, where its ownership is kept.
    auto dim = _datamgr->GetVarTopologyDim(varName);

    if (dim == 1) {
        Wasp::MyBase::SetErrMsg("Variable Dimension Wrong!");
        return nullptr;
    }
    _recentGrids.insert(key, new GridWrapper(grid, _datamgr));
    return grid;
}
