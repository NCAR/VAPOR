.. _isosurfaceRenderer:

Isosurfaces
___________

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/bhkmRjAHOpo" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Isosurface Renderer displays the user's 3D data variables within a volume described by the source data file, according to color and opacity settings defined by the user.

Basic Controls
--------------

This renderer contains all of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.

Specialized Controls
--------------------

The Isosurface Renderer's Variables tab allows users to color the volume by a secondary variable.  This mans that the opacity of the volume can be controlled by a primary variable, and the color can bee attributed to a secondary variable.

Under the Appearance tab, the Isosurface Renderer also allows users to modify its Raytracing and Lighting parameters.

Raytracing Parameters
`````````````````````

    *Rendering Algorithm* - Defaults to either Curvilinear or Regular, depending on the topology of the current variable's grid.  If using an older graphics card, the Isosurface Renderer may default to Rectilinear due to the increased complexity of the Curvilinear raytracer.

    *Sampling Rate Multiplier* - The volume renderer takes samples along the projected camera rays to determine the volume density at a point and the multiplier increases the number of samples which can give a more accurate visualization in certain cases.

Lighting Parameters
```````````````````

    *Apply Lighting* - Controls whether Vapor's default light sources are emitted onto the volume during rendering.  The default lights are directional.

    *Ambient* - Controls the amount of ambient light that is absorbed by the volume.  Ambient lights do not have a single directed source, and controls the overall brightness of the scene.

    *Specular* - Controls the angle of reflection for incident light on the volume.  In general, if a volume is very specular, it will appear to have more detail and depth.  It will appear flatter with smaller Specular parameterization.

    *Shininess* - Controls the overall reflection of light incident upon the volume.
~
