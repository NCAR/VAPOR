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
A sample file `demo10K.ini` is included in this repository.

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

The important parameters in this file that need to be replaced are :

```
datapath : path to the directory containing your NetCDF data
fielx, fieldy, fieldz : the x, y, z components of your velocity field
steps : the maximum number of steps for advection
length : the length for each step
seeds : number of seeds for the advection
xrake, yrake, zrake : The rake region in the x, y, z dimension
```

The number of OpenMP threads to be used by the program are provided by 
exporting the environment variable `OMP_NUM_THREADS`

Once all the parameters have been updated the program can be run as :
```
export OMP_NUM_THREADS=16
./benchmark params.ini
```

Following is one example of a slurm batch script that can be used to run the program
on a cluster such as Casper.

```
#!/bin/bash -l

#SBATCH --job-name=advection
#SBATCH --account=DEMOACC
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --time=08:00:00
#SBATCH --partition=dav
#SBATCH --output=adv.out.%j

### Run program
export OMP_NUM_THREADS=16
./benchmark demo10k.ini
```

One important variable that needs to be updated here is `--cpus-per-task`
which will enable slurm to reserve as many CPUs for your program execution.

Here are the results from some benchmarks run on Casper.
The parameters of the benchmark are all the same as that in the file `demo10k.ini` in the repository
while varying the `seeds` parameter.

| Number of Particles | VAPOR | R1 | R2 | R4 | R8 | R16 | R32 | R64 |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 10000   | 517.96 | 550.94 | 562.55 | 372.32 | 233.46 | 157.19 | 217.8 | 219.17 | 
| 100000  |5282.56 | 5500.41 | 5636.72 | 3634.61 | 2313.86 | 1572.83 | 2204.82 | 2246.82 | 
| 1000000 | DNF | DNF | DNF | DNF | 22852.51 | 15770.41 | 22567.71 | 22664.41 |

All execution times in the table are in seconds.
The `VAPOR` column has the respective execution times before the refactoring for parallelization.
The `R[X]` columns have the respective execution times after the refactoring using X OpenMP threads.

