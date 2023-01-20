.. _cfRequirements:

NetCDF-CF Requirements
======================

.. _coordinateVariables:

Coordinate variables
********************

Vapor requires variables that describe the physical coordinates of netCDF data.  

If your data is on a rectilinear grid, these variables should be 1D arrays that specify monotonically increasing grid points along an axis.  These variables are referred to as ``coordinate variables`` if their names match the dimension they refer to.

.. figure:: /_images/rectilinearCompliant.png
    :align: center
    :figclass: align-center

    A CF Compliant netCDF header with 1D coordinate variables (x, y, and z), named the same as their dimension. 

If your grid is curvilinear, coordinate variables are not sufficient to describe the 2D physical coordinates of your grid.  In this case, the CF Conventions define ``auxiliary coordinate variables``, which do not have the same name as their dimensions.  ``auxiliary coordinate variables`` must have an ``axis`` and ``units`` attribute.  Additionally, any variable that is mapped to these coordinates must specify a ``coordinates`` attribute that points the their ``auxiallary coordinate variables``.  For example:

.. figure:: /_images/curvilinearCompliant.png
    :align: center
    :figclass: align-center

    A 8x8x8 curvilinear grid, with physical coordinates defined as 2D auxiliary coordinate variables.  Note that the sphere variable specifies its auxiliary coordinate variables "X_Coord Y_Coord "Z_Coord" with the coordinates attribute.
    

.. _theAxisAttribute:

The axis attribute
******************

We strongly recommend that each coordinate variable have ``axis`` attribute as follows:

    - ``X`` coordinate variables must contain an ``axis`` attribute that is equal to ``0`` or ``X``.
    - ``Y`` coordinate variables must contain an ``axis`` attribute that is equal to ``1`` or ``Y``.
    - ``Z`` coordinate variables must contain an ``axis`` attribute that is equal to ``2`` or ``Z``.
    - ``Time`` coordinate variables must contain an ``axis`` attribute that is equal to ``3`` or ``T``.

.. note::

    Coordinate variables do not require an ``axis`` attribute if its axis can be inferred by a ``units`` attribute.  For example, a coordinate variable with a ``units`` attribute of ``degreesEast`` would infer that it's aligned with the with X axis, and no ``axis`` attribute is needed.

.. _theUnitsAttribute:

The units attribute
*******************

The Time coordinate variable **must** have a ``units`` attribute which can be identifiable by the Udunits library.  Suitable ``units`` attributes include:

    - seconds
    - s
    - days since 0001-01-01 00:00:00
    - seconds since 2011-01-01 00:00:00

Coordinate variables for the X, Y and Z axes need to have an attribute that defines the units they are measured in.  These units will allow Vapor to create accurate flow renderings, as well as render multiple datasets.  Some suitable values for the ``units`` attribute are:

    - degree_east
    - meters
    - m
    - km

Variables with missing data values must have the attribute *_FillValue* or *missing_value* specified.  See section 2.5.1 of the CF 1.6 specification for more information.
