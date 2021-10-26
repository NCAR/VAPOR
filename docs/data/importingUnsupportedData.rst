.. _importingUnsupportedData:

|

Importing unsupported data
__________________________

.. image:: /_images/importingUnsupportedData.png
    :width: 400
    :align: center

Several data types can be adjusted for importing into Vapor.  The table below shows these formats, along with the supported and unsupported features of each data type.

    +--------------------+--------------------------------------------+----------------------------+------------------+
    |                    |  :ref:`CF Compliant netCDF <cfCompliance>` | :ref:`BOV <brickOfValues>` | :ref:`VDC <vdc>` |  
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Regular grids      |  Yes                                       | Yes                        | Yes              |  
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Stretched grids    |  Yes                                       | No                         | Yes              | 
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Curvilinear grids  |  Yes                                       | No                         | Yes              |
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Missing values     |  Yes                                       | No                         | Yes              |  
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Raw, Binary data   |  No                                        | Yes                        | Yes              |  
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Multi-resolution*  |  No                                        | No                         | Yes              |  
    +--------------------+--------------------------------------------+----------------------------+------------------+
    | Complexity         |  Medium                                    | Low                        | High             |  
    +--------------------+--------------------------------------------+----------------------------+------------------+

For example, if your data is organized as a collection of RAW files that are sampled on a `regular grid <https://en.wikipedia.org/wiki/Regular_grid>`_ you may want to use Vapor's BOV format. 

If your data are RAW files on a rectilinear or curvilinar grid, VAPOR can read your data after converting it to VDC with the :ref:`vdccreate <vdccreate>` and raw2vdc command line tools that are bundled with your installation.

For rectilinear or curvilinear grids you'll need to use the more flexible NetCDF-CF.  If your data are already in the NetCDF format but are not CF compliant, then NetCDF-CF is probably your best choice.

.. toctree::
   :maxdepth: 1

   rawData
   Non CF-Compliant NetCDF <netCDF/cfComplianceTOC>

.. note::

    *The multi-resolution feature of the VDC Allows you to render large datasets with or without lossy compression.  Viewing compressed data can dramatically increase the interactivity for your visualizations if your computer's resources are constrained by the size of your data.  

    When you're done with exploring compressed data and ready for a final rendering, you can render an image or animation sequence with lossless compression.
