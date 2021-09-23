.. _cfCompliance:

CF Compliance
`````````````

Vapor can read NetCDF data that is CF-Compliant.

This means that a given NetCDF file aheres to the `CF Conventions <https://cfconventions.org/>`_.  These conventions exist to provide a common description for what data each variable represents in a NetCDF file.

.. note::

    Below, you can find examples to make your data CF-Compliant, as well as documentation on what these examples are doing.  If you have trouble, contact us `on our forum <https://vapor.discourse.group/>`_.

    Examples in Python:

    * `Dealing with regular grids <regularGridExample>`_

    * `Dealing with stretched grids <stretchedGridExample>`_

    You can read more about Vapor's CF requirements `here. <convertToCF>`_

.. _whatDoesVaporNeed:

What does Vapor need in a CF-Compliant file?
============================================

.. _coordinateVariables:

Coordinate Variables
********************

What is a "Coordinate Variable"?  From the CF1.X Definition:

*We use this term precisely as it is defined in section 2.3.1 of the NUG.  It is a one- dimensional va
riable with the same name as its dimension [e.g., time(time)], and it is defined as a numeric data type with values that are ordered monotonically. Missing values are not allowed in coordinate variables.*

A coordinate variable is a variable that defines where your grid nodes exist in space.  It usually has the same name as the dimension it is describing.  See the Python examples for guidance on how to generate coordinate variables if your NetCDF files do not contain them.

.. _theAxisAttribute:

The axis attribute
******************


Each coordinate variable must have an ``axis`` attribute as follows:

    - ``X`` coordinate variables must contain an ``axis`` attribute that is equal to ``0`` or ``X``.
    - ``Y`` coordinate variables must contain an ``axis`` attribute that is equal to ``1`` or ``Y``.
    - ``Z`` coordinate variables must contain an ``axis`` attribute that is equal to ``2`` or ``Z``.
    - ``Time`` coordinate variables must contain an ``axis`` attribute that is equal to ``3`` or ``T``.

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
