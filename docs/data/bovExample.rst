:orphan:

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
