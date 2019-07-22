.. _downloads:

=========
Downloads
=========

Vapor 3.1.0.rc0 
```````````````

+-----------------+-----------------+-----------------+-----------------+-----------------+
| Downloads                                                                               |
+-----------------+-----------------+-----------------+-----------------+-----------------+
| CentOS7_        | Ubuntu16_       | Ubuntu14_       | OSX_            | Windows10_      |
+-----------------+-----------------+-----------------+-----------------+-----------------+

.. _CentOS7: https://github.com/NCAR/VAPOR/releases/download/VAPOR3_1_0_RC0/VAPOR3-3.1.0.RC0-CentOS7.sh
.. _OSX: https://github.com/NCAR/VAPOR/releases/download/VAPOR3_1_0_RC0/VAPOR3-3.1.0.RC0-Darwin.dmg
.. _Ubuntu14: https://github.com/NCAR/VAPOR/releases/download/VAPOR3_1_0_RC0/VAPOR3-3.1.0.RC0-Ubuntu14.04.sh
.. _Ubuntu16: https://github.com/NCAR/VAPOR/releases/download/VAPOR3_1_0_RC0/VAPOR3-3.1.0.RC0-Ubuntu16.04.sh
.. _Windows10: https://github.com/NCAR/VAPOR/releases/download/VAPOR3_1_0_RC0/VAPOR3-3.1.0.RC0-win64.exe

Release notes for VAPOR-3.1.0.RC0

    New Features:

    - 3D Variable Support
    - Direct Volume Renderer
    - Isosurfaces
    - Slice Renderer
    - Wireframe Renderer
    - Python variable engine
    - Geotiff creation from Vapor renderings
    - Support for MPAS-A and MOM6 models

Note: This Release Candidate contains known issues with Windows drivers for Intel based graphics cards, Iris in particular.  We are working on a resolution.  You can keep track of our progress with our issue tracker on GitHub.

|

.. _installationInstructions:

Installation Instructions
-------------------------

**Linux**

Run the downloaded .sh script in a terminal window.  It will prompt you as to where the binaries will be installed. For example:
 
::

    % sh VAPOR3-3.0.0.beta-RH7.sh


**OSX**

Double click on the downloaded .dmg file.  Once the Finder window pops up, drag the Vapor icon into the Applications folder.

**Windows**

Run the downloaded .exe file.  A wizard will step you through the installer settings necessary for setup.

|

.. _buildFromSource:

Building From Source
--------------------

Link to GitHub

List of third-party libraries and flags used to build

Link to windows precompiled libraries

+-----------------+-----------------+----------------------------------------------+
| *Vapor 3 was build with the following third party library configuration.*        |
+-----------------+-----------------+----------------------------------------------+
| Library         | Version         | Configuration                                |
+-----------------+-----------------+----------------------------------------------+
| assimp          | 3.3.1           |                                              |
+-----------------+-----------------+----------------------------------------------+
| freetype        | 2.7.1           |                                              |
+-----------------+-----------------+----------------------------------------------+
| ftgl            | 2.1.3-rc5       |                                              |
+-----------------+-----------------+----------------------------------------------+
| glew            | 2.0.0           |                                              |
+-----------------+-----------------+----------------------------------------------+
| grib_api        | 1.19.0          | Built with unrealsed memory leak patches     |
+-----------------+-----------------+----------------------------------------------+
| hdf5            | 1.10.0          |                                              |
+-----------------+-----------------+----------------------------------------------+
| jpeg            | 9b              |                                              |
+-----------------+-----------------+----------------------------------------------+
| libgeotiff      | 1.4.2           | --with-libtiff                               |
+-----------------+-----------------+----------------------------------------------+
| udunits         | 2.2.20          |                                              |
+-----------------+-----------------+----------------------------------------------+
| netCDF          | 4.4.1.1         | --enable-shared --disable-dap                |
+-----------------+-----------------+----------------------------------------------+
| tiff	          | 4.0.7           | --enable-lzw --enable-jpeg --enable-old-jpeg |
+-----------------+-----------------+----------------------------------------------+
| proj            | 4.9.2           |                                              |
+-----------------+-----------------+----------------------------------------------+
| python          | 2.7.13          | --enable-shared                              |
+-----------------+-----------------+----------------------------------------------+
| Qt              | 4.8.7           | -no-webkit -no-multimedia -no-script         |
|                 |                 | -no-qt3support -opensource (-cocoa on Mac)   |
+-----------------+-----------------+----------------------------------------------+
 	 	 
|

.. _sampleData:

Sample Data
-----------

Link to sample data 1

Link to sample data 2

Link to sample data 3

Link to sample data 4

|
