# VAPOR Particle Advection Benchmarking

This directory has the benchmarking program for VAPOR's particle advection module

## Build project

The project has 1 external dependency apart from VAPOR
-- The C++ Boost library [Program Options, File System]

In order to find all the required VAPOR components the following variables must
be provided to CMake

```
VAPOR_PREFIX : path to the VAPOR build/installation
VAPOR_INCLUDE_DIR : path to the include directory in the VAPOR repository
VAPORDEPS_PREFIX : path to where the VAPOR dependencies are located
```
A sample CMake command to build this project would look like

```
cmake -DVAPOR_PREFIX=/Users/abhishek/repositories/VAPOR/build \
      -DVAPOR_INCLUDE_DIR=/Users/abhishek/repositories/VAPOR/include \
      -DVAPORDEPS_PREFIX=/usr/local/VAPOR-Deps/2019-Aug \
      -DCMAKE_BUILD_TYPE=Release \
      .
```

## Running benchmarks

The program requires an input config file for all the required parameters.
A smaple file `demo10K.ini` is included in this repository.

```
datapath=/glade/u/home/abhisheky/NetCDF
fieldx=uinterp
fieldy=vinterp
fieldz=winterp
# Seeding 0 - > Uniform | 1 -> Random | 2 -> Coords
steps=1000
length=0.000427931
seeds=10000
xrake=-8.085 3.015
yrake=-8.985 4.215
zrake=0.015 0.9
```


The important parameters in this file that need to be replaced are
```
datapath : path to the directory containing your NetCDF data
fielx, fieldy, fieldz : the x, y, z components of your velocity field
steps : the maximum number of steps for advection
length : the length for each step
seeds : number of seeds for the advection
xrake, yrake, zrake : The rake region in the x, y, z dimension
```

Once all the parameters have been updated the program can be run as
`./benchmark params.ini`
