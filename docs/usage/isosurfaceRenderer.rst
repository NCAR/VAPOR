.. _isosurfaceRenderer:

Isosurfaces
___________

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/bhkmRjAHOpo" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Isosurface Renderer displays surfaces that follow a single value of their selected variable.  This is similar to a 2D contour, but in 3D space.  Each instance of the Isosurface renderer can display up to four surfaces.  Initially, only one will be displayed.  Additional isosurfaces can be created by double-clicking on the Transfer Function historgram, in the Appearance tab.

Basic Controls
--------------

This renderer contains all of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.

Specialized Controls
--------------------

The Isosurface Renderer's Variables tab allows users to color the volume by a secondary variable.  This mans that the opacity of the volume can be controlled by a primary variable, and the color can bee attributed to a secondary variable.

Under the Appearance tab, the Isosurface Renderer also allows users to modify its Raytracing and Lighting parameters.

.. include:: commonControls/raytracingParameters.rst

.. include:: commonControls/lightingParameters.rst
