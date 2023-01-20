.. _flowRenderer:

Flow
____

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/Zyz30rYD8rU" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

|

Description
-----------

The Flow Renderer creates either *Streamlines* or *Pathlines* within a user's data domain.  *Streamlines* are time-invariant trajectories that follow the user's defined field variables.  *Pathlines* are time-variant, following trajectories of the user's field variables as time progresses.

Basic Controls
--------------

The Flow Renderer uses Vapor's standard controls located in the :ref:`Variables <variablesTab>`, :ref:`Appearance <appearanceTab>`, :ref:`Geometry <geometryTab>`, and :ref:`Annotation <annotationTab>` tabs.  In addition, the Flow renderer contains a Seeding tab, described below.

Specialized Controls
--------------------

The Flow Renderer Seeding Tab contains four sets of parameters as follows:

Integration Settings
********************


**Flow type:** Specifies whether the Flow Renderer is advecting *Streamlines* or *Pathlines*.  

*Streamlines* draw trajectories along a vector field at a single timestep.  These trajectories are time-invariant, and are best used to analyze vector fields at a single time step.

*Pathlines* draw trajectories along a vector field as it changes in time.  If you were to throw a feather into a tornado, the path of the feather would be analogous to a Pathline.

Streamline Settings
```````````````````

**Flow direction:** Determines whether the advection of the *Streamlines* will go forward along the vector field, backwards against it, or bidirectionally.

**Integration steps:** Specifies the number of integration steps to advect the *Streamline* along.

Pathline Settings
`````````````````

**Pathline length:** Specifies the length of each individual pathline to be shown in the unit of timesteps. If set to 1, the trajectory of a particle travels from the previous timestep (t-1) to the current timestep (t) will be displayed. If set to 2, the trajectory of a particle travels from the past two timesteps (t-2 and t-1) to the current timestep (t) will be displayed.

**Injection Interval:** This controls the frequency at which pathlines are injected into the scene.  A value of 0 will only inject pathlines at the initial time step of the data set.  A value of 1 will inject at every timestep, 2 will inject at every 2nd timestep, etc.

Both *Streamline* and *Pathline* integration contain the following parameters:

**Vector field multiplier:** Allows the user to multiply their vector field values by a scalar.  This is useful in the situation where the data domain and the vector field are in different units.  For example, if the data domain is in units of km, and the vector field is in units of m/s, the user may multiply their vector field by 0.001 so that all units are congruent with each other.

**Axis Periodicity:** If the X, Y, or Z axes are periodic, flow lines that exit the domain along these axes will be re-inserted on the opposite side of the domain.  Flow lines will continue advecting from that point.

Seed Distribution Settings
**************************

**Seed distribution type:** This parameter determines the way that the Flow Renderer places seeds within the scene.  The options are:

*Gridded:* Users can specify a grid of seeds that are distributed on the X, Y, and Z axes.  The Z axis seed option is removed when rendering 2D flow.

*Random:* Users select a number of seeds that will be randomly placed within the Flow Renderer's Rake region.

*Random w/ bias:* Like a Random distribution, except that users also specify a bias variable that influences the random distribution.  If the bias is positive, the seed distribution will prefer regions where there are greater values of the selected variable.  If the bias is negative, the distribution will prefer regions where the variable is lesser in value. 

*List of seeds:* A user may specify a list of seeds to be read from the renderer.  The file must contain lines of comma separated values that represent the locations of the seeds on the X, Y, and Z axes.  Users may optionally add a time value after the Z coordinate.  Empty lines are ignored, and lines beginning with the # character are comments.

The following example would place a seed at spatial location of (.5, .8, .25) 
with timestamp .5.::

    # X, Y, Z, T
    .5, .8, .25, 0.5 


Rake Region
***********

If the Flow Renderer is using a Gridded, Random, or Random w/ Bias seed distribution, users may constrain the region of seed injection with the Flow Rake.  By default, the Rake is as large as the entire domain.  If there is a specific region of interest, users should constrain the Rake to only distribute seeds within that region.

The Rake can be adjusted through the left-hand control panel by either moving the sliders corresponding to the desired axis, or typing explicit values.  Alternatively, the current Rake can be rendered and manipulated within the scene.  This is done by clicking on the Navigation drop-down menu at the top left of the application, and selecting "Region".  Users must be in the Seeding tab with the Region mode activated to show the Flow Rake within the scene.

Write Flowlines to File
***********************

Users may write the geometry of the currently rendered flow lines by selecting a text file, and clicking Write to file.  The data format of the file is a CSV containing values as follows::

    # ID,   X-position,    Y-position,    Z-position,    Time,   Value+

+Value is the value of the currently selected color-mapped variable.
