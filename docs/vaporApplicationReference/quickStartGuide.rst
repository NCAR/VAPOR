.. _quickStartGuide:

=================
Quick Start Guide
=================

First, install Vapor from our :ref:`downloads <downloads>` page.  Sample data can be found :ref:`here <sampleData>`.

Launch Vapor
------------

After following the :ref:`installation instructions <installationInstructions>`, launch the application by doing the following:

:OSX:

    Vapor3 will be located within your Applications folder.  Double click on Vapor3's icon.

:Windows:

    Unless you chose a different directory during installation, Vapor3 will exist in C:\\Program Files\\VAPOR\\vapor.exe.  Double click on the executable.

:Linux:

    From a BASH shell, navigate to the directory you installed Vapor3 into.  Then issue the following command:

    ::

        user@localhost:/vaporInstallDir$ bin/vapor

Load Data
---------

There are two ways to get data into Vapor3.

To load data, do one of the following from the ``File`` menu:

1. Load a .vdc file after converting your data into a :ref:`vdc`
    ``File->Open VDC``

2. Import your data, if your datatype is supported.
    ``File->Import->[dataType]``

+-------------------------------------------------+----+--------------------------------------------------+
| Loading a .vdc file                             |    | Importing data                                   |
+-------------------------------------------------+----+--------------------------------------------------+
| .. image :: /_images/loadData.png               | or | .. image :: /_images/importData.png              |
+-------------------------------------------------+----+--------------------------------------------------+

Create a Renderer
-----------------

Now that we've loaded data, we can create a new :ref:`Renderer <Renderers>`.

Vapor3 displays all of your renderers in a table in the upper left corner of the application.  Next to this table are controls that  let you create ``New`` renderers, ``Delete`` renderers, or ``Duplicate`` existing renderers.  

.. figure:: /_images/rendererTable.png
    :width: 500
    :align: center
    :figclass: align-center

    Vapor3's Renderer Table.

Click on ``New``.  This will raise a window that will let you choose from the currently available renderers.  Pick the Slice Renderer by double-clicking on the ``Slice`` button.

.. figure:: /_images/newRenderer.png
    :width: 500
    :align: center
    :figclass: align-center

    Vapor3's ``New Renderer`` Dialog

Notice that your new Slice Renderer has been added to the Renderer Table.  By default, all renderers are disabled after being created.  To enable your Slice renderer, click the ``Enabled`` checkbox in the Renderer Table that's in the same row as your new Slice.

Now that you have your first Renderer, you can do the following:

1. Change the displayed variable in the :ref:`Variables Tab <variablesTab>`
2. Change the color mapping of your variable in the :ref:`Appearance Tab <appearanceTab>`
3. Modify the orientation and region that your renderer is drawn to in the :ref:`Geometry Tab <geometryTab>`
4. Add annotations and color bars in the :ref:`Annotations Tab <annotationTab>`

At this point, we've created our first renderer.  To customize it, we need to get familiar with the four tabs listed above.  The :ref:`Variables Tab <variablesTab>` and :ref:`Appearance Tab <appearanceTab>` are the most important to get started with.  We also encourage you to watch active demonstrations in our `YouTube channel <https://www.youtube.com/channel/UCpf-d1GDO1sotzjJ2t_QkDw>`_.
If you have any questions, bug reports, or feature requests, see our `forum <https://vapor.discourse.group/>`_ section.  Thank you. 
