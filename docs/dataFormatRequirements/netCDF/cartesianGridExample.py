#!/usr/bin/env python3

"""
cartesianGridExample.py
=====================
This script applies the CF Conventions to a basic NetCDF file so Vapor can read it.
It performs the the following:
   - Creates "Coordinate Variables" for the spatial and time dimensions
   - Adds the "axis" attributes to these new Coordinate Variables
   - Adds the "units" attribute to these new Coordinate Variables

The sample data for this script can be downloaded here:
    https://github.com/NCAR/VAPOR-Data/blob/main/netCDF/simple.nc
"""

###############################################################################
# Import packages:

import xarray as xr
import numpy as np
from pathlib import Path
# sphinx_gallery_thumbnail_path = '_images/cartesian.png'

###############################################################################
# Open sample data as an xarray dataset.
# Modify the 'home' variable to point to the directory containing sample data
home = str(Path.home())
simpleNC = home + "/simple.nc"
ds = xr.open_dataset(simpleNC)

ds.info()

###############################################################################
# Create a Coordinate Variable for our 'time' dimension and assign a value to it
ds['time'] = np.linspace(start=0, stop=0, num=1);

###############################################################################
# Generate Coordinate Variables for our x, y, z dimensions.  The coordinate
# values are monotonically increasing and equally spaced.  They are generated
# with numpy's linspace function.
# https://numpy.org/doc/stable/reference/generated/numpy.linspace.html
  
ds['y'] = np.linspace(start=0, stop=100, num=48)
ds['x'] = np.linspace(start=0, stop=100, num=48)
ds['z'] = np.linspace(start=0, stop=50, num=24)

ds.info() 

###############################################################################
# Give our Coordinate Variables 'axis' attributes so Vapor knows which axes they
# apply to

ds.time.attrs['axis']      = 'T'
ds.x.attrs['axis']         = 'X'
ds.y.attrs['axis']         = 'Y'
ds.z.attrs['axis']         = 'Z'

###############################################################################
# Give our Coordinate Variables 'units' attributes

ds.time.attrs['units']     = 'seconds since 2000-0101'
ds.x.attrs['units']        = 'm'
ds.y.attrs['units']        = 'm'
ds.z.attrs['units']        = 'm'


###############################################################################
# It's optional but advisable to give our scalar variables a 'units' attribute
ds.temperature.attrs['units'] = 'K'

ds.info()

###############################################################################
# Save our file for reading into Vapor
ds.to_netcdf( home + "/regularCompliant.nc")

###############################################################################
# Plot a 2D cross section of temperature
ds.isel(time=0, z=0).temperature.plot(size=6, robust=True);
