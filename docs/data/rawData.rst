.. _rawData:

Raw binary data
---------------

Vapor can read binary data with the Brick of Values (BOV) reader.  If you have a data file that consists of one variable written as a FLOAT, DOUBLE, or INT, on a NX*NY*NZ rectilinear mesh, you have two options.  The first is the BOV data reader, and if the size of your data makes it hard to interact with your renderings, you can convertyour data into a Vapor Data Collection (VDC).

.. toctree::
   :maxdepth: 1

   Brick of Values (BOV)
   Converting to VDC

Brick of Values (BOV)
_____________________

Vapor's Brick of Values (BOV) file support was inspired by the VisIt project at the Lawerence Livermore National Laboratory.

When your data is output as a series of raw, binary values of types float, double, or int, you can use this data importer as long as you have the following:

    1) A series of individual data files, containing values regarding a single variable, at a single timestep

    2) A series of BOV header files, which describe the structure of each individual data file

Assuming you have a Fortran or C++ simulation providing the data files, all you need is a set of .bov header files that describe the structure of the data.

.. literalinclude:: bovExample.rst

 brief Class for reading a "Brick of Values", explained in section 3.1 (page 11) in the
 following VisIt document: https://github.com/NCAR/VAPOR/files/6341067/GettingDataIntoVisIt2.0.0.pdf

 The following BOV tags are mandatory for Vapor to ingest data:
 - DATA_FILE    (type: string)
 - DATA_SIZE    (type: three integer values that are >1)
 - DATA_FORMAT  (type: string of either INT, FLOAT, or DOUBLE)
 - TIME         (type: one floating point value.  May not be equal to FLT_MIN)

 The following BOV tags are optional:
 - BRICK_ORIGIN (type: three floating point values,   default: 0., 0., 0.)
 - BRICK_SIZE   (type: three floating point values,   default: 1., 1., 1.)
 - VARIABLE     (type: one alphanumeric string value, default: "brickVar")
 - BYTE_OFFSET  (type: one integer value,             default: 0)

 The following BOV tags are currently unsupported.  They can be included in a BOV header,
 but they will be unused.
 - DATA_ENDIAN
 - CENTERING
 - DIVIDE_BRICK
 - DATA_BRICKLETS
 - DATA_COMPONENTS

 Each .bov file can only refer to a single data file for a single variable, at a single timestep.
 If duplicate key/value pairs exist in a BOV header, the value closest to the bottom of the file will be used.
 If duplicate values exist for whatever reason, all entries must be valid (except for DATA_FILE, which gets validated after parsing)
 Scientific notation is supported for floating point values like BRICK_ORIGIN and BRICK_SIZE.
 Scientific notation is not supported for integer values like DATA_SIZE.
 DATA_SIZE must contain three values greater than 1.
 Wild card characters are not currently supported in the DATA_FILE token.
 VARIABLE must be alphanumeric (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-)


.. include:: ./importData.rst
