.. _downloads:

=========
Downloads
=========

.. note:: If you are looking for our legacy version of Vapor 2.6, follow :ref:`this link <vapor2>` to the bottom of the page.

Weekly Build
------------

Vapor builds installers on a weekly basis.  These have new features that have not been tested, and may be unstable.

`Download here <https://github.com/NCAR/VAPOR/releases/tag/Weekly>`_

Vapor 3.5.0 is now Live!
------------------------

August 4, 2021

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Vapor's release cycle is accelerating, and now 3.5.0 is live.

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/WPRpeXVXedM" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

New improvements include:
- New particle data reader: Data Collection Particles (DCP)
- New raw data reader: Brick of Values (BOV)
- Easily, and visually save your state with the new Bookmarks feature
- Further improvements that streamline Vapor's GUI
- Faster data access with OpenMP

As well as many other reported issues, including the 1/4 canvas bug on BigSur.  Thanks to users ViggoHanst and Fortran for reporting this.

Thank you to everyone who has helped steer us by filling out our survey for 3.6 features.  The survey is still live, so please fill it out if you have 5 minutes.

Please `engage us on our forum <https://vapor.discourse.group/>`_.

Thank you!

-The Vapor Team

Vapor 3.4.0
-----------

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/8FabwuxxbDo" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Major usability improvements include:

    - Improved algorithm for the Flow renderer's random seeding bias variable
    - Data-independent settings (e.g. Cache size) can now be modified before data is loaded
    - Restructuring of Vapor's top-level tabs
    - New and improved Colorbars
    - Improved functionality of the Geometry tab
    - Better space management of the Renderer Table

Other notable issues include:

    - Addition of clang-format linter, and git pre-push hook
    - Fixed weekly builds on all platforms

A comprehensive list of fixes can be viewed in the `3_4_0 release milestone <https://github.com/NCAR/VAPOR/issues?q=is%3Aissue+milestone%3A%223_4_0+release%22+is%3Aclosed>`_.

Vapor 3.3.0
-----------

December 17, 2020

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Release notes for VAPOR-3.3.0

    - Introduced new flow rendering algorithms, including pathlines and streamlines rendered in 3D, as well as the option for density colormapping
    - The Flow renderer can now sample any set of data values along a pathline or streamline, and write those samples to a text file
    - Included a prototype Volume Renderer using Intel's Ospray engine, allowing for 3D rendering of unstructured models like MPAS
    - Improved Vapor's GUI components and architecture for more consistent behavior and improved usability
    - Vapor's sliders no longer have pre-set limits, and can now have their range of values adjusted through a menu opened with a right-mouse click
    - Multiple performance optimizations to Vapor's internal data management system, and the Flow and Image renderers
    - New colormaps, taken from MatPlotLib's cmocean package
    - Updates to Vapor's third-party libraries

|

Vapor 3.2.0
-----------

February 3, 2020 

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Release notes for VAPOR-3.2.0

    New Features:

    - Flow Renderer
    - Model Renderer
    - New Transfer Function Editor
    - Off screen rendering
    - Performance optimization to Vapor's DataMgr class
    - Added support for stretched grids to vdccreate
    - Added ability to color Volume Renderings with a secondary variable
    - Increased Volume Rendering sampling rate maximum setting
    - Updated 3rd party libraries

|

.. _installationInstructions:

Installation Instructions
-------------------------

We encourage users of Vapor to install with the methods described here.  If you're a developer and would like to contribute, see the :ref:`Contributing to Vapor <contributing>` section.

**Linux**

Run the downloaded .sh script in a terminal window.  It will prompt you as to where the binaries will be installed. For example:
 
::

    % sh VAPOR3-3.0.0.beta-RH7.sh

A demonstration for installation on Linux systems can be viewed here:

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/QImhCy_zLO0?start=509" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

**OSX**

Double click on the downloaded .dmg file.  Once the Finder window pops up, drag the Vapor icon into the Applications folder.

A demonstration for installing on OSX systems can be viewed here:

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/QImhCy_zLO0?start=56" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

**Windows**

Run the downloaded .exe file.  A wizard will step you through the installer settings necessary for setup.

A demonstration for installing on Windows systems can be viewed here:

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/QImhCy_zLO0?start=1195" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

.. _sampleData:

Sample Data
-----------

+--------------+-------+-------------------+-----------+
| Dataset      | Model | Grid Resolution   | File Size |
+--------------+-------+-------------------+-----------+
| DUKU_        | WRF   | 181 x 166 x 35    | 734 MB    |
+--------------+-------+-------------------+-----------+
| Kauffman_    | ROMS  | 226 x 642 x 43    | 495 MB    |
+--------------+-------+-------------------+-----------+

.. _DUKU: https://dashrepo.ucar.edu/dataset/VAPOR_Sample_Data/file/dukuSample.tar.gz

.. _Kauffman: https://dashrepo.ucar.edu/dataset/VAPOR_Sample_Data/file/kauffmanSample.tar.gz

.. note:: Users can download a 500 meter resolution image of NASA's `BigBlueMarble <https://drive.google.com/open?id=1qIwh8ZJj67d85ktkjpgOVBAE-oMRi3rD>`_ for use in Vapor's Image Renderer.

|

Previous Releases
-----------------

Vapor 3.1.0
```````````

July 5, 2019

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Release notes for VAPOR-3.1.0

    New Features:

    - 3D Variable Support
    - Direct Volume Renderer
    - Isosurfaces
    - Slice Renderer
    - Wireframe Renderer
    - Python variable engine
    - Geotiff creation from Vapor renderings
    - Support for MPAS-A and MOM6 models

|

.. _vapor2:

Vapor 2
```````

Vapor 2 is deprecated, and we strongly encourage users to download the currently supported releases of Vapor 3.

If you are interested in using Vapor 2, it can be `downloaded after filling out a short survey <https://forms.gle/ZLX7oZ7LYAVEEBH4A>`_.

Legacy documentation for Vapor 2 can be found `here <https://ncar.github.io/vapor2website/index.html>`_.  Please note that this website is no longer supported, and some links may be broken.  Use at your own discretion.

Vapor 2 source code can be found `here <https://sourceforge.net/projects/vapor/>`_.
