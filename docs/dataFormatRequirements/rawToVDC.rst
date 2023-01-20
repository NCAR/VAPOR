.. _rawToVDC:

Converting raw binary data into VDC
-----------------------------------

The two tools for converting raw binary data into VDC are ``vdccreate`` and ``raw2vdc``.  Advanced options can be seen here, or by issuing the commands without any arguments.

Create .vdc metadata file
_________________________

In the directory where Vapor 3 is installed, there is a command line utility called ``vdccreate``.  Issue this command in a terminal (Unix) or command prompt (Windows) with necessary arguments, followed by the name of the .vdc file to be written.  You must at least include the variable names in your dataset as well as your grid size.  Advanced options can be seen below.

.. code-block:: c

   vdccreate -dimension 512x512x512 -vars3d U:V:W myVDCFile.vdc

Advanced options for ``vdccreate``:

.. literalinclude:: ../commandLineTools/vdccreate.txt

Generate VDC Data
_________________

Once we have a .vdc file, the metadata has been recorded and we can transform the data into the VDC format.  From Vapor 3's installation directory, issue the command ``raw2vdc``, followed by the arguments for your target variable and target timestep.  The last argument will be the .vdc file that was made in Step 1.  ``raw2vdc`` must be run on each data file that contains values for a single variable, at a single timestep.

.. code-block:: c

   raw2vdc -ts 1 -varname U myVDCFile.vdc binaryFile.bin
   raw2vdc -ts 1 -varname V myVDCFile.vdc binaryFile.bin
   raw2vdc -ts 1 -varname W myVDCFile.vdc binaryFile.bin

Advanced options for ``raw2vdc``:

.. literalinclude:: ../commandLineTools/raw2vdc.txt
