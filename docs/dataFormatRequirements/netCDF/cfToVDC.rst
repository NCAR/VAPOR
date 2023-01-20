:orphan:

The two tools for converting CF-Compliant NetCDF into VDC are ``cfvdccreate`` and ``cf2vdc``.  Advanced options can be seen at the bottom of this page, or by issuing the commands
 without any arguments.

Converting NetCDF data into VDC
-------------------------------

Create .vdc metadata file
_________________________

In the directory where Vapor 3 is installed, there is a command line utility called ``cfvdccreate``.  Issue this command in a terminal (Unix) or command prompt (Windows), followed by your WRF-ARW files, and finally the name of the .vdc file to be written.

.. code-block:: c

   cfvdccreate 24Maycontrol.04005.000000.nc tornado.vdc

Advanced options for cfvdccreate:

.. literalinclude:: ../../commandLineTools/cfvdccreate.txt

Generate VDC Data
_________________

Once we have a .vdc file, the metadata has been recorded and we can transform the data into the VDC format.  From Vapor 3's installation directory, issue the command ``wrf2vdc``, followed by your WRF-ARW files, and finally followed by the .vdc file that was made in Step 1.

.. code-block:: c

   cf2vdc 24Maycontrol.04005.000000.nc tornado.vdc

.. literalinclude:: ../../commandLineTools/cf2vdc.txt
