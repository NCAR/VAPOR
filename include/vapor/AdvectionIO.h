/*
 * Define input/output operations given an Advection.
 * Specifically, it can read a list of seeds for the advection class to start with,
 * and also output the trajectory of advectios to a text file.
 */

#ifndef ADVECTION_IO_H
#define ADVECTION_IO_H

#include <iostream>
#include "vapor/Advection.h"

namespace flow {
// Output a certain number of steps from an advection.
// When `append == false`, a header will also be output.
// Otherwise, only trajectories are output.
FLOW_API auto OutputFlowlinesNumSteps(const Advection *adv, const char *filename, size_t numStep, const std::string &proj4string, bool append) -> int;

// Output trajectory to a maximum time.
// When `append == false`, a header will also be output.
// Otherwise, only trajectories are output.
FLOW_API auto OutputFlowlinesMaxTime(const Advection *adv, const char *filename, double maxTime, const std::string &proj4string, bool append) -> int;

// Input a list of seeds from lines of CSVs.
// In case of any error occurs, it returns an empty list.
FLOW_API auto InputSeedsCSV(const std::string &filename) -> std::vector<flow::Particle>;

};    // namespace flow
#endif
