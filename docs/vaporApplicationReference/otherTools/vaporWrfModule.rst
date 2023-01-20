The vapor_wrf module
____________________

The vapor_wrf module comes with functions that are useful for WRF-ARW output.  For example, the following can be used to calculate relative humidity once the appropriate variables have been selected and a new **relativeHumidity** variable has been defined in the gui.  See below for all available functions.

.. code-block:: python

    from vapor_wrf import *
    relativeHumidity = RH(P, PB, T, QVAPOR)

.. automodule:: vapor_wrf
   :members:
