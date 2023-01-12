.. _barbRenderer:

Barbs
_____

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/wcvpdoaWB1M" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Barb Renderer displays an array of arrows with the users domain, with custom dimensions that are defined by the user in the X, Y, and Z axes.  The arrows represent a vector whose direction is determined by up to three user-defined variables. Barbs can have a constant color applied to them, or they may be colored according to an additional user-defined variable.

Basic Controls
--------------

This renderer contains all of Vapor's standard renderer controls: the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.

The Barb Renderer can operate on either two-dimensional, or three-dimensional field variables.  This is defined in the Variables tab.

Specialized Controls
--------------------

The Barb's Appearance tab contains controls as follows:

*X Dimension* - Controls how many barbs to uniformly distribute along the X axis of the Barb Renderer's region.

*Y Dimension* - Controls how many barbs to uniformly distribute along the Y axis of the Barb Renderer's region.

*Z Dimension* - Controls how many barbs to uniformly distribute along the Z axis of the Barb Renderer's region.  This field is only enabled when rendering barbs with 3D variables.

*Length Scale* - A unitless value that controls how long the barbs are.  This value is derived by the values of the field variables at the data set's initial timestep, or the timestep at which *Recalculate Scales* is pressed.

*Thickness Scale* - A unitless value that controls how thick the barbs are.  This value is derived by the size of the data set's domain.

*Recalculate Scales* - Recalculates the unitless values for *Length Scale* and *Thickness Scale* at the current timestep.

.. include:: commonControls/heightVariable.rst
