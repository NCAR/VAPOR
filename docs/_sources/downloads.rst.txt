.. _downloads:

=========
Downloads
=========

If you are looking for our legacy version of Vapor 2.6, follow :ref:`this link <vapor2>` to the bottom of the page.

Current Release: Vapor 3.1.0
----------------------------

July 5, 2019

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Release notes for VAPOR-3.1.0

    New Features:

    - 3D Variable Support
    - Direct Volume Renderer
    - Isosurfaces
    - Slice Renderer
    - Wireframe Renderer
    - Python variable engine
    - Geotiff creation from Vapor renderings
    - Support for MPAS-A and MOM6 models

|

.. _installationInstructions:

Installation Instructions
-------------------------

We encourage users of Vapor to install with the methods described here.  If you're a developer and would like to contribute, see the :ref:`Building From Source <buildFromSource>` section below.

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

**Warning: Building Vapor from source is a complex process.  We highly encourage users to use our binary installers provided above.**

**Step 1 - Download the source code**

Vapor's current master branch source code can be downloaded from GitHub_.

.. _GitHub: https://github.com/NCAR/vapor

**Step 2 - Install Vapor's third party libraries**

*Windows*

Untar the three files linked below into the root of your C:\ directory.

    `vaporwin64deps2015.tar <https://drive.google.com/a/ucar.edu/file/d/1CHUxsPZYrZPDVqRCT-1qtTEQtZfgTn7u/view?usp=sharing>`_

    `Vapor3rdParty2015.tar <https://drive.google.com/a/ucar.edu/file/d/1ZDK2pDu66XDVhJBpFogdqVgNFIMKsrtM/view?usp=sharing>`_

    `Qt-2015.tar <https://drive.google.com/a/ucar.edu/file/d/19RGYew30dH6T6zG3HzfWwck4RZNLXAhp/view?usp=sharing>`_

*Linux and OSX*

If building on Linux or OSX, third party libraries are unable to be written to arbitrary directory locations.  If you choose do download the pre-built libraries, they must be unpacked in the following directories:

OSX: /glade/p/VAST/VAPOR/third-party/apps-2017/Darwin_x86_64/

    `Link to OSX third-party libraries <https://drive.google.com/open?id=1JHl6kHkBvbd17BUC-9nvWZupjyWfwyw7>`_

Linux: /glade/p/VAST/VAPOR/third-party/apps-2017/Linux_x86_64/

    `Link to Ubuntu third-party libraries <https://drive.google.com/open?id=0B0dQMtxB89M0azF5RW1RSE5qcTg>`_

    `Link to CentOS third-party libraries <https://drive.google.com/open?id=1_JdUuiy_iQUuIDoPyBn2pupBTz-LS4pM>`_

Alternatively, you can build the libraries yourself and store them wherever you want.  If you choose to do this, you must also configure CMake to point to your custom directory.

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
| grib_api        | 1.19.0          | Built with unreleased memory leak patches    |
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
 	 
**Step 3 - Set up your compiler**

The following compilers are required to build Vapor.

Ubuntu/CentOS - GCC 4.8.5 or higher
Windows - Microsoft Visual Studio 2015, version 14
	
**Step 4 - Configure CMake**

CMake version 3.2 or higher is required on all platforms.  If you chose to build the third party libraries manually, CMake must be configured to point to those libraries wither with the CMake GUI (Windows) or the ccmake command (OSX and Linux).  Run either the GUI or ccmake on the directory where your source code resides to configure the build process.

**Step 5 - Run CMake**

On all operating systems, create a directory where the build will take place.  

On Windows, enter this directory as the "Where to build the binaries" field in the GUI.  Click *Configure*, *Generate*, and then *Open Project* in that order.  Visual Studio will open, and you can build the target *PACKAGE* to compile the source code.

On OSX and Linux, navigate to your build directory and type *cmake <build_directory> && make*, where <build_directory> is where your build is taking place.

**Step 6 - Build an installer**

Edit the file *CMakeLists.txt* in the root of your source code directory, so that the field *CMAKE_BUILD_TYPE Debug* is changed to *CMAKE_BUILD_TYPE Release*.  Also change the field *DIST_INSTALLER OFF* to be *DIST_INSTALLER ON*.

On Windows, make sure that the build is taking place in *Release* mode, not *Debug*, and build the target *PACKAGE*.

On OSX, run *cmake <build_directory> && make && make installer* from your build directory.

On Linux, run  *cmake <build_directory> && make linuxpreinstall && make installer* from your build directory.

|

.. _sampleData:

Sample Data
-----------

**Coming soon**

|

.. _vapor2:

Vapor 2
-------

If you are interested in using Vapor 2, it can be `downloaded after filling out a short survey <https://forms.gle/ZLX7oZ7LYAVEEBH4A>`_.

Vapor 2 is deprecated, and we strongly encourage users to download the currently supported releases of Vapor 3.

`Legacy documentation for Vapor 2 can be found here <https://ncar.github.io/vapor2website/index.html>`_.  Please note that this website is no longer supported, and some links may be broken.  Use at your own discretion.
