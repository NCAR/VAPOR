.. _imageRenderer:

Image Renderer
______________

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/84sWTNgUvUU" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Image Renderer displays a georeferenced image that is automatically reprojected and fit to the user's data, as long as the data contains georeference metadata.  

The Image Renderer may be offset by a height variable to show bathymetry or mountainous terrain.

Basic Controls
--------------

This renderer contains a subset of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, and :ref:`Geometry <geometryTab>` tabs.

The Image Renderer's Appearance tab does not contain a transfer function since it is not rendering any variable with color or opacity.

Specialized Controls
--------------------

Georeferenced images may be selected for rendering in the Appearance tab.  Vapor comes bundled with two georeferenced image products: *NaturalEarth*, and *BigBlueMarble*.

If an image contains transparency, this transparency can be toggled with the *Ignore transparency* checkbox.  Vapor's BigBlueMarble contains transparency over its ocean regions so that sub-surface ocean models may be viewed alongside land masses.
