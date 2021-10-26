.. _cfConventions:

.. figure:: /_images/cfCompliance.png
    :width: 400
    :align: center
    :figclass: align-center

NetCDF CF-Conventions
_____________________

The following examples show how to make NetCDF data comply with the CF-Conventions using Python.  If your data does not fit these Python examples, the documentation below gives an overview of what Vapor needs in order to read your NetCDF data.  If you still have trouble, contact us `on our forum <https://vapor.discourse.group/>`_.

.. toctree::
   :maxdepth: 1

   regularGridExample
   stretchedGridExample

Coordinate Variables
====================

What is a "Coordinate Variable"?  From the CF1.X Definition:

*We use this term precisely as it is defined in section 2.3.1 of the NUG.  It is a one- dimensional variable with the same name as its dimension [e.g., time(time)], and it is defined as a numeric data type with values that are ordered monotonically. Missing values are not allowed in coordinate variables.*

A coordinate variable is a variable that defines where your grid nodes exist in space.  It usually has the same name as the dimension it is describing.  See the Python examples for guidance on how to generate coordinate variables if your NetCDF files do not contain them.

The axis attribute
==================

Each coordinate variable must have an ``axis`` attribute as follows:

    - ``X`` coordinate variables must contain an ``axis`` attribute that is equal to ``0`` or ``X``.
    - ``Y`` coordinate variables must contain an ``axis`` attribute that is equal to ``1`` or ``Y``.
    - ``Z`` coordinate variables must contain an ``axis`` attribute that is equal to ``2`` or ``Z``.
    - ``Time`` coordinate variables must contain an ``axis`` attribute that is equal to ``3`` or ``T``.

The units attribute
===================

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
