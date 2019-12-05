.. _controllingYourRenderers:

Common Renderer Controls
------------------------

Each of Vapor's renderers has a common set of controls that will create imagery of your variables according to color, opacity, and region of interest.

While each renderer is unique, controlling their parameters is mostly the same.  All renderers are controlled by four tabs:

    - :ref:`Variables <variablesTab>`
    - :ref:`Appearance <appearanceTab>`
    - :ref:`Geometry <geometryTab>`
    - :ref:`Annotation <annotationTab>`

See the :ref:`Renderers <renderers>` section for more information on how each of these tabs work for a given renderer.  Again, they all operate in the same way for the most part.

.. figure:: ../../_images/variablesTab.png
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

.. figure:: ../../_images/transferFunctionDocumentation.png
    :align: center
    :figclass: align-center

    Vapor 3's Transfer Function editor

In the figure above, we can see that our transfer function is operating on the variable P.  The range of values within the transfer function are -1314.76 to 1268.32.  All values of P less than 1314.76 are colored deep blue.  The coloration transitions into red at the high end of the PDF, until becoming saturated at values of 1268.32 and higher.

Below the histogram is a button to update the histogram, which is calculated only when the user requests it to save on compute time.  Options to change the color interpolation type are also available.

.. figure:: ../../_images/TFOptions.png
    :align: center
    :width: 500
    :figclass: align-center

    Additional options for the Transfer Function

.. _controllingColor:

Controlling Color
"""""""""""""""""

Vapor's default color map is called CoolWarm.  This is arbitrary, and may not suit your needs.  Vapor bundles several other color maps that can be found by pressing the "Load TF" button at the top of the Appearance tab.

The colors in the color map be moved by creating a color-control-point, and dragging it.  To create a new color-control-point, right click on the Colorbar, and then click "New Color Control Point."  The color at this control point may now be dragged to suit your needs.

.. figure:: ../../_images/colorControlPoint.gif
    :align: center
    :figclass: align-center

    Adding and moving color control points in the Colorbar

These control points may also be given direct color values by either double clicking them, or right-clicking and selecting "Edit color control point".  After a color has been changed, Vapor will interpolate between control points to give a smooth color transition.

Controlling Opacity
"""""""""""""""""""

Opacity is controlled by the green line on top of the PDF.  The higher this green line is on the PDF's Y axis, the more opaque the colors will be at that point.  For example, the green bar is set to Y=0 over the blue values in the image below.  All of these values will be masked out.  The green bar then ramps up, and the values become more opaque, until we reach full opacity in the red region.

.. figure:: ../../_images/opacityMap.png
    :align: center
    :width: 500 
    :figclass: align-center

    Blue values are hiden completely.  White values ramp up from transparent to opaque, and red values are fully opaque.

.. _geometryTab:

Geometry Tab
____________

The Geometry tab controls where your renderer is drawing, within the space of your simulation.  By excluding regions of data from being drawn, occluded features may be seen more clearly. Compute time will also be reduced, as well as the memory needed for a given renderer.

.. figure:: ../../_images/geometryWidget.png
    :align: center
    :width: 500
    :figclass: align-center

    Coordinate selector in the Geometry Tab
    
If you have a region of interest in another renderer, that region can be copied in the Geometry tab.

.. figure:: ../../_images/copyRegionWidget.png
    :align: center
    :width: 500 
    :figclass: align-center

    Copy geometry from one renderer to another


Users can apply transforms to scale, translate, and rotate their renderers on X, Y, or Z.  The origin used for these transforms may also be adjusted.

.. figure:: ../../_images/transformTable.png
    :align: center
    :width: 500 
    :figclass: align-center

    Transformation options within the Geometry widget

.. _regionMouseMode:

Users may also control the geometry of their renderer by using the `Region Mouse Mode`, located at the top left corner of the application.  This will enable a red box with handlebars that can be right-clicked to grow or shrink the region being rendered on any axis.

.. figure:: ../../_images/selectRegionMouseMode.png
    :align: center
    :width: 400 
    :figclass: align-center

    Select the Region Mouse Mode for interactive geometry adjustment

.. figure:: ../../_images/regionMouseMode.png
    :align: center
    :width: 500 
    :figclass: align-center

    Interactive geometry controls alongside a Barb renderer, after activating the Region Mouse Mode

.. _annotationTab:

Annotation Tab
______________

Quantifying the colors to your viewers can be done by adding a colorbar in the Annotation tab.

.. figure:: ../../_images/colorbarTab.png
    :align: center
    :width: 500 
    :figclass: align-center

    Colorbar size and position controlls, located in the Annotation tab

.. figure:: ../../_images/colorbar.png
    :align: center
    :figclass: align-center

    An exmaple colorbar

|

Specialized Renderer Controls
-----------------------------

Some renderers have unique controls that do not exist elsewhere.

Raycasting Renderers
____________________

The Volume and Isosurface renderers perform a method called raycasting, where a line is drawn from each pixel on the screen into the loaded dataset.  Each line samples data values and returns a color, according to what's been configured in the Transfer Function.  Additional appearance settings for raycasting renderers are naturally found in the Appearance tab.

.. figure:: ../../_images/raycastingOptions.png
    :align: center
    :width: 500 
    :figclass: align-center

    Raycasting options in the Volume and Isosurface renderers, in the Appearance tab.

Isosurface Color Controls
_________________________

The Isosurface Appearance tab is unique in that it allows the user to select up to four values to draw isosurfaces with.

.. figure:: ../../_images/isovalueSelector.png
    :align: center
    :width: 500 
    :figclass: align-center

Isosurfaces can be colored by a secondary variable, and therefore have a Transfer Function that is split into an isovalue selector, and a color mapping.  When the user disables the "Use Constant Color" checkbox, the Color Mapped Variable tab will be enabled.

.. figure:: ../../_images/isosurfaceTF.png
    :align: center
    :width: 750 
    :figclass: align-center

    The isosurface Transfer Funciton, displaying the current isovalue in the variable PDF, and the Transfer Function for the isosurface's Color Mapped Variable.

Height Variable Offsets
_______________________

The TwoDData, Barb, Contour, Image, and Wireframe renderers all have the option to offset the data by a height variable.

.. figure:: ../../_images/contourHeightSelection.png
    :align: center
    :width: 500 
    :figclass: align-center

    Selecting a height variable in the Contour renderer

.. figure:: ../../_images/contourWithHeight.png
    :align: center
    :width: 500 
    :figclass: align-center

    The resultant contour plot, offset by the height variable HGT

Slice Quality Control
_____________________

The Slice renderer samples variable data at a fixed rate across a two dimensional plane.  Usually the default sampling rate is sufficient, but it can be increased with the Quality controller in the Appearance tab.

.. figure:: ../../_images/sliceQualityAdjustment.png
    :align: center
    :width: 500 
    :figclass: align-center

    Quality adjustment for the Slice renderer

Contour Selection
_________________

The Appearance tab for the Contour renderer includes controls for the contour count, spacing between contours, minimum contour value, and the width of the contours being rendered.

.. figure:: ../../_images/contourAppearance.png
    :align: center
    :width: 500 
    :figclass: align-center

    Controls for the Contour renderer


Barbs Selection
_______________

The Barb renderer operates on a set of vectors to determine which direction they point in.  Users need to select variables to correspond with the X, Y, and optionally Z vectors in their dataset.

Users may also offset the barbs by a height variable, and color them according to an additional variable if desired.  For example, users may have wind barbs being drawn based on their U, V, and W variables, and colored by their Pressure variable.

.. figure:: ../../_images/barbVariableSelector.png
    :align: center
    :width: 500 
    :figclass: align-center

    Variable selector for the Barb renderer

Georeferenced Images
____________________

The Image renderer is the only one that does not have a Transfer Function.  All the user needs to do is select either one of Vapor's bundled GeoTiff images, or one that they have made themselves.

The Image renderer is also the only renderer than can extend beyond the domain of the user's data.  This can be done by switching to the :ref:`Region Mouse Mode <regionMouseMode>`.

.. figure:: ../../_images/imagePastDomain.png
    :align: center
    :width: 500 
    :figclass: align-center

    An Image renderer that is drawing outside the data domain, using the Region Mouse Mode

Model Renderer
______________

The model renderer is used to import and display 3D models alongside your data. It supports most common 3D model files (a full list can be found at github.com/assimp/assimp). It can also display more complex 3D scenes that can be animated with your data timesteps by using .vms files. 

Importing a 3D Model 
""""""""""""""""""""

To import a 3D model file, create a model renderer and use the “Select” button next to the 3D Model/Scene dialog. Enable the renderer to view it. If you cannot see anything, make sure your 3D model is an appropriate size for your dataset. You can move/rotate/scale the model under the “Geometry” tab.

Importing a 3D Scene
""""""""""""""""""""

Importing a 3D scene follows the same process as importing a 3D model. See below for documentation on creating a 3D Scene.

Creating a 3D Scene 
"""""""""""""""""""

3D scenes are stored in .vms files containing an XML description of a 3D scene. 

.vms XML layout 
"""""""""""""""

A .vms file must have a top-level node called ``scene``. Everything else is a child of this node. 

Instances 
"""""""""

Every model displayed in the scene is called an ``instance``. The associate tag is called ``instance_<name>``. Any time you add an ``instance_<name>`` tag with a new <name>, you create a new instance. When creating a new instance, you need to specify the file from which to load the 3D model data for that instance. This is done with the attribute ``file`` within the instance tag. You can optionally transform the instance by adding child Transformation nodes. While instances can be created outside of a timestep, they will only be displayed if they are referenced inside a timestep.

Time
""""

The ``time_<timestep #>`` tag represents a timestep. The ``<timestep #>`` is an integer representing the timestep as it appears in the vapor timestep selector. Time nodes can contain instances which will be displayed during the referenced timestep. When rendering, the most recent valid timestep is rendered. For example, if you want to have the same scene displayed for every timestep, create a single ``time_0`` tag and create your instances inside of it. If you want to stop rendering after a certain timestep, <N>, create an empty tag ``time_N``.

Transformations
"""""""""""""""

There are 4 transformation tags: ``translate``, ``rotate``, ``scale``, and ``origin``. Each tag results in the same transformation as Vapor’s renderer/dataset transform settings. Each transformation tag has three possible attributes: ``x``, ``y``, and ``z``. The rotate ``x``, ``y``, and ``z`` values rotate round the corresponding axis by the value given in degrees. If an origin is specified when creating a new instance, that origin will be used for all subsequent transformations of that instance unless otherwise specified.


.vms File Example
"""""""""""""""""

.. code-block:: xml

    <scene>
    <instance_blade file="turbine-blade.stl">
        <origin z="90" />
    </instance_blade>
    <instance_tower file="turbine-tower.stl" />
        <time_0>
            <instance_blade1>
                <translate x="420" y="300" z="0" />
            </instance_blade1>
            <instance_tower>
                <translate x="420" y="300" z="0" />
            </instance_tower>
    </time_0>
    <time_1>
        <instance_blade>
            <translate x="420" y="300" z="0" />
            <rotate x="-72.6" />
        </instance_blade>
        <instance_tower>
            <translate x="420" y="300" z="0" />
        </instance_tower>
    </time_1>
    </scene>
