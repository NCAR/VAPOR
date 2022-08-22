.. _contourRenderer:

Contours
________

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/X4ziyt6qPZQ" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

Displays a series of user defined contours along a two dimensional plane within the user's domain.  

Basic Controls
--------------

This renderer contains all of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.

Specialized Controls
--------------------

The Contour Renderer has specialized controls in its Appearance tab, under the heading "Contour Properties".

    *Spacing* controls the incremental increase in data value between contours

    *Count* controls how many contours are currently being drawn

    *Contour Minimum* sets the value of the lowest valued contour in the series

    *N Samples* This renderer operates by sampling data values along the X and Y axis of a plane.  The *N Samples* parameter will increase the sampling rate used both axes for generating the contours among the plane.

.. include:: commonControls/heightVariable.rst
