# VAPOR Particle Advection Benchmarking

This directory has the benchmarking program for VAPOR's particle advection module

## Build project

The program builds with VAPOR with the `BUILD_TEST_APPS` flag turned on for CMake.

## Running benchmarks

The program requires the following list of parameters for it to run successfully :

| parameter | description |
| :-- | :-- |
| datapath  | path to the directory containing your NetCDF data
| fieldname | the x, y, z components of your velocity field
| seeds     | number of seeds for the advection
| steps     | the maximum number of steps for advection
| length    | the prescribed length for each advection step
| minu      | the minimum of the rake extents in the x, y, z directions
| maxu      | the maximum of the rake extents in the x, y, z directions
```

The number of OpenMP threads to be used by the program are provided by 
exporting the environment variable `OMP_NUM_THREADS`

Once all the parameters have been updated the program can be run as :
```
export OMP_NUM_THREADS=16
./benchmark -datapath path_to_your_data \
            -fieldname u:v:w \
            -seeds 10000 \
            -steps 5000 \
            -length 0.0005 \
            -minu 0:0:0 \
            -maxu 1:1:1
```

Following is one example of a slurm batch script that can be used to run the program
on a cluster such as Casper.

```
#!/bin/bash -l

#SBATCH --job-name=advection
#SBATCH --account=YOUR_ACC
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --time=08:00:00
#SBATCH --partition=dav
#SBATCH --output=adv.%j

### Run program
export OMP_NUM_THREADS=16
./benchmark -datapath path_to_your_data \
            -fieldname u:v:w \
            -seeds 10000 \
            -steps 5000 \
            -length 0.0005 \
            -minu 0:0:0 \
            -maxu 1:1:1
```

One important variable that needs to be updated here is `--cpus-per-task`
which will enable slurm to reserve as many CPUs for your program execution.

Here are the results from some benchmarks run on the Casper cluster at NCAR/UCAR.
The parameters of the benchmark listed in the following table

| parameter | value |
| :-- | :-- |
| datapath  | 24Maycontrol.04000.000000.nc (a test dataset for VAPOR) |
| fieldname | uniterp:vinterp:winterp |
| length    | 0.000427931 |
| minu      | -8.085:-8.985:0.015 | 
| maxu      | 3.015:4.215:0.9 |

The `seeds` argument was varyed for three different values, `10000`, `100000`, and `1000000`.
 
| Number of Particles | VAPOR | R1 | R2 | R4 | R8 | R16 | R32 | R64 |
|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 10000   | 517.96 | 550.94 | 562.55 | 372.32 | 233.46 | 157.19 | 217.8 | 219.17 | 
| 100000  |5282.56 | 5500.41 | 5636.72 | 3634.61 | 2313.86 | 1572.83 | 2204.82 | 2246.82 | 
| 1000000 | DNF | DNF | DNF | DNF | 22852.51 | 15770.41 | 22567.71 | 22664.41 |

All execution times in the table are in seconds, and are a mean of 2 different runs.
The `VAPOR` column has the respective execution times before the refactoring for parallelization.
The `R[X]` columns have the respective execution times after the refactoring using X OpenMP threads.

