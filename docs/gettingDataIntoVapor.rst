.. _gettingDataIntoVapor:

Getting Data into Vapor
-----------------------

.. toctree::
   :maxdepth: 1

   regularGridExample
   stretchedGridExample

Vapor can read the datasets listed below.  Some of them can be converted into the :ref:`VDC <vdc>` format if you are experiencing a performance bottleneck.  We strongly recommend that users start by importing their data and only convert VDC if their data is large enough that it slows down the application enough that it's hard to interact with.

Click on the following links for more information on each supported data format.

    - :ref:`NetCDF files that follow the CF Conventions <netCDF>`
    - :ref:`WRF-ARW <wrf>`
    - :ref:`MPAS<mpas>`
    - :ref:`Brick of Values (BOV)<bov>`
    - :ref:`Data Collection Particles (DCP)<dcb>`
    - :ref:`Vapor Data Collection (VDC)<vdc>`

.. _vdc:

Vapor Data Collection (VDC)
```````````````````````````

In most cases, directly importing data is sufficient for an interactive user experience.  However, if rendering times keep the application from being interactive, users may want to consider using the VDC data foramt.

The VDC data format allows users to render their data at different levels of compression.  Viewing compressed data reduces the time a rendering takes to complete, improving interactivity.  

With VDC, users can configure their renderers quickly at low fidelity, and then turn off compression for their final renderings.  Being able to interact with your data becomes important when rendering takes many seconds, minutes, or even hours to complete.

.. toctree::
    referenceDocumentation.rst

Using the Command Line Tools
''''''''''''''''''''''''''''

Creating a VDC requires command line tools that come bundled with Vapor.  These tools can be found and issued in the installation directory.  For example, Windows users may find it in C:\Program Files\VAPOR\ and Linux users may find it in /home/john_doe/vapor/bin/wrf2vdc.

Alternatively, the tools can be added to the user's system path by clicking on the `Tools` menu, and selecting `Install Command Line Tools`.After doing this, users will be able to issue the command line utilities from any directory in their terminal or command prompt.

.. _commandLineTools:

.. figure:: _images/installCLTools.png
    :align: center
    :figclass: align-center

    Installing the command line tools for VDC creation, through Vapor's GUI.

The two step VDC creation process is as follows:

    Step 1) Create a .vdc metadata file that describes the structure of your data

    Step 2) Transform the data values into VDC format

This process is supported with :ref:`WRF-ARW <wrf>`, :ref:`NetCDF-CF <netCDF>`, and `raw binary data <binary>`.

Once the conversion is complete, users can load VDC files into Vapor.  Read on for instructions for your data type.

.. figure:: _images/loadData.png
    :width: 400
    :align: center
    :figclass: align-center

    Loading a VDC into Vapor.

Raw Binary Data
```````````````

Converting raw binary data to VDC is a complex process for converting data in Vapor 3.  With WRF-ARW and NetCDF-CF data, Vapor can read the files an extract metadata that describes the grid that the data exists within.  With raw binary data, we need to define that metadata ourselves in step 1.

Step 1: Create .vdc metadata file
_________________________________

To make a VDC from scratch, users need to carefully read all options in the ``vdccreate`` utility, and define their .vdc metadata file accordingly.

In this `sample dataset of a sphere <https://drive.google.com/open?id=1wJtPX0DPgovZSulAC8kntDKVcDkTw1Y7>`_, we have a 64x64x64 rectilinear grid with one timestep, and one 3D variable.  For this example, we can create a .vdc metadata file with the following flags from vdccreate.  Note that we are not using the raw data file yet, just defining the grid, time dimension, and variables.

    ``vdccreate -dimension 64x64x64 -numts 1 -vars3d exampleVar sphere64.vdc``

.. figure:: _images/vdccreate.png
    :align: center
    :figclass: align-center

    Command line arguments for vdccreate, seen by issuing the command without any arguments

Step 2: The wavelet transform
_____________________________

Now that a .vdc metadata file has been created, the VDC transform can take place.  Each variable must be converted individually with ``raw2vdc``, and this must be done one timestep at a time.

    ``raw2vdc -ts 0 -varname exampleVar sphere64.vdc sphere64.raw``

.. figure:: _images/sphere.png
    :align: center
    :figclass: align-center

    A volume rendering of our sphere, converted from raw binary data.

.. figure:: _images/raw2vdc.png
    :align: center
    :figclass: align-center

    Command line arguments for raw2vdc wavelet transform, seen by issuing the command without any arguments.
