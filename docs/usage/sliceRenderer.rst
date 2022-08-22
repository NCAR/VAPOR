.. _sliceRenderer:

Slices 
______

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/cytoGmyCEDc" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Slice Renderer displays an axis-aligned slice or cutting plane through a 3D variable.  Slices are sampled along the plane's axes according to a sampling rate defined by the user.

Basic Controls
--------------

This renderer contains all of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.

Specialized Controls
--------------------

The Slice Renderer contains a *N Samples* adjustment parameter in the Appearance tab.  This renderer operates by sampling data values along the X and Y axis of a plane.  The *N Samples* parameter will increase the sampling rate used both axes for generating the Slice.
