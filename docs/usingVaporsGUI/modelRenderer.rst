3D Models
_________

The Model Renderer can display 3D geometry files within the Vapor scene.  This is commonly used for rendering models of wind turbines or cityscapes in Large Eddy Simulations.

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
