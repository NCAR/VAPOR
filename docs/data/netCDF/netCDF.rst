.. _netCDF:

NetCDF (CF Compliant)
---------------------

.. include:: data/cfConventions.rst

If your NetCDF data follows the CF conventions, then the process of converting it to VDC is nearly identical to the WRF conversion process.  The commands that will be used are named *cfvdccreate* for .vdc metadata creation, and *cf2vdc* for applying the transform.

The `CF Conformance Checker <http://cfconventions.org/compliance-checker.html>`_ may be used to test if your data adheres to the standard.  Passing this checker does not guarantee that Vapor will read the data, but a failer is a strong indicator that Vapor will not be able to read the data.

In order for NetCDF data to be compliant with the CF conventions, the following conditions must be met for the file's vector, scalar, and coordinate variables.

Coordinate Variables
____________________

What is a "Coordinate Variable"?  From the CF1.X Definition:

*We use this term precisely as it is defined in section 2.3.1 of the NUG.  It is a one- dimensional variable with the same name as its dimension [e.g., time(time)], and it is defined as a numeric data type with values that are ordered monotonically. Missing values are not allowed in coordinate variables.*

The axis attribute
______________________

Each coordinate variable must have an ``axis`` attribute as follows:

    - ``X`` coordinate variables must contain an ``axis`` attribute that is equal to ``0`` or ``X``.
    - ``Y`` coordinate variables must contain an ``axis`` attribute that is equal to ``1`` or ``Y``.
    - ``Z`` coordinate variables must contain an ``axis`` attribute that is equal to ``2`` or ``Z``.
    - ``Time`` coordinate variables must contain an ``axis`` attribute that is equal to ``3`` or ``T``.

The units attribute
_______________________

The Time coordinate variable **must** have a ``units`` attribute which must be identifiable by the Udunits library.  Suitable ``units`` attributes include:

    - seconds
    - s
    - days since 0001-01-01 00:00:00
    - seconds since 2011-01-01 00:00:00

Coordinate variables for the X, Y and Z axes may have an attribute that defines the units they are measured in.  These units will assiste vapor in creating renderings that are accurate between multiple datasets.  Some suitable values for the ``units`` attribute are:

    - degree_east
    - meters
    - m
    - km

Variables with missing data values must have the attribute *_FillValue* or *missing_value* specified.  See section 2.5.1 of the CF 1.6 specification for more information.
