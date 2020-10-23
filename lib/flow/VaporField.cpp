#include "vapor/VaporField.h"
#include "vapor/ConstantGrid.h"
#include "GrownGrid.h"

using namespace flow;

//
// Class GridKey
//
void GridKey::Reset(uint64_t ts, int32_t ref, int32_t comp, std::string var,
                    const std::vector<double> &min,
                    const std::vector<double> &max,
                    float dz) {
    timestep = ts;
    refLev = ref;
    compLev = comp;
    defaultZ = dz;

    for (int i = 0; i < 3; i++) {
        ext_min[i] = 0.0;
        ext_max[i] = 0.0;
    }
    for (int i = 0; i < min.size(); i++)
        ext_min[i] = min[i];
    for (int i = 0; i < max.size(); i++)
        ext_max[i] = max[i];

    varName = std::move(var);
}

bool GridKey::operator==(const GridKey &other) const {
    // String comparison seems to occur most frequently, so put it at the first.
    if (this->varName != other.varName)
        return false;
    if (this->timestep != other.timestep)
        return false;
    if (this->refLev != other.refLev)
        return false;
    if (this->compLev != other.compLev)
        return false;
    if (this->defaultZ != other.defaultZ)
        return false;
    if (this->ext_min != other.ext_min)
        return false;
    if (this->ext_max != other.ext_max)
        return false;

    return true;

    // Note the decision to always record and compare DefaultZ value.
    // In the event of DefaultZ changes, but it's 3D case in essense,
    // the keys will compare as different, and trigger a grid reconstruction.
    // This is a comprimise to avoid very complicated logic, and given that
    // DefaultZ isn't likely to change during a truly 3D case.
}

bool GridKey::emptyVar() const {
    return varName.empty();
}

//
// Class GridWrapper
//
GridWrapper::GridWrapper(const VAPoR::Grid *gp, VAPoR::DataMgr *mp)
    : gridPtr(gp), mgr(mp) {}

GridWrapper::~GridWrapper() {
    if (gridPtr) {
        if (gridPtr->GetType() == "GrownGrid") {
            delete gridPtr; // GrownGrid can unlock itself...
        } else {
            if (mgr) {
                mgr->UnlockGrid(gridPtr);
                delete gridPtr;
            }
        }
    }
}

const VAPoR::Grid *GridWrapper::grid() const {
    return gridPtr;
}

//
// Class VaporField
//
VaporField::VaporField(size_t cache_limit)
    : _recentGrids(cache_limit, false) {}

auto VaporField::LockParams() -> int {
    if (!_isReady())
        return 1;

    // Retrieve param values and put them in the local cache.
    _c_currentTS = _params->GetCurrentTimestep();
    _c_refLev = _params->GetRefinementLevel();
    _c_compLev = _params->GetCompressionLevel();
    _c_vel_mult = _params->GetVelocityMultiplier();

    _params->GetBox()->GetExtents(_c_ext_min, _c_ext_max);

    for (int i = 0; i < 3; i++) {
        _c_velocity_grids[i] = _getAGrid(_c_currentTS, this->VelocityNames[i]);
    }
    _c_scalar_grid = _getAGrid(_c_currentTS, this->ScalarName);

    // Note that if the DefaultZ value is changed by the renderer after LockParams(),
    // cached grids here won't reflect the change.
    // This is due to the that DefaultZ is kept by the Renderer, but not Params.
    // A query directed to _getAGrid() would reflect this change though.

    _params_locked = true;
    return 0;
}

auto VaporField::UnlockParams() -> int {
    _c_currentTS = 0;
    _c_refLev = -2;
    _c_compLev = -2;
    _c_vel_mult = 0.0;
    _c_ext_min.clear();
    _c_ext_max.clear();

    for (int i = 0; i < 3; i++) {
        _c_velocity_grids[i] = nullptr;
    }
    _c_scalar_grid = nullptr;

    _params_locked = false;
    return 0;
}

bool VaporField::InsideVolumeVelocity(float time, const glm::vec3 &pos) const {
    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *grid = nullptr;
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

            if (grid == nullptr)
                return false;
            if (!grid->InsideGrid(coords))
                return false;
        }
    } else // In case of unsteady, we check two time steps
    {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back())
            return false;

        // Then locate the 2 time steps
        size_t floor = 0;
        int rv = LocateTimestamp(time, floor);
        if (rv != 0)
            return false;

        // Then test if pos is inside of time step "floor"
        for (auto &v : VelocityNames) {
            grid = _getAGrid(floor, v);
            if (grid == nullptr)
                return false;
            if (!grid->InsideGrid(coords))
                return false;
        }

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if (time > _timestamps[floor]) {
            for (auto &v : VelocityNames) {
                grid = _getAGrid(floor + 1, v);
                if (grid == nullptr)
                    return false;
                if (!grid->InsideGrid(coords))
                    return false;
            }
        }
    }

    return true;
}

bool VaporField::InsideVolumeScalar(float time, const glm::vec3 &pos) const {
    // When this variable doesn't exist, it doesn't make sense to say if
    // a position is inside of the volume, so simply return true.
    if (ScalarName.empty())
        return true;

    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *grid = nullptr;
    VAssert(_isReady());

    // In case of steady field, we only check a specific time step
    if (IsSteady) {
        if (_params_locked) {
            grid = _c_scalar_grid;
        } else {
            auto currentTS = _params->GetCurrentTimestep();
            grid = _getAGrid(currentTS, ScalarName);
        }
        if (grid == nullptr)
            return false;
        return grid->InsideGrid(coords);
    } else // In case of unsteady field, we check two time steps
    {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back())
            return false;

        // Then locate the 2 time steps
        size_t floor = 0;
        int rv = LocateTimestamp(time, floor);
        if (rv != 0)
            return false;

        // Then test if pos is inside of time step "floor"
        grid = _getAGrid(floor, ScalarName);
        if (grid == nullptr)
            return false;
        if (!grid->InsideGrid(coords))
            return false;

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if (time > _timestamps[floor]) {
            grid = _getAGrid(floor + 1, ScalarName);
            if (grid == nullptr)
                return false;
            if (!grid->InsideGrid(coords))
                return false;
        }

        return true;
    }
}

int VaporField::GetVelocityIntersection(size_t ts, glm::vec3 &minxyz, glm::vec3 &maxxyz) const {
    const VAPoR::Grid *grid = nullptr;
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

int VaporField::GetVelocity(float time, const glm::vec3 &pos, glm::vec3 &velocity) const {
    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *grid = nullptr;

    // Retrieve the missing value and velocity multiplier
    glm::vec3 missingV(0.0f); // stores missing values for 3 velocity variables
    velocity = glm::vec3(0.0f);

    if (IsSteady) {
        for (int i = 0; i < 3; i++) {
            if (_params_locked) {
                grid = _c_velocity_grids[i];
            } else {
                auto currentTS = _params->GetCurrentTimestep();
                grid = _getAGrid(currentTS, VelocityNames[i]);
            }
            if (grid == nullptr)
                return GRID_ERROR;
            velocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal(velocity, missingV);
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
        if (time < _timestamps.front() || time > _timestamps.back())
            return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS = 0;
        int rv = LocateTimestamp(time, floorTS);
        VAssert(rv == 0);

        // Find the velocity values at floor time step
        glm::vec3 floorVelocity, ceilVelocity;
        for (int i = 0; i < 3; i++) {
            grid = _getAGrid(floorTS, VelocityNames[i]);
            if (grid == nullptr)
                return GRID_ERROR;
            floorVelocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal(floorVelocity, missingV);
        if (glm::any(hasMissing)) {
            return MISSING_VAL;
        }

        if (time == _timestamps[floorTS]) {
            velocity = floorVelocity * mult;
            return 0;
        } else // Find the velocity values at the ceiling time step
        {
            // We need to make sure there aren't duplicate time stamps
            VAssert(_timestamps[floorTS + 1] > _timestamps[floorTS]);
            for (int i = 0; i < 3; i++) {
                grid = _getAGrid(floorTS + 1, VelocityNames[i]);
                if (grid == nullptr)
                    return GRID_ERROR;
                ceilVelocity[i] = grid->GetValue(coords);
                missingV[i] = grid->GetMissingValue();
            }
            hasMissing = glm::equal(ceilVelocity, missingV);
            if (glm::any(hasMissing)) {
                return MISSING_VAL;
            }

            float weight = (time - _timestamps[floorTS]) /
                           (_timestamps[floorTS + 1] - _timestamps[floorTS]);
            velocity = glm::mix(floorVelocity, ceilVelocity, weight) * mult;
            return 0;
        }
    } // end of unsteady condition
}

int VaporField::GetScalar(float time, const glm::vec3 &pos, float &scalar) const {
    // When this variable doesn't exist, it doesn't make sense to get a scalar value
    // from it, so just return that fact.
    if (ScalarName.empty())
        return NO_FIELD_YET;

    const std::array<double, 3> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *grid = nullptr;

    if (IsSteady) {
        if (_params_locked) {
            grid = _c_scalar_grid;
        } else {
            auto currentTS = _params->GetCurrentTimestep();
            grid = _getAGrid(currentTS, ScalarName);
        }
        if (grid == nullptr)
            return GRID_ERROR;
        scalar = grid->GetValue(coords);
        if (scalar == grid->GetMissingValue()) {
            return MISSING_VAL;
        } else {
            return 0;
        }
    } else {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back())
            return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS = 0;
        int rv = LocateTimestamp(time, floorTS);
        VAssert(rv == 0);
        grid = _getAGrid(floorTS, ScalarName);
        if (grid == nullptr)
            return GRID_ERROR;
        float floorScalar = grid->GetValue(coords);
        if (floorScalar == grid->GetMissingValue()) {
            return MISSING_VAL;
        }

        if (time == _timestamps[floorTS]) {
            scalar = floorScalar;
            return 0;
        } else {
            grid = _getAGrid(floorTS + 1, ScalarName);
            if (grid == nullptr)
                return GRID_ERROR;
            float ceilScalar = grid->GetValue(coords);
            if (ceilScalar == grid->GetMissingValue()) {
                return MISSING_VAL;
            } else {
                float weight = (time - _timestamps[floorTS]) /
                               (_timestamps[floorTS + 1] - _timestamps[floorTS]);
                scalar = glm::mix(floorScalar, ceilScalar, weight);
                return 0;
            }
        }
    } // end of unsteady condition
}

bool VaporField::_isReady() const {
    if (!_datamgr)
        return false;
    if (!_params)
        return false;

    return true;
}

void VaporField::AssignDataManager(VAPoR::DataMgr *dmgr) {
    _datamgr = dmgr;

    // Make a copy of the timestamps from the new data manager
    const auto &timeCoords = dmgr->GetTimeCoordinates();
    _timestamps.resize(timeCoords.size());
    for (size_t i = 0; i < timeCoords.size(); i++)
        _timestamps[i] = timeCoords[i];
}

void VaporField::UpdateParams(const VAPoR::FlowParams *p) {
    _params = p;

    // Update properties of this Field
    IsSteady = p->GetIsSteady();
    ScalarName = p->GetColorMapVariableName();
    auto velNames = p->GetFieldVariableNames();
    for (int i = 0; i < 3; i++) {
        if (i < velNames.size())
            VelocityNames[i] = velNames.at(i);
        else
            VelocityNames[i] = ""; // make sure it keeps an empty string,
    }                              // instead of whatever left from before.
}

int VaporField::LocateTimestamp(float time, size_t &floor) const {
    if (_timestamps.empty())
        return TIME_ERROR;
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

int VaporField::GetNumberOfTimesteps() const {
    return _timestamps.size();
}

int VaporField::CalcDeltaTFromCurrentTimeStep(float &delT) const {
    VAssert(_isReady());

    // This function is called only one-time before advection starts,
    // so don't worry about parameter locking here.
    const auto currentTS = _params->GetCurrentTimestep();
    const auto timestamp = _timestamps.at(currentTS);

    // Let's find the intersection of 3 velocity components.
    glm::vec3 minxyz(0.0f), maxxyz(0.0f);
    int rv = this->GetVelocityIntersection(currentTS, minxyz, maxxyz);
    if (rv != 0)
        return rv;

    // Let's make sure the max is greater than or equal to min
    const auto invalid = glm::lessThan(maxxyz, minxyz);
    if (glm::any(invalid)) {
        Wasp::MyBase::SetErrMsg("One of the selected volume dimension is zero!");
        return GRID_ERROR;
    }

    // Let's sample some locations along each dimension.
    const long N = 10; // Num of samples along each axis
    const long totalSamples = N * N * N;
    const glm::vec3 numOfSteps(float(N + 1));
    const glm::vec3 stepSizes = (maxxyz - minxyz) / numOfSteps;
    glm::vec3 samples[totalSamples];
    long counter = 0;
    for (long z = 1; z <= N; z++)
        for (long y = 1; y <= N; y++)
            for (long x = 1; x <= N; x++) {
                samples[counter].x = minxyz.x + stepSizes.x * float(x);
                samples[counter].y = minxyz.y + stepSizes.y * float(y);
                samples[counter].z = minxyz.z + stepSizes.z * float(z);
                counter++;
            }

    // Let's find the maximum velocity on these sampled locations
    // Note that we want the raw velocity, which will be the value
    // returned by GetVelocity() divided by the velocity multiplier.
    float mult = _params->GetVelocityMultiplier();
    if (mult == 0.0f)
        mult = 1.0f;
    const float mult1o = 1.0f / mult;
    float maxmag = 0.0;
    glm::vec3 vel;
    for (long i = 0; i < totalSamples; i++) {
        int rv = this->GetVelocity(timestamp, samples[i], vel);
        if (rv != 0)
            return rv;
        vel *= mult1o; // Restore the raw velocity values
        auto mag = glm::length(vel);
        if (mag > maxmag)
            maxmag = mag;
    }

    // Let's dictate that using the maximum velocity FROM OUR SAMPLES
    // a particle needs 500 steps to travel the entire space.
    const float desiredNum = 500.0f;
    const float actualNum = glm::distance(minxyz, maxxyz) / maxmag;
    delT = actualNum / desiredNum;

    return 0;
}

const VAPoR::Grid *VaporField::_getAGrid(size_t timestep, const std::string &varName) const {
    GridKey key;
    if (_params_locked) {
        // Because in unsteady case, both currentTS and currentTS will be queried,
        // so we do a sanity check here. The assertion will be gone in release mode.
        assert(timestep == _c_currentTS);
        key.Reset(_c_currentTS, _c_refLev, _c_compLev, varName,
                  _c_ext_min, _c_ext_max, this->DefaultZ);
    } else {
        std::vector<double> extMin, extMax;
        _params->GetBox()->GetExtents(extMin, extMax);
        int refLevel = _params->GetRefinementLevel();
        int compLevel = _params->GetCompressionLevel();
        key.Reset(timestep, refLevel, compLevel, varName, extMin, extMax, this->DefaultZ);
    }

    // First check if we have the requested grid in our cache.
    // If it exists, return the grid directly.
    const auto &grid_wrapper = _recentGrids.query(key);
    if (grid_wrapper != nullptr) {
        return grid_wrapper->grid();
    }

    //
    // There's no such grid in our cache!
    // Let's create a new grid by doing one of the three things, and then put it in the cache.
    // 1) create it by ourselves if a ConstantGrid is required, or
    // 2) ask for it from the data manager,
    // 3) query a 2D grid and grow it to be a GrownGrid.
    //

    // Note that we use a lock here, so no two threads querying _datamgr simultaneously.
    const std::lock_guard<std::mutex> lock_gd(_grid_operation_mutex);

    VAPoR::Grid *grid = nullptr;
    if (key.emptyVar()) {
        // In case of an empty variable name, we generate a constantGrid with zeros.
        grid = new VAPoR::ConstantGrid(0.0f, 3);
    } else {
        if (_params_locked) {
            grid = _datamgr->GetVariable(_c_currentTS, varName, _c_refLev, _c_compLev,
                                         _c_ext_min, _c_ext_max, true);
        } else {
            std::vector<double> extMin, extMax;
            _params->GetBox()->GetExtents(extMin, extMax);
            int refLevel = _params->GetRefinementLevel();
            int compLevel = _params->GetCompressionLevel();
            grid = _datamgr->GetVariable(timestep, varName, refLevel, compLevel,
                                         extMin, extMax, true);
        }
    }

    if (grid == nullptr) {
        Wasp::MyBase::SetErrMsg("Not able to get a grid!");
        return nullptr;
    }

    // Now we have this grid, but also put it in a GridWrapper so
    // 1) it will be properly deleted, and
    // 2) it is stored in our cache, where its ownership is kept.
    // We also make it become a GrownGrid if it's 2D in nature.
    auto dim = _datamgr->GetVarTopologyDim(varName);
    if (dim == 3 || dim == 0) // dim == 0 happens when varName is empty.
    {
        _recentGrids.insert(key, new GridWrapper(grid, _datamgr));
        return grid;
    } else if (dim == 2) {
        VAPoR::GrownGrid *ggrid = new VAPoR::GrownGrid(grid, _datamgr, DefaultZ);
        _recentGrids.insert(key, new GridWrapper(ggrid, _datamgr));
        return ggrid;
    } else {
        Wasp::MyBase::SetErrMsg("Variable Dimension Wrong!");
        return nullptr;
    }
}
