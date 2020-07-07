.. _contributing:

=====================
Contributing to Vapor
=====================

.. contents:: Table of contents:
   :local:
   :depth: 1

.. note::

  Large parts of this document came from the `xarray Contributing
  Guide <http://xarray.pydata.org/en/stable/contributing.html>`_.

Where to start?
---------------

All contributions are welcome.  The contributions that can be made include:

    - :ref:`Bug reports <contributing.bugReports>`
    - :ref:`Feature requests <contributing.bugReports>`
    - :ref:`Code contributions <contributing.git>`
    - :ref:`Documentation updates <contributing.documentation>`
    - :ref:`Your own visualizations to share in Vapor's gallery <contributing.visGallery>`

Vapor's current to-do list can be found on our `GitHub "issues" tab <https://github.com/ncar/vapor/issues>`_.  

If you’ve found an interesting issue that you would like to help fix, leave a comment on the issue that you would like to be assigned to it.  Assigning each issue to one or more individuals helps coordinaton among developers.

After assignment, you can refer to this guide to :ref:`set up your development environment <contributing.environment>`, and make your contribution.  Vapor's Architecture guide can help guide you on what part of the code base is relevant to your new feature.

Feel free to ask questions on the `Vapor Discourse Forum <https://vapor.discourse.group>`_.

.. _contributing.bugReports:

Bug Reports and Feature Requests
--------------------------------

To submit a new bug report or feature request, you can click the "New issue" button in the upper right, or `clicking on this link <https://github.com/NCAR/VAPOR/issues/new/choose>`_.

Bug reports are an important part of making *Vapor* more stable.  A bug report should have a brief description of the problem, as well as a list of steps to reproduce the problem.  See `this stackoverflow article <https://stackoverflow.com/help/mcve>`_ for tips on writing a good bug report.

Trying the bug-producing code out on the *master* branch is often a worthwhile exercise to confirm the bug still exists. It is also worth searching existing bug reports and pull requests to see if the issue has already been reported and/or fixed.

.. _contributing.git:

Code Contributions
---------------------

If you've found an issue you'd like to fix yourself, you'll need to compile *Vapor*, fix the issue at hand, and submit your changes for review and approval.  Prerequesite software includes:

    * Git for Vapor's version control software
      
    * CMake (version 3.2 or higher) for Vapor's its build system
      
    * One of the following compilers, dependent on which system its being built on:

        * OSX - LLVM 10.0.1 (clang-1001.0.46.4)
        * Ubuntu/CentOS - GCC 4.8.5 or higher
        * Windows - Microsoft Visual Studio 2015, version 14

Version control, Git, and GitHub
________________________________

To the new user, working with Git is one of the more daunting aspects of contributing to *Vapor*.  It can very quickly become overwhelming, but sticking to the guidelines below will help keep the process straightforward and mostly trouble free.  As always, if you are having difficulties please feel free to ask for help.

The code is hosted on `GitHub <https://www.github.com/NCAR/vapor>`_. To contribute you will need to sign up for a `free GitHub account <https://github.com/signup/free>`_. We use `Git <http://git-scm.com/>`_ for version control to allow many people to work together on the project.  It can also be acquired by package managers like apt-get (Ubuntu), yum (RedHat), and choco (Windows).

The `GitHub help pages <http://help.github.com/>`_ are a great place to get started with Git.

`GitHub also has instructions <http://help.github.com/set-up-git-redirect>`__ for installing git,
setting up your SSH key, and configuring git.  All these steps need to be completed before
you can work seamlessly between your local repository and GitHub.

.. _contributing.environment:

Forking Vapor's code
____________________

After installing Git and registering with GitHub, it's time to "Fork" Vapor's code base by clicking the Fork button on the upper right corner of `Vapor's GitHub repository <https://github.com/NCAR/VAPOR>`_.  This creates your own repository on GitHub that contains a copy of Vapor's current master branch.  Clone this repository to a suitable location on your local work machine.  This new remote repository is what will be merged with Vapor's master branch.  The next step is to compile Vapor's source code, and then begin making changes.

Download the Third-Party Libraries
__________________________________

After forking and cloning your new Vapor repository, you will need to download the third-party libraries that Vapor will link to.  Links to these libraries are below.  Be sure to select the correct download for the operating system you're building on.

*Windows*

Unzip the following file linked below into the root of your C:\\ directory.

    `Windows third party libraries <https://drive.google.com/open?id=1NRMu4_g8ZXu9bILBVRDsuUKIGBiT2016>`_

*Linux and OSX*

If building on Linux or OSX, the third party libraries must be placed in /usr/local/VAPOR-Deps/.

    `OSX third-party libraries <https://drive.google.com/open?id=1p47yeyjyUfxlc7-eglsXkjCGYJLqqtEs>`_

    `Ubuntu third-party libraries <https://drive.google.com/open?id=1j4IO4VCU0Wvyu2T3BH0e9I0qiiwCIrEd>`_

    `CentOS third-party libraries <https://drive.google.com/open?id=1e7F3kDoKctBmB3NOF4dES2395oScb9_0>`_

Build Vapor with CMake
______________________

On all operating systems, create a directory where the build will take place.  One option is to put the build directory inside of the Vapor source code directory.

    > cd VAPOR
    > mkdir build
    > cd build

On Windows, enter this build directory as the "Where to build the binaries" field in the CMake GUI.  Click *Configure*, *Generate*, and then *Open Project* in that order.  Visual Studio will open, and you can build the target *PACKAGE* to compile the source code.

On OSX and Linux, navigate to your build directory and run the command *cmake <source_directory>*, where <source_directory> is the root directory of Vapor's source code.  If the configuraiton was successful, you can then run *make* to compile Vapor.

    cmake .. && make

If compilation is successful, you can find Vapor's executable in the following locations

.. _contributing.architecture:

Adding to the Code Base
-----------------------

After successfully compiling Vapor, you can make changes to the code base.  Make sure to follow Vapor's `Code Conventions <https://github.com/NCAR/VAPOR/wiki/Vapor-Coding-Convention>`_, and eliminate all compiler warnings as you proceed.

What pieces of code you add or modify will depend on the issue you're trying to fix.  Most often, contributors will be doing one of two things:

.. toctree::
   :maxdepth: 1

   createDataReader
   createRenderer

Submitting Your Changes
-----------------------

After your implementation is complete, push your committs to your forked repository on GitHub.  You can then submit your changes for review through a GitHub Pull Request.  Vapor will automatically run a small set of tests to see if its internals are in tact, and check for warnings.  If these tests pass, Vapor's team will review the Pull Request to make sure that Vapor's `Code Conventions <https://github.com/NCAR/VAPOR/wiki/Vapor-Coding-Convention>`_ were honored, and that the logic and structure of the code is sound.  After review, further changes may be requested, or the Pull Request will be merged into Vapor's master repository.


`Vapor Coding Conventions <https://github.com/NCAR/VAPOR/wiki/Vapor-Coding-Convention>`_
________________________________________________________________________________________

.. _contributing.documentation:

Contributing to Vapor's Documentation
-------------------------------------

Vapor uses the Sphinx documentation generator.  

To contribute to Vapor's documentation, first follow these `instructions for installing Sphinx <https://www.sphinx-doc.org/en/master/usage/installation.html>`_.

Then navigate to <vapor-install-dir/docs/sphinx.  Within this directory you will find .rst files that are used to generate the HTML used for Vapor's documentation.  You can modify these .rst files, or add new ones.  Once the .rst files are complete, type *make ..* to generate the HTML, which will be built in <vapor-install-dir>/docs.  The HTML and .rst files can then be added to a pull request for review.

.. _contributing.visGallery:

Contributing to Vapor's Gallery
-------------------------------

Contributing to `Vapor's visualiation gallery <https://visgallery.ucar.edu/category/visualization-software/vapor/>`_ helps us understand how the application is being used, and can drive future requirements.  If you'd like to share your visualizations with us, you can submit them to our gallery by filling out the following form:

`Submit a visualization to Vapor's gallery <https://docs.google.com/forms/d/e/1FAIpQLSdJQA_1LGZ-DNySLGn4c9TekVuUcwvKo4Hgw3uBf9BmSDipUw/viewform?vc=0&c=0&w=1>`_



