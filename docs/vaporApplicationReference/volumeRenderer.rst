.. _volumeRenderer:

Volume Renderer
_______________

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/Td6fZap4HAw" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Volume Renderer displays the user's 3D data variables within a volume described by the source data file, according to color and opacity settings defined by the user.

Basic Controls
--------------

This renderer contains all of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.

The Volume Renderer additionally has specialized controls in its Variables and Appearance tabs.

Specialized Controls
--------------------

The Volume Renderer's Variables tab allows users to color the volume by a secondary variable.  This mans that the opacity of the volume can be controlled by a primary variable, and the color can bee attributed to a secondary variable.

Under the Appearance tab, the Volume Renderer also allows users to modify its Raytracing and Lighting parameters.

.. include:: commonControls/raytracingParameters.rst

.. include:: commonControls/lightingParameters.rst
