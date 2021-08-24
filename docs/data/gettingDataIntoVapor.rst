.. _gettingDataIntoVapor:

Getting Data into Vapor
-----------------------

Vapor can directly import the datasets listed below.  Some of them can be converted into the :ref:`VDC <vdc>` format if you are experiencing a performance bottleneck.  We strongly recommend that users start by importing their data and only convert VDC if necessary.

    - NetCDF files that follow the CF Convention (:ref:`NetCDF-CF <netCDF-CF>`)
    - WRF-ARW :ref:`wrf`
    - MPAS :ref:`mpas`

:ref:`Raw binary <binary>` data must be converted to VDC before loading into Vapor.

.. include:: ./importData.rst

.. include:: vdc.rst

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

This process is supported with :ref:`WRF-ARW <wrf>`, :ref:`NetCDF-CF <netCDF-CF>`, and `raw binary data <binary>`.

Once the conversion is complete, users can load VDC files into Vapor.  Read on for instructions for your data type.

.. figure:: _images/loadData.png
    :width: 400
    :align: center
    :figclass: align-center

    Loading a VDC into Vapor.

.. include:: wrf.rst

.. include:: netCDF/netCDF.rst

.. include:: vdc.rst

.. include:: binary.rst
