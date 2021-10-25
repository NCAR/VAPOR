#!/usr/bin/env python3

import xarray as xr
import numpy as np
import requests
from pathlib import Path

# Acquire sample data
home = str(Path.home())
simpleNC = home + "/simple.nc"
dataFile = "https://github.com/NCAR/VAPOR-Data/raw/main/netCDF/simple.nc"
response = requests.get(dataFile, stream=True)
with open(simpleNC, "wb") as file:
  for chunk in response.iter_content(chunk_size=1024):
    if chunk:
      file.write(chunk)

ds = xr.open_dataset(simpleNC)

ds.info()

ds['time'] = np.linspace(start=0, stop=0, num=1);
  
# Generate coordinates for x, y, z dimensions using numpy's linspace 
# https://numpy.org/doc/stable/reference/generated/numpy.linspace.html
  
ds['y'] = np.linspace(start=0, stop=100, num=48)
ds['x'] = np.linspace(start=0, stop=100, num=48)
ds['z'] = np.linspace(start=0, stop=50, num=24)

ds.info() 

ds.time.attrs['axis']      = 'T'
ds.x.attrs['axis']         = 'X'
ds.y.attrs['axis']         = 'Y'
ds.z.attrs['axis']         = 'Z'

ds.time.attrs['units']     = 'seconds since 2000-0101'
ds.x.attrs['units']        = 'm'
ds.y.attrs['units']        = 'm'
ds.z.attrs['units']        = 'm'

ds.temperature.attrs['units'] = 'K'

ds.info()

ds.to_netcdf( home + "/regularCompliant.nc")

ds.isel(time=0, z=0).temperature.plot(size=6, robust=True);
