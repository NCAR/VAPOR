#include <sstream>

#include "vapor/VaporField.h"
#include "vapor/ConstantGrid.h"
#include "GrownGrid.h"

using namespace flow;

// Constructor
VaporField::VaporField(size_t cache_limit) : _recentGrids(cache_limit) {}

bool VaporField::InsideVolumeVelocity(float time, const glm::vec3 &pos) const
{
    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;
    VAssert(_isReady());

    // In case of steady field, we only check a specific time step
    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        for (auto &v : VelocityNames) {
            grid = _getAGrid(currentTS, v);
            if (grid == nullptr) return false;
            if (!grid->InsideGrid(coords)) return false;
        }
    } else    // we check two time steps
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

bool VaporField::InsideVolumeScalar(float time, const glm::vec3 &pos) const
{
    // When this variable doesn't exist, it doesn't make sense to say if
    // a position is inside of the volume, so simply return true.
    if (ScalarName.empty()) return true;

    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;
    VAssert(_isReady());

    // In case of steady field, we only check a specific time step
    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        grid = _getAGrid(currentTS, ScalarName);
        if (grid == nullptr) return false;
        if (!grid->InsideGrid(coords)) return false;
    } else    // we check two time steps
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
    }

    return true;
}

int VaporField::GetVelocityIntersection(size_t ts, glm::vec3 &minxyz, glm::vec3 &maxxyz) const
{
    const VAPoR::Grid * grid = nullptr;
    std::vector<double> min[3], max[3];

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

int VaporField::GetVelocity(float time, const glm::vec3 &pos, glm::vec3 &velocity, bool checkInsideVolume) const
{
    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;

    // First make sure the query positions are inside of the volume
    if (checkInsideVolume)
        if (!InsideVolumeVelocity(time, pos)) return OUT_OF_FIELD;

    // Retrieve the missing value and velocity multiplier
    const float mult = _params->GetVelocityMultiplier();
    glm::vec3   missingV(0.0f);    // stores missing values for 3 velocity variables
    velocity = glm::vec3(0.0f);

    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        for (int i = 0; i < 3; i++) {
            grid = _getAGrid(currentTS, VelocityNames[i]);
            if (grid == nullptr) return GRID_ERROR;
            velocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal(velocity, missingV);
        if (glm::any(hasMissing))
            velocity = glm::vec3(0.0f);
        else
            velocity *= mult;
    } else {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back()) return TIME_ERROR;

        // Then we locate the floor time step
        size_t floorTS = 0;
        int    rv = LocateTimestamp(time, floorTS);
        VAssert(rv == 0);

        // Find the velocity values at floor time step
        glm::vec3 floorVelocity, ceilVelocity;
        for (int i = 0; i < 3; i++) {
            grid = _getAGrid(floorTS, VelocityNames[i]);
            if (grid == nullptr) return GRID_ERROR;
            floorVelocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal(floorVelocity, missingV);
        if (glm::any(hasMissing)) {
            velocity = glm::vec3(0.0f);
            return 0;
        }

        if (time == _timestamps[floorTS]) {
            velocity = floorVelocity * mult;
        } else    // Find the velocity values at the ceiling time step
        {
            // We need to make sure there aren't duplicate time stamps
            VAssert(_timestamps[floorTS + 1] > _timestamps[floorTS]);
            for (int i = 0; i < 3; i++) {
                grid = _getAGrid(floorTS + 1, VelocityNames[i]);
                if (grid == nullptr) return GRID_ERROR;
                ceilVelocity[i] = grid->GetValue(coords);
                missingV[i] = grid->GetMissingValue();
            }
            hasMissing = glm::equal(ceilVelocity, missingV);
            if (glm::any(hasMissing)) {
                velocity = glm::vec3(0.0f);
                return 0;
            }

            float weight = (time - _timestamps[floorTS]) / (_timestamps[floorTS + 1] - _timestamps[floorTS]);
            velocity = glm::mix(floorVelocity, ceilVelocity, weight) * mult;
        }
    }

    return 0;
}

int VaporField::GetScalar(float time, const glm::vec3 &pos, float &scalar, bool checkInsideVolume) const
{
    // When this variable doesn't exist, it doesn't make sense to get a scalar value
    // from it, so just return that fact.
    if (ScalarName.empty()) return NO_FIELD_YET;
    if (checkInsideVolume)
        if (!InsideVolumeScalar(time, pos)) return OUT_OF_FIELD;

    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;

    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        grid = _getAGrid(currentTS, ScalarName);
        if (grid == nullptr) return GRID_ERROR;
        float gridV = grid->GetValue(coords);
        if (gridV == grid->GetMissingValue())
            scalar = 0.0f;
        else
            scalar = gridV;
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
        if (floorScalar == grid->GetMissingValue()) {
            scalar = 0.0f;
            return 0;
        }

        if (time == _timestamps[floorTS])
            scalar = floorScalar;
        else {
            grid = _getAGrid(floorTS + 1, ScalarName);
            if (grid == nullptr) return GRID_ERROR;
            float ceilScalar = grid->GetValue(coords);
            if (ceilScalar == grid->GetMissingValue()) {
                scalar = 0.0f;
                return 0;
            }
            float weight = (time - _timestamps[floorTS]) / (_timestamps[floorTS + 1] - _timestamps[floorTS]);
            scalar = glm::mix(floorScalar, ceilScalar, weight);
        }
    }

    return 0;
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

    // Update properties of this Field
    IsSteady = p->GetIsSteady();
    ScalarName = p->GetColorMapVariableName();
    auto velNames = p->GetFieldVariableNames();
    for (int i = 0; i < 3; i++) {
        if (i < velNames.size())
            VelocityNames[i] = velNames.at(i);
        else
            VelocityNames[i] = "";    // make sure it keeps an empty string,
    }                                 // instead of whatever left from before.
}

int VaporField::LocateTimestamp(float time, size_t &floor) const
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

int VaporField::CalcDeltaTFromCurrentTimeStep(float &delT) const
{
    VAssert(_isReady());
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

    // Let's find the maximum velocity on these sampled locations
    // Note that we want the raw velocity, which will be the value
    // returned by GetVelocity() divided by the velocity multiplier.
    float mult = _params->GetVelocityMultiplier();
    if (mult == 0.0f) mult = 1.0f;
    const float mult1o = 1.0f / mult;
    float       maxmag = 0.0;
    glm::vec3   vel;
    for (long i = 0; i < totalSamples; i++) {
        int rv = this->GetVelocity(timestamp, samples[i], vel, false);
        if (rv != 0) return rv;
        vel *= mult1o;    // Restore the raw velocity values
        auto mag = glm::length(vel);
        if (mag > maxmag) maxmag = mag;
    }

    // Let's dictate that using the maximum velocity FROM OUR SAMPLES
    // a particle needs 500 steps to travel the entire space.
    const float desiredNum = 500.0f;
    const float actualNum = glm::distance(minxyz, maxxyz) / maxmag;
    delT = actualNum / desiredNum;

    return 0;
}

const VAPoR::Grid *VaporField::_getAGrid(size_t timestep, const std::string &varName) const
{
    // First check if we have the requested grid in our cache.
    // If it exists, return the grid directly.
    std::vector<double> extMin, extMax;
    _params->GetBox()->GetExtents(extMin, extMax);
    int         refLevel = _params->GetRefinementLevel();
    int         compLevel = _params->GetCompressionLevel();
    std::string key = _paramsToString(timestep, varName, refLevel, compLevel, extMin, extMax);
    if (_recentGrids.contains(key)) {
        const VAPoR::Grid *grid = _recentGrids.find(key)->grid();
        // Is this a GrownGrid?
        const VAPoR::GrownGrid *ggrid = dynamic_cast<const VAPoR::GrownGrid *>(grid);
        if (ggrid == nullptr)
            return grid;
        else    // Need to test if the DefaultZ value has changed.
        {
            if (ggrid->GetDefaultZ() == this->DefaultZ)    // DefaultZ is the same!
                return grid;
            // If DefaultZ has changed, a new GrownGrid will be constructed
            // with the new DefaultZ value.
        }
    }

    //
    // There's no such grid in our cache!
    // Let's do one of the three and then keep it in the cache.
    // 1) create it by ourselves if a ConstantGrid is required, or
    // 2) ask for it from the data manager,
    // 3) query a 2D grid and grow it to be a GrownGrid.
    //
    VAPoR::Grid *grid = nullptr;
    if (key == _constantGridZero)    // need a ConstantGrid
    {
        grid = new VAPoR::ConstantGrid(0.0f, 3);
    } else {
        grid = _datamgr->GetVariable(timestep, varName, refLevel, compLevel, extMin, extMax, true);
    }
    if (grid == nullptr) {
        Wasp::MyBase::SetErrMsg("Not able to get a grid!");
        return nullptr;
    }

    // Now we have this grid, but also put it in a GridWrapper so
    // 1) it will be properly deleted, and 2) it is stored in our cache
    // where its ownership is kept.
    // We also make it become a GrownGrid if it's 2D in nature.
    int dim = _datamgr->GetVarTopologyDim(varName);
    if (dim == 3 || dim == 0)    // dim == 0 happens when varName is empty.
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

std::string VaporField::_paramsToString(size_t currentTS, const std::string &var, int refLevel, int compLevel, const std::vector<double> &min, const std::vector<double> &max) const
{
    // In case of an empty variable name, we generate a string that represents a
    // ConstantGrid with zeros, no matter what other parameters are.
    if (var.empty()) {
        return _constantGridZero;
    } else {
        std::ostringstream oss;
        std::string        space("  ");
        oss << currentTS << space << var << space << refLevel << space << compLevel << space;
        for (size_t i = 0; i < min.size(); i++) oss << min[i] << space;
        for (size_t i = 0; i < max.size(); i++) oss << max[i] << space;
        return oss.str();
    }
}
