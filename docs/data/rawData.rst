.. _rawData:

Raw Binary Data
```````````````

.. figure:: /_images/rawData.png
    :width: 400
    :align: center
    :figclass: align-center

    A file containing binary float, double, or integer data that represent values for a single variable, at a single timestep.

If your data files are just a series of floats, doubles, or integers, Vapor can read your data in two ways.

1) Directly import your data with the :ref:`Brick of Values (BOV)<brickOfValues>` reader
2) Convert your data into a :ref:`Vapor Data Collection (VDC)<rawToVDC>`, useful for very large datasets

.. toctree::
   :maxdepth: 1

   brickOfValues
   rawToVDC
