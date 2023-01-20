:orphan:

Image Renderer
==============

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/84sWTNgUvUU" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Image Renderer displays a georeferenced image that is automatically reprojected and fit to the user's data.  The image may be offset by a height variable to show bathymetry or mountainous terrain.

Basic Controls
--------------

This renderer contains a subset of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, and :ref:`Geometry <geometryTab>` tabs.

The Image Renderer's Appearance tab does not contain a transfer function since it is not rendering any variable with color or opacity.

Georeferenced Images
--------------------

Georeferenced images may be selected in the Appearance tab.  Vapor comes bundled with two georeferenced image products: *NaturalEarth*, and *BigBlueMarble*.  However these are global images that may not have the resolution you need, depending on the size of your simulation domain.

The example below (makeGeotiff.py) will show you how to generate your own georeferenced image using NASA's WorldView map server.  For a list of available WorldView layers, you can see :ref:`this document <worldViewLayers>`.  These layers can also be previewed at `NASA's EarthView webpage <https://worldview.earthdata.nasa.gov/>`_.

.. include:: ../commonControls/heightVariable.rst
