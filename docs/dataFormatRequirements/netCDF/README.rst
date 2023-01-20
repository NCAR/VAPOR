:orphan:

.. _ncdfExamples:

.. figure:: /_images/netcdfExamplesInPython.png
    :width: 400
    :align: center
    :figclass: align-center

Examples
========

The examples below will show you how to make NetCDF data CF-Compliant, so that it can be read by Vapor.  

Make sure to pick the example that matches the grid type of your data.  For information on Vapor's supported grid types (cartesian, rectilinear, and curvilinear), you can `read more here <https://en.wikipedia.org/wiki/Regular_grid>`_. 

A demonstration can be viewed below, and the sample data used in these examples can be downloaded `here <https://github.com/NCAR/VAPOR-Data/blob/main/netCDF/simple.nc>`.

.. note::

    The Cartesian and Rectilinar grid examples are nearly identical.  The only difference is how we define the coordinate variables, using numpy.linspace in the cartesian grid example, and numpy.geomspace in the rectilinear grid example.

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/lYzj3ZQMlBE" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

