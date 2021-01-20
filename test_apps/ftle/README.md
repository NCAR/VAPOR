# Calculate FTLE field using VAPOR 

To know more about FTLE (Finite Time Lyapunov Exponents) consider watching
the keynote from the Rocky Mountain Fluid Mechanics Symposium 2020
by Dr. Melissa Green [here](https://www.youtube.com/watch?v=Z2F79Su-Cvc&feature=youtu.be)

This directory has an example program for calculation of FTLE field using VAPOR's
flow module.

## Build project

The program builds with VAPOR with the `BUILD_TEST_APPS` flag turned on for CMake.

## Running benchmarks

The program requires the following list of parameters for it to run successfully :

| parameter | description |
| :-- | :-- |
| datapath   | path to the directory containing your NetCDF data
| fieldname  | the x, y, z components of your velocity field
| length     | the prescribed length for an advection step
| duration   | the total duration over which FTLE is to be calculated
| dimensions | The dimensions for the gird over which FTLE is to be calculated
| output     | name for the output file containing the scalar FTLE field

Once all the parameters have been updated the program can be run as :
```
./fltecalc -datapath path_to_your_data \
           -fieldname u:v:w \
           -length 0.005 \
           -duration 100 \
           -dimensions 50:100:50 \
           -output FTLEField.dat
```
The above command will produce an output binary file named `FTLEField.dat`
which can be visualized using a program like VAPOR, VisIt, or ParaView. 

Following is one example of a slurm batch script that can be used to run the program
on a cluster using Slurm.
```
#!/bin/bash -l

#SBATCH --job-name=ftlecalc
#SBATCH --account=YOUR_ACC
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=08:00:00
#SBATCH --partition=dav
#SBATCH --output=adv.out.%j

### Run program
./fltecalc -datapath path_to_your_data \
           -fieldname u:v:w \
           -length 0.005 \
           -duration 100 \
           -dimensions 50:100:50 \
           -output FTLEField.dat
```
The program produces a single output file for the provided duration,
to produce animations of the FTLE field significant changes are needed. 
