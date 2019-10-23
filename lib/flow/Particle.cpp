#include "vapor/Particle.h"
#include <stdexcept>

using namespace flow;

Particle::Particle(const glm::vec3 &loc, float t, float val)
{
    location = loc;
    time = t;
    value = val;
}

Particle::Particle(const float *loc, float t, float val)
{
    location.x = loc[0];
    location.y = loc[1];
    location.z = loc[2];
    time = t;
    value = val;
}

Particle::Particle(float x, float y, float z, float t, float val)
{
    location.x = x;
    location.y = y;
    location.z = z;
    time = t;
    value = val;
}

void Particle::AttachProperty(float v)
{
    auto itr = _properties.cbefore_begin();
    for (const auto &x : _properties) {
        (void)x;
        ++itr;
    }
    _properties.insert_after(itr, v);
    _nProperties++;
}

float Particle::RetrieveProperty(int idx) const
{
    if (idx < 0 || idx > _nProperties) throw std::out_of_range("flow::Particle");

    auto itr = _properties.cbegin();
    for (int i = 0; i < idx; i++) { ++itr; }
    return *itr;
}

void Particle::ClearProperties()
{
    _properties.clear();
    _nProperties = 0;
}

int Particle::GetNumOfProperties() const { return (_nProperties); }

void Particle::SetSpecial(bool isSpecial)
{
    // Give both "time" and "value" a nan to indicate the "special state."
    // Accidental assignment of nan to one of the two variables would not
    // render a "special state."
    if (isSpecial) {
        time = std::nanf("1");
        value = std::nanf("1");
    } else {
        time = 0.0f;
        value = 0.0f;
    }
}

bool Particle::IsSpecial() const { return (std::isnan(time) && std::isnan(value)); }
