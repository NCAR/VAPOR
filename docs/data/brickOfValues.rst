.. _brickOfValues:

Brick of Values (BOV)
_____________________

Vapor's Brick of Values (BOV) file support was inspired by the VisIt project at the Lawerence Livermore National Laboratory.

When your data is output as a series of binary files that have values of type float, double, or int, you can use this data importer as long as:

    1) Each data file contains values for a single variable, at a single timestep

    2) You've written a BOV header file that describes the structure of a corresponding data file

    3) Your data is on a 3D regular grid*

.. note::

   *For more information on the regular grids supported by the BOV reader, see `this wikipedia article <https://en.wikipedia.org/wiki/Regular_grid>`_. 

BOV header files
----------------

The BOV header files that describe the structure of your data are just a list of key/value pairs.  Some keys are mandatory, and some are optional.  See below for descriptions of these keys, special rules that pertain to the keys and their values, and an example file.

Example BOV header file
-----------------------

.. code-block:: c

   # TIME is a floating point value specifying the timestep being read in DATA_FILE
   TIME: 1.1

   # DATA_FILE points to a binary data file.  It can be a full file path, or a path relative to the BOV header.
   DATA_FILE: bovA1.bin

   # The data size corresponds to NX,NY,NZ in the above example code.  It must contain three values
   DATA_SIZE: 10 10 10

   # Allowable values for DATA_FORMAT are: INT,FLOAT,DOUBLE
   DATA_FORMAT: FLOAT

   # VARIABLE is a string that specifies the variable being read in DATA_FILE.  Must be alphanumeric (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-)
   VARIABLE: myVariable

   # BRICK_ORIGIN lets you specify a new coordinate system origin for # the mesh that will be created to suit your data.  It must contain three values.
   BRICK_ORIGIN: 0. 0. 0.

   # BRICK_SIZE lets you specify the size of the brick on X, Y, and Z.  It must contain three values.
   BRICK_SIZE: 10. 20. 5.

   # BYTE_OFFSET is optional and lets you specify some number of
   # bytes to skip at the front of the file. This can be useful for # skipping the 4-byte header that Fortran tends to write to files. # If your file does not have a header the
   BYTE_OFFSET. BYTE_OFFSET: 4

BOV header file requirements and options
----------------------------------------

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

The following BOV tags are currently unsupported.  They can be included in a BOV header, but they will be unused:
    - DATA_ENDIAN
    - CENTERING
    - DIVIDE_BRICK
    - DATA_BRICKLETS
    - DATA_COMPONENTS

Special rules:
    - Each .bov file can only refer to a single data file for a single variable, at a single timestep
    - If duplicate key/value pairs exist in a BOV header, the value closest to the bottom of the file will be used
    - If duplicate values exist for whatever reason, all entries must be valid (except for DATA_FILE, which gets validated after parsing)
    - Scientific notation is supported for floating point values like BRICK_ORIGIN and BRICK_SIZE
    - Scientific notation is not supported for integer values like DATA_SIZE
    - DATA_SIZE must contain three values greater than 1
    - Wild card characters are not currently supported in the DATA_FILE token
    - VARIABLE must be alphanumeric, only containing the following characters: abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-

.. include:: ./importData.rst
