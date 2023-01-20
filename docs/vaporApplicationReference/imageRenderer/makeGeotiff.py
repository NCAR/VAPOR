#!/usr/bin/env python3

"""
makeGeotiff.py
=========================
This script creates a GeoTiff image that can be read by VAPOR, when given
a set of lat/lon coordinates.

It performs the the following:
   - Takes user-specified lat/lon coordinates to query NASA's WorldView WTMS server for satellite imagery
   - WTMS servers and layers can be changed by modifying the "url" and "layer" global variables
   - A NaturalEarth shapfile describing roads in North America are added to the produced GeoTiff
   - Coastlines are added to the map through Cartopy
"""

# sphinx_gallery_thumbnail_path = '_images/map.png'

targetDir = "/Users/pearse/"
fileName = "landSat_test2"
west = -105.5 
north = 40.25 
east = -104.75 
south = 39.6

###############################################################################
# Size of our output figure.
# Note: If your specified lat/lon extents have a different aspect ratio than 
# your width and height, the geotiff will have either its dimensions scaled to 
# match the aspect ratio of the specified extents of the west/north/east/south 
# variables.
width = 1920
height = 1080

###############################################################################
# For the generated tiff to have the correct width and height, the "dpi" 
# variable must be set according to that of your monitor.  
# To find your DPI, see here: https://www.infobyip.com/detectmonitordpi.php
dpi = 96

###############################################################################
# URL for NASA's EarthData/WorldView web map tile service
url = 'https://map1c.vis.earthdata.nasa.gov/wmts-geo/wmts.cgi'

###############################################################################
# Specify the layer from the EarthData WMTS to draw to our geotiff.
# See Vapor's Image Renderer documentation for a complete list of available 
# layers.
# Some options include:
#   MODIS_Terra_CorrectedReflectance_TrueColor
#   Landsat_WELD_CorrectedReflectance_Bands157_Global_Annual
#   VIIRS_CityLights_2012
#   GOES-West_ABI_Band2_Red_Visible_1km
# To preview these layers, visit https://worldview.earthdata.nasa.gov/
layer = 'Landsat_WELD_CorrectedReflectance_TrueColor_Global_Annual'

###############################################################################
# Generate our matplotlib figure with a subplot to draw our map upon
import matplotlib.pyplot as plt
import cartopy.crs as ccrs
fig = plt.figure(
    figsize=(width/dpi, height/dpi), 
    tight_layout=True 
)
ax = fig.add_subplot(1, 1, 1, projection=ccrs.PlateCarree())
ax.add_wmts(url, layer)
ax.set_extent(
    [west, east, south, north], 
    crs=ccrs.PlateCarree()
)

###############################################################################
# Add coastlines from Cartopy
ax.coastlines(resolution='50m', color='yellow')

###############################################################################
# Add roads from NaturalEarth
import cartopy.feature as cf
ax.add_feature(
    cf.NaturalEarthFeature('cultural', 'roads_north_america', '10m'), 
    edgecolor='yellow', 
    facecolor='none'
)

###############################################################################
# Generate our initial tiff file
tiffFile = targetDir + fileName + ".tif"
fig.savefig( tiffFile,
             bbox_inches='tight',
             pad_inches=0
)


###############################################################################
# Write our tiff file with GeoTiff extent information
from osgeo import gdal
gdal.OpenShared( tiffFile, gdal.GA_Update)
translatedTiff = targetDir + fileName + "Translated.tif"
gdal.Translate( srcDS=tiffFile, 
                destName=translatedTiff,
                format = 'GTiff', 
                outputBounds = [ west, north, east, south ],
                outputSRS = 'EPSG:4326'
)

###############################################################################
# Give our GeoTiff file a projected coordinate system, equivalent to the following proj4 string:
# Proj4: "+proj=eqc +lat_ts=0 +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84"
gdal.Warp(  destNameOrDestDS=tiffFile, 
            srcDSOrSrcDSTab=translatedTiff, 
            srcSRS = 'EPSG:4326',
            dstSRS='EPSG:32662'
)

###############################################################################
# Clean up intermediate translated file
import os
os.remove(translatedTiff)
