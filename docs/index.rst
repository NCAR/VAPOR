.. Vapor documentation master file, created by
   sphinx-quickstart on Mon Feb 11 14:40:12 2019.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. .. image:: vaporNCARLogoWhite2.png    
..    :scale: 10%
..    :align: right

.. |
.. |

Vapor 3.5.0 is now Live!
========================

August 4, 2021

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Vapor's release cycle is accelerating, and now 3.5.0 is live.
New improvements include:
    - New particle data reader: Data Collection Particles (DCP)
    - New raw data reader: Brick of Values (BOV)- Bookmarks 
    - Easily, and visually save your state
    - Further improvements that streamline Vapor's GUI- Faster data access with OpenMP 

As well as many other reported issues, including the 1/4 canvas bug on BigSur.  Thanks to users ViggoHanst and Fortran for reporting this.

Thank you to everyone who has helped steer us by filling out our survey for 3.6 features.  The survey is still live, so please fill it out if you have 5 minutes.

Please `engage us on our forum <https://vapor.discourse.group/>`_.

Thank you!
-The Vapor Team

We need your feedback!
======================

May 21, 2021

We are trying to prioritize new features for Vapor 3.6.  Our goal has always been to support scientists and help them produce superior visualizations of their simulation data. We can't do this without the feedback of the people in our user community.

Please help us by providing your feedback in the 5 minute survey below, which outlines new features we are considering for Vapor 3.6 and beyond. 

`New Feature Survey <https://docs.google.com/forms/d/e/1FAIpQLSeZWvuAXaRiWyFrQ16zO25bfy8AANp8C8HpVXeMk83uQPdTLA/viewform?usp=sf_link>`_

Each feature in the survey is linked to an issue on GitHub (https://github.com/NCAR/VAPOR), so feel free to comment there if you'd like to track our progress, or discuss the issue further after the survey. And please let us know if you have new ideas that we haven't considered yet.

Thank you for helping us produce better open source visualization software.

-The Vapor Team

Vapor 3.4.0 is now Live!
========================

April 5, 2021

`Download here <https://forms.gle/piowN9Lnd3oZhno79>`_

Version 3.4 is Vapor's first usability-focused release.  See some of the new features in the following demo:

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/8FabwuxxbDo" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

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

A comprehensive list of fixes can be viewed in `the 3_4_0 release milestone. <https://github.com/NCAR/VAPOR/issues?q=is%3Aissue+milestone%3A%223_4_0+release%22+is%3Aclosed>`_

|

Vapor 3
=================================

VAPOR is the Visualization and Analysis Platform for Ocean, Atmosphere, and Solar Researchers. VAPOR provides an interactive 3D visualization environment that can also produce animations and still frame images. VAPOR runs on most UNIX and Windows systems equipped with modern 3D graphics cards.

The VAPOR Data Collection (VDC) data model allows users progressively access the fidelity of their data, allowing for the visualization of terascale data sets on commodity hardware. VAPOR can also directly import data formats including WRF, MOM, POP, ROMS, and some GRIB and NetCDF files.

Users can perform ad-hoc analysis with VAPOR’s interactive Python interpreter; which allows for the creation, modification, and visualization of new variables based on input model data.

VAPOR is a product of the National Center for Atmospheric Research’s Computational and Information Systems Lab. Support for VAPOR is provided by the U.S. National Science Foundation (grants # 03-25934 and 09-06379, ACI-14-40412), and by the Korea Institute of Science and Technology Information

.. image:: _images/vaporBanner.png


.. toctree::
   :caption: Contents:

   downloads
   quickStartGuide
   gettingDataIntoVapor
   usage
   YouTube Channel <https://www.youtube.com/channel/UCpf-d1GDO1sotzjJ2t_QkDw>
   Get Help on Our Forum <https://vapor.discourse.group>
   contributing
   licenseAndCitation

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
