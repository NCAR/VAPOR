.. _contributorsGuide:

===================
Contributor's Guide
===================

All contributions, bug reports, bug fixes, documentation improvements, enhancements, and ideas are welcome and encoraged, to help steer to Vapor on its mission.  These can be submitted on our `GitHub issue tracker <https://github.com/ncar/vapor/issues>`_ by clicking on the `New issue` button in the upper right, or `clicking on this link <https://github.com/NCAR/VAPOR/issues/new/choose>`_.

If you’ve found an interesting issue that you would like to help fix, you can refer to our Building From Source guide to get your development environment setup.  After that, our Vapor Architecture guide can help guide you on what part of the code base is relevant to your new feature.

Feel free to ask questions on the `Vapor Discourse Forum <https://vapor.discourse.group>`_.

Prerequisites
-------------
Git is used for Vapor's version control software.  It's avaialable on all platforms, and can be acquired by package managers like apt-get (Ubuntu), yum (RedHat), and choco (Windows).  Some familiarity with Git is required for code contributions.  GitHub is used as Vapor's source code repository, so contributors will need an account there as well.

Vapor uses CMake (version 3.2 or higher) for its build system.

Finally, Vapor is normally built with one of the following compilers, dependent on which system its being built on. 

OSX - LLVM 10.0.1 (clang-1001.0.46.4)
Ubuntu/CentOS - GCC 4.8.5 or higher
Windows - Microsoft Visual Studio 2015, version 14

Forking Vapor's code
--------------------
After installing Git and registering with GitHub, it's time to "Fork" Vapor's code base by clicking the Fork button on the upper right corner of `Vapor's GitHub repository <https://github.com/NCAR/VAPOR>`_.  This creates your own repository on GitHub that contains a copy of Vapor's current master branch.  Clone this repository to a suitable location on your local work machine.  This new remote repository is what will be merged with Vapor's master branch.  The next step is to compile Vapor's source code, and then begin making changes.

Download the Third-Party Libraries
----------------------------------

After forking and cloning your new Vapor repository, you will need to download the third-party libraries that Vapor will link to.  Links to these libraries are below.  Be sure to select the correct download for the operating system you're building on.

*Windows*

Unzip the following file linked below into the root of your C:\\ directory.

    `Windows third party libraries <https://drive.google.com/open?id=1NRMu4_g8ZXu9bILBVRDsuUKIGBiT2016>`_

*Linux and OSX*

If building on Linux or OSX, the third party libraries must be placed in /usr/local/VAPOR-Deps/.

    `OSX third-party libraries <https://drive.google.com/open?id=1p47yeyjyUfxlc7-eglsXkjCGYJLqqtEs>`_

    `Ubuntu third-party libraries <https://drive.google.com/open?id=1j4IO4VCU0Wvyu2T3BH0e9I0qiiwCIrEd>`_

    `CentOS third-party libraries <https://drive.google.com/open?id=1e7F3kDoKctBmB3NOF4dES2395oScb9_0>`_

CMake & make
------------

On all operating systems, create a directory where the build will take place.  One option is to put the build directory inside of the Vapor source code directory.

    > cd VAPOR
    > mkdir build
    > cd build

On Windows, enter this build directory as the "Where to build the binaries" field in the CMake GUI.  Click *Configure*, *Generate*, and then *Open Project* in that order.  Visual Studio will open, and you can build the target *PACKAGE* to compile the source code.

On OSX and Linux, navigate to your build directory and run the command *cmake <source_directory>*, where <source_directory> is the root directory of Vapor's source code.  If the configuraiton was successful, you can then run *make* to compile Vapor.

    cmake .. && make

If compilation is successful, you can find Vapor's executable in the following locations

Submitting Code Changes
-----------------------

After successfully compiling Vapor, you can make changes to the code base.  What pieces of code you add or modify will depend on the issue you're trying to fix.  Most often, contributors will be doing one of two things:

.. toctree::
   :maxdepth: 1

   createDataReader
   createRenderer


.. _contributingToVaporsDocumentation:
Contributing to Vapor's Documentation
-------------------------------------

.. _submittingVisToGallery:
Contributing to Vapor's Gallery
-------------------------------



.. _getHelp:

`Get help on our forum <https://vapor.discourse.group/>`_
__________________________________________________________________________

.. _reportABug:

`Report a bug <https://github.com/NCAR/VAPOR/issues/new/choose>`_
_________________________________________________________________

.. _submitAnEnhancement:

`Request an enhancement <https://github.com/NCAR/VAPOR/issues/new/choose>`_
___________________________________________________________________________

.. _forkOnGithub:

`GitHub Repository <https://github.com/NCAR/VAPOR>`_
_______________________________________________________


.. _readCodingConvention:

`Read our coding conventions <https://github.com/NCAR/VAPOR/wiki/Vapor-Coding-Convention>`_
___________________________________________________________________________________________
