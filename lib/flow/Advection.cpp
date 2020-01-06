#include <iostream>
#include "vapor/Advection.h"
#include <fstream>

using namespace flow;

// Constructor;
Advection::Advection() : _lowerAngle(3.0f), _upperAngle(15.0f)
{
    _lowerAngleCos = glm::cos(glm::radians(_lowerAngle));
    _upperAngleCos = glm::cos(glm::radians(_upperAngle));

    for (int i = 0; i < 3; i++) { _isPeriodic[i] = false; }
}

void Advection::UseSeedParticles(const std::vector<Particle> &seeds)
{
    _streams.clear();
    _streams.resize(seeds.size());
    for (size_t i = 0; i < seeds.size(); i++) _streams[i].push_back(seeds[i]);

    _separatorCount.resize(seeds.size());
    for (auto &e : _separatorCount) e = 0;
}

int Advection::CheckReady() const
{
    for (const auto &s : _streams) {
        if (s.size() < 1) return NO_SEED_PARTICLE_YET;
    }

    return 0;
}

int Advection::AdvectOneStep(Field *velocity, float deltaT, ADVECTION_METHOD method)
{
    int ready = CheckReady();
    if (ready != 0) return ready;

    bool   happened = false;
    size_t streamIdx = 0;
    for (auto &s : _streams)    // Process one stream at a time
    {
        // Check if the particle is inside of the volume.
        // Also wrap it along periodic dimensions if enabled.
        if (!velocity->InsideVolumeVelocity(s.back().time, s.back().location)) {
            auto &p0 = s.back();
            // Attempt to apply periodicity
            bool locChanged = false;
            auto loc = p0.location;
            for (int i = 0; i < 3; i++)    // correct coordinates in each periodic dimension
            {
                if (_isPeriodic[i]) {
                    loc[i] = _applyPeriodic(loc[i], _periodicBounds[i][0], _periodicBounds[i][1]);
                    locChanged = true;
                }
            }

            if (!locChanged)    // no dimension is periodic
                continue;       // skip this particle, since it's out of the volume

            // If the new location comes inside volume, then we do these things:
            // 1) Update the location of p0 to represent the wrapped result.
            // 2) Insert a separator particle before p0.
            if (velocity->InsideVolumeVelocity(p0.time, loc)) {
                p0.location = loc;

                Particle separator;
                separator.SetSpecial(true);
                auto itr = s.end();    // Should use const iterator here,
                                       // but gcc-4.8 on CentOS7 doesn't support...
                --itr;                 // insert before the last element, p0
                s.insert(itr, std::move(separator));
                _separatorCount[streamIdx]++;
            } else
                continue;    // skip this particle, since it's out of the volume

        }    // end of if condition

        const auto &past0 = s.back();
        float       dt = deltaT;
        if (s.size() > 2)    // If there are at least 3 particles in the stream and
        {                    // neither is a separator, we also adjust *dt*
            const auto &past1 = s[s.size() - 2];
            const auto &past2 = s[s.size() - 3];
            if ((!past1.IsSpecial()) && (!past2.IsSpecial())) {
                float mindt = deltaT / 20.0f, maxdt = deltaT * 20.0f;
                dt = past0.time - past1.time;    // step size used by last integration
                dt *= _calcAdjustFactor(past2, past1, past0);
                if (dt > 0)    // integrate forward
                    dt = glm::clamp(dt, mindt, maxdt);
                else    // integrate backward
                    dt = glm::clamp(dt, maxdt, mindt);
            }
        }

        Particle p1;
        int      rv = 0;
        switch (method) {
        case ADVECTION_METHOD::EULER: rv = _advectEuler(velocity, past0, dt, p1); break;
        case ADVECTION_METHOD::RK4: rv = _advectRK4(velocity, past0, dt, p1); break;
        }
        if (rv != 0)    // Advection wasn't successful for some reason...
            continue;
        else    // Advection successful, keep the new particle.
        {
            // printf("old: (%f, %f, %f),  new: (%f, %f, %f)\n", past0.location.x, past0.location.y,
            //       past0.location.z, p1.location.x, p1.location.y, p1.location.z );
            happened = true;
            s.push_back(std::move(p1));
        }

        streamIdx++;

    }    // end of for loop

    if (happened)
        return ADVECT_HAPPENED;
    else
        return 0;
}

int Advection::AdvectTillTime(Field *velocity, float deltaT, float targetT, ADVECTION_METHOD method)
{
    int ready = CheckReady();
    if (ready != 0) return ready;

    bool   happened = false;
    size_t streamIdx = 0;
    for (auto &s : _streams)    // Process one stream at a time
    {
        Particle p0 = s.back();    // Start from the last particle in this stream
        while (p0.time < targetT) {
            // Check if the particle is inside of the volume.
            // Wrap it along periodic dimensions if applicable.
            if (!velocity->InsideVolumeVelocity(p0.time, p0.location)) {
                bool locChanged = false;
                auto itr = s.end();
                --itr;    // pointing to the last element
                auto loc = itr->location;
                for (int i = 0; i < 3; i++) {
                    if (_isPeriodic[i]) {
                        loc[i] = _applyPeriodic(loc[i], _periodicBounds[i][0], _periodicBounds[i][1]);
                        locChanged = true;
                    }
                }
                if (!locChanged)    // no dimension is periodic
                    break;          // break the while loop

                // See if the new location is inside of the volume
                if (velocity->InsideVolumeVelocity(itr->time, loc)) {
                    itr->location = loc;
                    p0 = *itr;    // p0 is equal to the wrapped particle

                    Particle separator;
                    separator.SetSpecial(true);
                    s.insert(itr, std::move(separator));
                    _separatorCount[streamIdx]++;
                } else
                    break;    // break the while loop

            }    // Finish of the if condition

            float dt = deltaT;
            if (s.size() > 2)    // If there are at least 3 particles in the stream,
            {                    // we also adjust *dt*
                float mindt = deltaT / 20.0f, maxdt = deltaT * 20.0f;
                maxdt = glm::min(maxdt, targetT - p0.time);
                const auto &past1 = s[s.size() - 2];
                const auto &past2 = s[s.size() - 3];
                if ((!past1.IsSpecial()) && (!past2.IsSpecial())) {
                    dt = p0.time - past1.time;    // step size used by last integration
                    dt *= _calcAdjustFactor(past2, past1, p0);
                    dt = glm::clamp(dt, mindt, maxdt);
                }
            }

            Particle p1;
            int      rv = 0;
            switch (method) {
            case ADVECTION_METHOD::EULER: rv = _advectEuler(velocity, p0, dt, p1); break;
            case ADVECTION_METHOD::RK4: rv = _advectRK4(velocity, p0, dt, p1); break;
            }
            if (rv != 0)    // Advection wasn't successful for some reason...
            {
                break;
            } else    // Advection successful, keep the new particle.
            {
                happened = true;
                s.push_back(p1);
                p0 = std::move(p1);
            }
        }    // Finish the while loop to advect one particle to a time

        streamIdx++;

    }    // Finish the for loop to advect all particles to a time

    if (happened)
        return ADVECT_HAPPENED;
    else
        return 0;
}

int Advection::CalculateParticleValues(Field *scalar, bool skipNonZero)
{
    size_t mostSteps = 0;
    for (const auto &s : _streams)
        if (s.size() > mostSteps) mostSteps = s.size();

    // Color step i of all particles, and then move on to the next step
    for (size_t i = 0; i < mostSteps; i++) {
        for (auto &s : _streams) {
            if (i < s.size()) {
                auto &p = s[i];
                // Skip this particle if it's a separator
                if (p.IsSpecial()) continue;
                // Do not evaluate this particle if its value is non-zero
                if (skipNonZero && p.value != 0.0f) continue;
                float value;
                int   rv = scalar->GetScalar(p.time, p.location, value, false);
                if (rv == 0)            // The end of a stream could be outside of the volume,
                    p.value = value;    // so let's only color it when the return value is 0.
            }
        }
    }

    return 0;
}

int Advection::CalculateParticleProperties(Field *scalar)
{
    size_t mostSteps = 0;
    for (const auto &s : _streams)
        if (s.size() > mostSteps) mostSteps = s.size();

    for (size_t i = 0; i < mostSteps; i++) {
        for (auto &s : _streams) {
            if (i < s.size()) {
                auto &p = s[i];
                float value;
                int   rv = scalar->GetScalar(p.time, p.location, value, false);
                if (rv == 0)                    // A particle could be out of the volume, so we only
                    p.AttachProperty(value);    // attach property when returns 0.
            }
        }
    }

    return 0;
}

int Advection::_advectEuler(Field *velocity, const Particle &p0, float dt, Particle &p1) const
{
    glm::vec3 v0;
    int       rv = velocity->GetVelocity(p0.time, p0.location, v0, false);
    if (rv != 0) return rv;
    p1.location = p0.location + dt * v0;
    p1.time = p0.time + dt;
    return 0;
}

int Advection::_advectRK4(Field *velocity, const Particle &p0, float dt, Particle &p1) const
{
    glm::vec3 k1, k2, k3, k4;
    float     dt2 = dt * 0.5f;
    int       rv;
    rv = velocity->GetVelocity(p0.time, p0.location, k1, false);
    if (rv != 0) return rv;
    rv = velocity->GetVelocity(p0.time + dt2, p0.location + dt2 * k1, k2, false);
    if (rv != 0) return rv;
    rv = velocity->GetVelocity(p0.time + dt2, p0.location + dt2 * k2, k3, false);
    if (rv != 0) return rv;
    rv = velocity->GetVelocity(p0.time + dt, p0.location + dt * k3, k4, false);
    if (rv != 0) return rv;
    p1.location = p0.location + dt / 6.0f * (k1 + 2.0f * (k2 + k3) + k4);
    p1.time = p0.time + dt;
    return 0;
}

float Advection::_calcAdjustFactor(const Particle &p2, const Particle &p1, const Particle &p0) const
{
    glm::vec3 p2p1 = p1.location - p2.location;
    glm::vec3 p1p0 = p0.location - p1.location;
    float     denominator = glm::length(p2p1) * glm::length(p1p0);
    float     cosine;
    if (denominator < 1e-7)
        return 1.0f;
    else
        cosine = glm::dot(p2p1, p1p0) / denominator;

    if (cosine > _lowerAngleCos)    // Less than "_lowerAngle" degrees
        return 1.25f;
    else if (cosine < _upperAngleCos)    // More than "_upperAngle" degrees
        return 0.5f;
    else
        return 1.0f;
}

std::FILE *Advection::_prepareFileWrite(const std::string &filename, bool append) const
{
    if (filename.empty()) return nullptr;

    FILE *f = nullptr;
    if (append) {
        f = std::fopen(filename.c_str(), "a");
    } else {
        f = std::fopen(filename.c_str(), "w");
    }
    if (f == nullptr) return nullptr;

    std::fprintf(f, "%s\n", "# This file could be plotted by Gnuplot using the following command:");
    std::fprintf(f, "%s\n\n", "# splot output_filename u 1:2:3 w lines ");
    std::fprintf(f, "%s\n\n", "# X-position      Y-position      Z-position     Time     Value");

    return f;
}

int Advection::OutputStreamsGnuplotMaxPart(const std::string &filename, size_t maxPart, bool append) const
{
    std::FILE *f = _prepareFileWrite(filename, append);
    if (f == nullptr) return FILE_ERROR;

    for (const auto &s : _streams) {
        // Either output all the particles in this stream,
        // or only up to a certain number of particles.
        size_t numPart = 0;
        for (size_t i = 0; i < s.size() && numPart < maxPart; i++) {
            const auto &p = s[i];
            if (!p.IsSpecial()) {
                std::fprintf(f, "%f, %f, %f, %f, %f\n", p.location.x, p.location.y, p.location.z, p.time, p.value);
                numPart++;
            }
        }
        std::fprintf(f, "\n\n");
    }
    std::fclose(f);

    return 0;
}

int Advection::OutputStreamsGnuplotMaxTime(const std::string &filename, float timeStamp, bool append) const
{
    std::FILE *f = _prepareFileWrite(filename, append);
    if (f == nullptr) return FILE_ERROR;

    for (const auto &s : _streams) {
        for (const auto &p : s) {
            if (p.time > timeStamp) break;    // finish this current stream

            if (!p.IsSpecial()) { std::fprintf(f, "%f, %f, %f, %f, %f\n", p.location.x, p.location.y, p.location.z, p.time, p.value); }
        }
        std::fprintf(f, "\n\n");
    }
    std::fclose(f);

    return 0;
}

int Advection::InputStreamsGnuplot(const std::string &filename)
{
    std::ifstream ifs(filename);
    if (!ifs.is_open()) return FILE_ERROR;

    std::vector<Particle> newSeeds;
    std::string           line;

    while (std::getline(ifs, line)) {
        // remove leading spaces and tabs
        size_t found = line.find_first_not_of(" \t");
        if (found != std::string::npos) line = line.substr(found, line.size() - found);
        // skip this line if it's empty
        if (line.empty()) continue;

        // If leading by a #, then skip it.
        if (line.front() == '#') continue;

        // Now try to parse numbers separated by comma
        line.push_back(',');    // append a comma to the very end of the line
        std::vector<float> values;
        size_t             start = 0;
        size_t             end = line.find(',');
        while (end != std::string::npos && values.size() < 4) {
            // Here we read in first 3 or 4 values of the line.
            // Additional values are discarded.
            auto  str = line.substr(start, end - start);
            float val;
            try {
                val = std::stof(str);
            } catch (const std::invalid_argument &e) {
                ifs.close();
                return FILE_ERROR;
            }
            values.push_back(val);
            start = end + 1;
            end = line.find(',', start);
        }

        // See if we have collected enough numbers
        if (values.size() < 3) {
            ifs.close();
            return FILE_ERROR;
        } else if (values.size() == 3)
            newSeeds.emplace_back(values.data(), 0.0f);
        else    // values.size() == 4
            newSeeds.emplace_back(values.data(), values[3]);
    }
    ifs.close();

    if (!newSeeds.empty()) this->UseSeedParticles(newSeeds);

    return 0;
}

size_t Advection::GetNumberOfStreams() const { return _streams.size(); }

const std::vector<Particle> &Advection::GetStreamAt(size_t i) const
{
    // Since this function is almost always used together with GetNumberOfStreams(),
    // I'm offloading the range check to std::vector.
    return _streams.at(i);
}

size_t Advection::GetMaxNumOfPart() const
{
    size_t max = 0;
    size_t idx = 0;
    for (const auto &s : _streams) {
        size_t num = s.size() - _separatorCount[idx];
        if (num > max) max = num;
        idx++;
    }
    return max;
}

void Advection::ClearParticleProperties()
{
    for (auto &stream : _streams)
        for (auto &part : stream) part.ClearProperties();
}

void Advection::ResetParticleValues()
{
    for (auto &stream : _streams)
        for (auto &part : stream) part.value = 0.0f;
}

void Advection::SetXPeriodicity(bool isPeri, float min, float max)
{
    _isPeriodic[0] = isPeri;
    if (isPeri)
        _periodicBounds[0] = glm::vec2(min, max);
    else
        _periodicBounds[0] = glm::vec2(0.0f);
}

void Advection::SetYPeriodicity(bool isPeri, float min, float max)
{
    _isPeriodic[1] = isPeri;
    if (isPeri)
        _periodicBounds[1] = glm::vec2(min, max);
    else
        _periodicBounds[1] = glm::vec2(0.0f);
}

void Advection::SetZPeriodicity(bool isPeri, float min, float max)
{
    _isPeriodic[2] = isPeri;
    if (isPeri)
        _periodicBounds[2] = glm::vec2(min, max);
    else
        _periodicBounds[2] = glm::vec2(0.0f);
}

float Advection::_applyPeriodic(float val, float min, float max) const
{
    if (min >= max)    // ill params, return val
        return val;
    else if (val >= min && val <= max)    // nothing needs to change
        return val;

    // Let's do some serious work
    float span = max - min;
    float pval = val;
    if (val < min) {
        while (pval < min) pval += span;
        return pval;
    } else    // val > max
    {
        while (pval > max) pval -= span;
        return pval;
    }
}
