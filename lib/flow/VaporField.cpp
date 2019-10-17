#include "vapor/VaporField.h"
#include <sstream>

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
        for (auto &v : VelocityNames)
            if (!v.empty()) {
                grid = _getAGrid(currentTS, v);
                VAssert(grid);
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

        // Second test if pos is inside of time step "floor"
        for (auto &v : VelocityNames)
            if (!v.empty()) {
                grid = _getAGrid(floor, v);
                VAssert(grid);
                if (!grid->InsideGrid(coords)) return false;
            }

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if (time > _timestamps[floor]) {
            for (auto &v : VelocityNames)
                if (!v.empty()) {
                    grid = _getAGrid(floor + 1, v);
                    VAssert(grid);
                    if (!grid->InsideGrid(coords)) return false;
                }
        }
    }

    return true;
}

bool VaporField::InsideVolumeScalar(float time, const glm::vec3 &pos) const
{
    if (ScalarName.empty()) return false;

    std::string               scalarname = ScalarName;    // const requirement...
    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;
    VAssert(_isReady());

    // In case of steady field, we only check a specific time step
    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        grid = _getAGrid(currentTS, scalarname);
        VAssert(grid);
        if (!grid->InsideGrid(coords)) return false;
    } else    // we check two time steps
    {
        // First check if the query time is within range
        if (time < _timestamps.front() || time > _timestamps.back()) return false;

        // Then locate the 2 time steps
        size_t floor = 0;
        int    rv = LocateTimestamp(time, floor);
        if (rv != 0) return false;

        // Second test if pos is inside of time step "floor"
        grid = _getAGrid(floor, scalarname);
        VAssert(grid);
        if (!grid->InsideGrid(coords)) return false;

        // If time is larger than _timestamps[floor], we also need to test _timestamps[floor+1]
        if (time > _timestamps[floor]) {
            grid = _getAGrid(floor + 1, scalarname);
            VAssert(grid);
            if (!grid->InsideGrid(coords)) return false;
        }
    }

    return true;
}

void VaporField::GetFirstStepVelocityIntersection(glm::vec3 &minxyz, glm::vec3 &maxxyz)
{
    const VAPoR::Grid * grid = nullptr;
    std::vector<double> min[3], max[3];

    for (int i = 0; i < 3; i++) {
        auto &varname = VelocityNames[i];
        grid = _getAGrid(0, varname);
        VAssert(grid);
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
}

int VaporField::GetVelocity(float time, const glm::vec3 &pos, glm::vec3 &velocity, bool checkInsideVolume) const
{
    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;

    // First make sure the query positions are inside of the volume
    if (checkInsideVolume)
        if (!InsideVolumeVelocity(time, pos)) return OUT_OF_FIELD;

    // Retrieve the missing value velocity multiplier
    const float mult = _params->GetVelocityMultiplier();
    glm::vec3   missingV;

    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        for (int i = 0; i < 3; i++) {
            auto &varname = VelocityNames[i];
            grid = _getAGrid(currentTS, varname);
            VAssert(grid);
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
            auto &varname = VelocityNames[i];
            grid = _getAGrid(floorTS, varname);
            VAssert(grid);
            floorVelocity[i] = grid->GetValue(coords);
            missingV[i] = grid->GetMissingValue();
        }
        auto hasMissing = glm::equal(floorVelocity, missingV);
        if (glm::any(hasMissing)) {
            velocity = glm::vec3(0.0f);
            return 0;
        }

        // Find the velocity values at the ceiling time step
        if (time == _timestamps[floorTS])
            velocity = floorVelocity * mult;
        else {
            // We need to make sure there aren't duplicate time stamps
            VAssert(_timestamps[floorTS + 1] > _timestamps[floorTS]);
            for (int i = 0; i < 3; i++) {
                auto &varname = VelocityNames[i];
                grid = _getAGrid(floorTS + 1, varname);
                VAssert(grid);
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
    if (ScalarName.empty()) return NO_FIELD_YET;
    if (checkInsideVolume)
        if (!InsideVolumeScalar(time, pos)) return OUT_OF_FIELD;

    std::string scalarname = ScalarName;    // const requirement...

    const std::vector<double> coords{pos.x, pos.y, pos.z};
    const VAPoR::Grid *       grid = nullptr;

    if (IsSteady) {
        size_t currentTS = _params->GetCurrentTimestep();
        grid = _getAGrid(currentTS, scalarname);
        VAssert(grid);
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
        grid = _getAGrid(floorTS, scalarname);
        VAssert(grid);
        float floorScalar = grid->GetValue(coords);
        if (floorScalar == grid->GetMissingValue()) {
            scalar = 0.0f;
            return 0;
        }

        if (time == _timestamps[floorTS])
            scalar = floorScalar;
        else {
            grid = _getAGrid(floorTS + 1, scalarname);
            VAssert(grid);
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
    for (int i = 0; i < 3 && i < velNames.size(); i++) VelocityNames[i] = velNames.at(i);
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

const VAPoR::Grid *VaporField::_getAGrid(size_t timestep, const std::string &varName) const
{
    // First check if we have the requested grid in our cache.
    // If it exists, return the grid directly.
    std::vector<double> extMin, extMax;
    _params->GetBox()->GetExtents(extMin, extMax);
    int         refLevel = _params->GetRefinementLevel();
    int         compLevel = _params->GetCompressionLevel();
    std::string key = _paramsToString(timestep, varName, refLevel, compLevel, extMin, extMax);
    if (_recentGrids.contains(key)) { return _recentGrids.find(key)->grid(); }

    // There's no such grid in our cache! Let's ask for it from the data manager,
    // and then keep it in our cache!
    VAPoR::Grid *grid = _datamgr->GetVariable(timestep, varName, refLevel, compLevel, extMin, extMax, true);
    if (grid == nullptr) {
        Wasp::MyBase::SetErrMsg("Not able to get a grid!");
        return nullptr;
    }

    // Now we have this grid, but also put it in a GridWrapper so
    // 1) it will be properly deleted, and 2) it is stored in our cache
    // where its ownership is kept.
    _recentGrids.insert(key, new GridWrapper(grid, _datamgr));

    return grid;
}

std::string VaporField::_paramsToString(size_t currentTS, const std::string &var, int refLevel, int compLevel, const std::vector<double> &min, const std::vector<double> &max) const
{
    std::string        space("  ");
    std::ostringstream oss;
    oss << currentTS << space << var << space << refLevel << space << compLevel << space;
    for (size_t i = 0; i < min.size(); i++) oss << min[i] << space;
    for (size_t i = 0; i < max.size(); i++) oss << max[i] << space;
    return oss.str();
}
