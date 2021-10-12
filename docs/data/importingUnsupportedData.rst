.. _importingUnsupportedData:

|

Importing unsupported data
__________________________

.. image:: /_images/importingUnsupportedData.png
    :width: 400
    :align: center

If your data is organized as a collection of RAW files that are sampled on a `regular grid <https://en.wikipedia.org/wiki/Regular_grid>`_
you may want to use Vapor's BOV format. 

If your data are RAW files on a rectilinear or curvilinar grid, VAPOR can read your data after converting it to VDC with the :ref:`vdccreate <vdccreate>` and raw2vdc command line tools that are bundled with your installation.

For rectilinear or curvilinear grids you'll need to use the more flexible NetCDF-CF.  If your data are already in the NetCDF format but are not CF compliant, then NetCDF-CF is probably your best choice.

.. figure:: /_images/grids.png
    :align: center

    From left to right: Regular, Rectilinear, and Curvilinear grids examples.

..
    This table can help you choose the best data format for your usage of Vapor.

    +--------------------------------------+--------------------------------+--------------------------------+--------------------------------+
    |                                      | BOV Reader                     | NetCDF-CF                      | VDC                            |
    +--------------------------------------+--------------------------------+--------------------------------+--------------------------------+
    | .. image :: /_images/regular.png     | .. image :: /_images/check.png | .. image :: /_images/check.png | .. image :: /_images/check.png |
    | Regular Grid                         |                                |                                |                                |
    +--------------------------------------+--------------------------------+--------------------------------+--------------------------------+
    | .. image :: /_images/rectilinear.png | .. image :: /_images/x.png     | .. image :: /_images/check.png | .. image :: /_images/check.png | 
    +--------------------------------------+--------------------------------+--------------------------------+--------------------------------+
    | .. image :: /_images/curvilinear.png | .. image :: /_images/x.png     | .. image :: /_images/check.png | .. image :: /_images/check.png |
    +--------------------------------------+--------------------------------+--------------------------------+--------------------------------+

    .. csv-table:: FooBar
       :file: /foo.csv
 
.. toctree::
   :maxdepth: 1

   rawData
   Non CF-Compliant NetCDF <netCDF/cfComplianceFull>
