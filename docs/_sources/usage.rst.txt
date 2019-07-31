.. _usage:

=============
Using Vapor 3
=============

Vapor 3 is comprised of a set of tools called Renderers.  Each Renderer visualizes your data in different ways based on your specifications.

We recommend that all users either watch our :ref:`Introductory Tutorial <introTutorial>` or work through the :ref:`quickStartGuide`.

If any feature in Vapor is not sufficienty self describing in the application, this is where to find elaboration.  Please :ref:`contact our team <contactAndContribute>` if you think you may have found a bug, usability issue, or you'd like to request an enhancement.

.. raw:: html

    <iframe width="560" height="315" src="https://youtu.be/CtXHdI4WUDE" frameborder="0" allowfullscreen></iframe>

.. _renderers:

The Renderers
-------------

Volume Renderer
_______________

Displays the user's 3D data variables within a volume described by the source data file, according to color and opacity settings defined by the user.

.. figure:: ../_images/DVR.png
    :align: center
    :figclass: align-center

Isosurfaces
___________

Displays the user's 3D data variables within a volume described by the source data file, according to color and opacity settings defined by the user.

.. figure:: ../_images/IsoSurface.png
    :align: center
    :figclass: align-center

Slices
______

Displays an axis-aligned slice or cutting plane through a 3D variable.  Slices are sampled along the plane's axes according to a sampling rate defined by the user.

.. figure:: ../_images/Slice.png
    :align: center
    :figclass: align-center

Contours
________

Displays a series of user defined contours along a two dimensional plane within the user's domain.  Contours may have constant coloration.  Contours may be displaced by a height variable.

.. figure:: ../_images/Contours.png
    :align: center
    :figclass: align-center

Barbs
_____

Displays an array of arrows with the users domain, with custom dimensions that are defined by the user in the X, Y, and Z axes.  The arrows represent a vector whose direction is determined by up to three user-defined variables. Barbs can have a constant color applied to them, or they may be colored according to an additional user-defined variable.

The Barb renderer may be offset by a height variable, if the barbs are referencing two-dimensional variables.

.. figure:: ../_images/Barbs.png
    :align: center
    :figclass: align-center

Two Dimensional Variables
_________________________

Displays the user's 2D data variables along the plane described by the source data file. These 2D variables may be offset by a height variable.

.. figure:: ../_images/TwoDData.png
    :align: center
    :figclass: align-center

Georefernced Images 
___________________

Displays a georeferenced image that is automatically reprojected and fit to the user's data, as long as the data contains georeference metadata.  The image renderer may be offset by a height variable to show bathymetry or mountainous terrain.

.. figure:: ../_images/Image.png
    :align: center
    :figclass: align-center

Wireframes
__________

Displays a wireframe of the mesh for the selected variable

.. figure:: ../_images/WireFrame.png
    :align: center
    :figclass: align-center

|

.. _controllingYourRenderers:

Controlling The Renderers
-------------------------

Each of Vapor's renderers will create imagery of your variables according to color, opacity, a region of interest, and sometimes a few more specialized parameters.  

Each renderer is unique, the specification of their parameters is mostly the same.  All renderers are controlled by four tabs:

    - :ref:`Variables <variablesTab>`
    - :ref:`Appearance <appearanceTab>`
    - :ref:`Geometry <geometryTab>`
    - :ref:`Annotation <annotationTab>`

See the :ref:`Renderers <renderers>` section for more information on how each of these tabs work for a given renderer.  Again, they all operate in the same way for the most part.

.. figure:: ../_images/variablesTab.png
    :align: center
    :width: 500 
    :figclass: align-center

    Variables tab for the Slice renderer

.. _variablesTab:

Variables Tab
_____________
The Variables Tab allows the user to define what variables are used as input to a renderer.  The options presented to the user in this tab depend on the renderer currently being used.

Users that have converted their data into :ref:`VDC <vdc>` will have a fidelity controller, which allows them to view compressed data to speed up their rendering time.  Making a visualization interactive lets you change parameters faster, so you can crank up the fidelity of your data for a final visualization after exploring first.

.. _appearanceTab:

Appearance Tab
______________
The appearance tab controls the color, opacity, and any renderer-specific parameters of your renderer.  Color and opacity are controlled by the Transfer Function.  Renderer-specific parameters will be grouped togheter within the Appearance Tab.  See the :ref:`Renderers <renderers>` section for more info on renderer-specific parameters.

The Transfer Function consists of a `Probability Density Function (PDF) <https://en.wikipedia.org/wiki/Probability_density_function>`_ of your currently selected variable.  Underneath the PDF is a color bar that shows the colors that get applied to the values located directly above it.

.. figure:: ../_images/transferFunctionDocumentation.png
    :align: center
    :figclass: align-center

    Vapor 3's Transfer Function editor

In the figure above, we can see that our transfer function is operating on the variable P.  The range of values within the transfer function are -1314.76 to 1268.32.  All values of P less than 1314.76 are colored deep blue.  The coloration transitions into red at the high end of the PDF, until becoming saturated at values of 1268.32 and higher.

Below the histogram is a button to update the histogram, which is calculated only when the user requests it to save on compute time.  Options to change the color interpolation type are also available.

.. figure:: ../_images/TFOptions.png
    :align: center
    :width: 500
    :figclass: align-center

    Additional options for the Transfer Function

.. _controllingColor:

Controlling Color
"""""""""""""""""

Vapor's default color map is called CoolWarm.  This is arbitrary, and may not suit your needs.  Vapor bundles several other color maps that can be found by pressing the "Load TF" button at the top of the Appearance tab.

The colors in the color map be moved by creating a color-control-point, and dragging it.  To create a new color-control-point, right click on the Colorbar, and then click "New Color Control Point."  The color at this control point may now be dragged to suit your needs.

.. figure:: ../_images/colorControlPoint.gif
    :align: center
    :figclass: align-center

    Adding and moving color control points in the Colorbar

These control points may also be given direct color values by either double clicking them, or right-clicking and selecting "Edit color control point".  After a color has been changed, Vapor will interpolate between control points to give a smooth color transition.

Controlling Opacity
"""""""""""""""""""

Opacity is controlled by the green line on top of the PDF.  The higher this green line is on the PDF's Y axis, the more opaque the colors will be at that point.  For example, the green bar is set to Y=0 over the blue values in the image below.  All of these values will be masked out.  The green bar then ramps up, and the values become more opaque, until we reach full opacity in the red region.

.. figure:: ../_images/opacityMap.png
    :align: center
    :width: 500 
    :figclass: align-center

    Blue values are hiden completely.  White values ramp up from transparent to opaque, and red values are fully opaque.

.. _geometryTab:

Geometry Tab
____________

The Geometry tab controls where your renderer is drawing, within the space of your simulation.  By excluding regions of data from being drawn, occluded features may be seen more clearly. Compute time will also be reduced, as well as the memory needed for a given renderer.

.. figure:: ../_images/geometryWidget.png
    :align: center
    :width: 500
    :figclass: align-center

    Coordinate selector in the Geometry Tab
    
If you have a region of interest in another renderer, that region can be copied in the Geometry tab.

.. figure:: ../_images/copyRegionWidget.png
    :align: center
    :width: 500 
    :figclass: align-center

    Copy geometry from one renderer to another


Users can apply transforms to scale, translate, and rotate their renderers on X, Y, or Z.  The origin used for these transforms may also be adjusted.

.. figure:: ../_images/transformTable.png
    :align: center
    :width: 500 
    :figclass: align-center

    Transformation options within the Geometry widget

.. _regionMouseMode:

Users may also control the geometry of their renderer by using the `Region Mouse Mode`, located at the top left corner of the application.  This will enable a red box with handlebars that can be right-clicked to grow or shrink the region being rendered on any axis.

.. figure:: ../_images/selectRegionMouseMode.png
    :align: center
    :width: 400 
    :figclass: align-center

    Select the Region Mouse Mode for interactive geometry adjustment

.. figure:: ../_images/regionMouseMode.png
    :align: center
    :width: 500 
    :figclass: align-center

    Interactive geometry controls alongside a Barb renderer, after activating the Region Mouse Mode

.. _annotationTab:

Annotation Tab
______________

Quantifying the colors to your viewers can be done by adding a colorbar in the Annotation tab.

.. figure:: ../_images/colorbarTab.png
    :align: center
    :width: 500 
    :figclass: align-center

    Colorbar size and position controlls, located in the Annotation tab

.. figure:: ../_images/colorbar.png
    :align: center
    :figclass: align-center

    An exmaple colorbar

|

Unique Renderer Controls
------------------------

Some renderers have unique controls that do not exist elsewhere.

Raycasting Renderers
____________________

The Volume and Isosurface renderers perform a method called raycasting, where a line is drawn from each pixel on the screen into the loaded dataset.  Each line samples data values and returns a color, according to what's been configured in the Transfer Function.  Additional appearance settings for raycasting renderers are naturally found in the Appearance tab.

.. figure:: ../_images/raycastingOptions.png
    :align: center
    :width: 500 
    :figclass: align-center

    Raycasting options in the Volume and Isosurface renderers, in the Appearance tab.

Isosurface Color Controls
_________________________

The Isosurface Appearance tab is unique in that it allows the user to select up to four values to draw isosurfaces with.

.. figure:: ../_images/isovalueSelector.png
    :align: center
    :width: 500 
    :figclass: align-center

Isosurfaces can be colored by a secondary variable, and therefore have a Transfer Function that is split into an isovalue selector, and a color mapping.  When the user disables the "Use Constant Color" checkbox, the Color Mapped Variable tab will be enabled.

.. figure:: ../_images/isosurfaceTF.png
    :align: center
    :width: 750 
    :figclass: align-center

    The isosurface Transfer Funciton, displaying the current isovalue in the variable PDF, and the Transfer Function for the isosurface's Color Mapped Variable.

Height Variable Offsets
_______________________

The TwoDData, Barb, Contour, Image, and Wireframe renderers all have the option to offset the data by a height variable.

.. figure:: ../_images/contourHeightSelection.png
    :align: center
    :width: 500 
    :figclass: align-center

    Selecting a height variable in the Contour renderer

.. figure:: ../_images/contourWithHeight.png
    :align: center
    :width: 500 
    :figclass: align-center

    The resultant contour plot, offset by the height variable HGT

Slice Quality Control
_____________________

The Slice renderer samples variable data at a fixed rate across a two dimensional plane.  Usually the default sampling rate is sufficient, but it can be increased with the Quality controller in the Appearance tab.

.. figure:: ../_images/sliceQualityAdjustment.png
    :align: center
    :width: 500 
    :figclass: align-center

    Quality adjustment for the Slice renderer

Contour Selection
_________________

The Appearance tab for the Contour renderer includes controls for the contour count, spacing between contours, minimum contour value, and the width of the contours being rendered.

.. figure:: ../_images/contourAppearance.png
    :align: center
    :width: 500 
    :figclass: align-center

    Controls for the Contour renderer


Barbs Selection
_______________

The Barb renderer operates on a set of vectors to determine which direction they point in.  Users need to select variables to correspond with the X, Y, and optionally Z vectors in their dataset.

Users may also offset the barbs by a height variable, and color them according to an additional variable if desired.  For example, users may have wind barbs being drawn based on their U, V, and W variables, and colored by their Pressure variable.

.. figure:: ../_images/barbVariableSelector.png
    :align: center
    :width: 500 
    :figclass: align-center

    Variable selector for the Barb renderer

Georeferenced Images
____________________

The Image renderer is the only one that does not have a Transfer Function.  All the user needs to do is select either one of Vapor's bundled GeoTiff images, or one that they have made themselves.

The Image renderer is also the only renderer than can extend beyond the domain of the user's data.  This can be done by switching to the :ref:`Region Mouse Mode <regionMouseMode>`.

.. figure:: ../_images/imagePastDomain.png
    :align: center
    :width: 500 
    :figclass: align-center

    An Image renderer that is drawing outside the data domain, using the Region Mouse Mode

Navigation Settings
-------------------

At the top level of Vapor's control menu, there is a top-level tab called Navigation, which contains settings that help users identify and visualize where they are in the scene.  The Navigation tab is composed of an Annotation tab, and a Viewpoint tab.

Annotations
___________

In the Annotaitons tab, users can add Axis Annotations, Time Annotations, and 3D arrows that indicate which direction the X, Y, and Z axes are oriented in.  Users can also control whether they want to render bounding boxes that indicate the extents of their domain.

Animation
_________

Currently in development.

Viewpoint
_________

The Viewpoint tab contains tools that let the user apply global transforms to datasets that they have loaded.  This is similar to how individual renderers can be transformed, but in this case the transform applies to all renderers in a dataset.

Projection strings can also be modified if a dataset is georeferenced.

Finally, camera position and direction values are displayed here and may be changed numerically for convenience.

.. figure:: ../_images/viewpointTab.png
    :align: center
    :width: 500 
    :figclass: align-center

    The Viewpoint Tab, within the top-level Navigation tab

Global Settings
---------------

The last top-level tab next to the Renderers and Navigation tabs is called Settings.  This is where Vapor's session file save frequency is set, as well as programatic settings like window sizes and cache sizes.

.. figure:: ../_images/settingsTab.png
    :align: center
    :width: 500 
    :figclass: align-center

    The top-level Settings Tab

Ancillary Tools
---------------

Vapor comes with a Tools menu that provides utilities that can help with visualization and analysis.

Python Engine
_____________

The Python Engine is a tool that allows users to derive new variables based on the data that exist in their files.  Users need to select input variables that will be read in their script, and they will need to define an output variable.  If the script successfully run by the Python Engine, the output variable will be usable in the same way as the native variables are in the dataset.

The modules *numpy* and *vapor_utils* are available for importation in the Python Engine.

Note: Input variables must exist on the same grid to produce a valid output.

.. figure:: ../_images/pythonEditor.png
    :align: center
    :width: 500 
    :figclass: align-center

2D Plots
________

Users can generate two-dimensional line lots of their variables using the Plot Utility.  Line plots can be done either through two points in space at a single timestep, or through a single point across a timespan.

.. figure:: ../_images/plotUtility.png
    :align: center
    :width: 500 
    :figclass: align-center

    The user interface for hte Plot Utility

.. figure:: ../_images/plot.png
    :align: center
    :width: 500 
    :figclass: align-center

    An example of a line plot of Pressure through the spatial domain, at timestep 0

Statistics
__________

Statistical values can help users select meaningful values for renderer color extents, isosurface values, and contour values.  Vapor currently supports calculating the minimum, maximum, mean, median, and mode for variables.  The spatial and temporal extents of the variables being queried are adjustable by the user.

.. figure:: ../_images/statistics.png
    :align: center
    :width: 500 
    :figclass: align-center

