##============================================================================
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##============================================================================
cmake_minimum_required(VERSION 3.8...3.15 FATAL_ERROR)
project(advection CXX)

list(APPEND CMAKE_MODULE_PATH "~/repositories/CMakeFindModules")

# Find VAPOR packages
# https://github.com/ayenpure/CMakeFindModules/blob/master/FindVAPOR.cmake
find_package(VAPOR REQUIRED QUIET)
# https://github.com/ayenpure/CMakeFindModules/blob/master/FindVAPORDEPS.cmake
find_package(VAPORDEPS REQUIRED QUIET)
# Find Boost
find_package(Boost COMPONENTS program_options filesystem REQUIRED)

add_executable(benchmark benchmark.cxx Advection.h Advection.cpp)
set_target_properties(benchmark PROPERTIES CXX_STANDARD 11)
target_include_directories(benchmark PRIVATE ${VAPORDEPS_INCLUDE_DIRS} ${VAPOR_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
target_link_libraries(benchmark PRIVATE ${VAPOR_LIBRARIES} ${Boost_LIBRARIES})
