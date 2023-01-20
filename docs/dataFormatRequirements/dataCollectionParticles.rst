.. _dcp:

Data Collection Particles (DCP)
```````````````````````````````

Data Collection Particles is a simple data format built upon NetCDF for importing particle data into VAPoR.

Dimensions
__________

P 
==

P is on only required dimension. The size of the dimension P represents the number of particles at this timestep. If the number of particles varies between timesteps, every timestep must be in its own file since each NetCDF file can only define a single length for a dimension.

T 
==

T is the time dimension and it is required for time-varying data. It should be set to *unlimited*.

axis
====

This optional dimension is must have a length of 3 and it is used to pack 3 component values such as position or velocity into a single variable for convenience.

Variables
_________

The only required variable is Position. Non-time-varying data should have a single dimension of `P`. If the data is time-varying, it should have the dimensions `(T, P)`. 3D particles will often have 3-component data associated with them, for example their velocity would have 3 components, one for each dimension. Since this is a common use case, variables can have 3 components per particle and this is done by setting the fastest-varying dimension to `axis` which as a size of 3. For an example, look at the position variable description below.

Position
========

This variable contains the position data for the particles. The data can be stored in either of the below methods:

.. code-block:: c

   float Position(T, P, axis) ;

Or:

.. code-block:: c

   float Position_x(T, P) ;
   float Position_y(T, P) ;
   float Position_z(T, P) ;


Examples
________

Python Script
=============

`vapor/share/examples/generateExampleDCP.py` shows how to generate a DCP dataset from a particle simulation. 


Basic NetCDF File
=================

This is an example of a NetCDF file containing 2 particles with time-varying position and particle speed data.

.. code-block:: c

   netcdf particles_000 {
   dimensions:
   	P = 2 ;
   	T = UNLIMITED ; // (1 currently)
   	axis = 3 ;
   variables:
   	double T(T) ;
   		T:units = "seconds" ;
   	float Position(T, P, axis) ;
   	float speed(T, P) ;
   data:
   
    T = 0 ;
   
    Position =
     9.762701, 43.03787, 20.55268,
     8.976637, -15.26904, 29.17882 ;
   
    speed =
     26.02757, 18.87516 ;
   }

.. include:: ./importData.rst

