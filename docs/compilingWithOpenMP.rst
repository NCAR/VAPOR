:orphan:

.. _compilingWithOpenMP:

=====================
Compiling with OpenMP
=====================

Some of Vapor's libraries have been optimized with the OpenMP API.  To build Vapor with these optimizations, ensure that your compiler supports OpenMP, and enable the following flags through `ccmake`,or by passing the arguments into `cmake`.

Our team builds Vapor with OpenMP with LLVM 13+ and GCC 4.2+.

Manually setting flags with ccmake
----------------------------------

To generate an optimized release build with OpenMP, set the following fields with ccmake.

+------------------------+----------------------------+
| CPACK_BINARY_DRAGNDROP | ON                         |
+------------------------+----------------------------+
| CMAKE_BUILD_TYPE       | Release                    |
+------------------------+----------------------------+
| DIST_INSTALLER         | ON                         |
+------------------------+----------------------------+
| CMAKE_CXX_COMPILER     | /path/to/your/c++/compiler |
+------------------------+----------------------------+
| CMAKE_C_COMPILER       | /path/to/your/c/compiler   |
+------------------------+----------------------------+
| USE_OMP                | ON                         |
+------------------------+----------------------------+

Command line CMake build incantation
------------------------------------

Alternatively, cmake can apply these arguments through command-line arguments as follows:

.. code-block:: bash

    cmake \
    -DCPACK_BINARY_DRAGNDROP=ON \
    -DCMAKE_BUILD_TYPE:String=Release \
    -DDIST_INSTALLER:string=ON \
    -DCMAKE_CXX_COMPILER=/opt/local/bin/clang++ \
    -DCMAKE_C_COMPILER=/opt/local/bin/clang \
    -DUSE_OMP=ON .. \
    && make \
    && make installer \
    /
