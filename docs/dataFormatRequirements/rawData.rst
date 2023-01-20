.. _rawData:

Raw Binary Data
```````````````

.. figure:: /_images/rawData.png
    :width: 400
    :align: center
    :figclass: align-center

    A file containing binary float, double, or integer data that represent values for a single variable, at a single timestep.

Data files that consist of sequential floating point values or integers can be read in two different ways:

1) Describe the structure of your raw data in .bov files with the :ref:`Brick of Values (BOV)<brickOfValues>` reader
2) Convert your data into a :ref:`Vapor Data Collection (VDC)<rawToVDC>`, useful for very large datasets

.. toctree::
   :maxdepth: 1

   brickOfValues
   rawToVDC
