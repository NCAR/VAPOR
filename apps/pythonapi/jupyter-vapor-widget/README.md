# jupyter-vapor-widget

Jupyter Visualizer Widgets for Vapor API

## Installation

To install use pip:

    $ pip install jupyter_vapor_widget

For a development installation (requires [Node.js](https://nodejs.org) and [Yarn version 1](https://classic.yarnpkg.com/)),

    $ git clone https://github.com/NCAR/jupyter-vapor-widget.git
    $ cd jupyter-vapor-widget
    $ pip install -e .
    $ jupyter nbextension install --py --symlink --overwrite --sys-prefix jupyter_vapor_widget
    $ jupyter nbextension enable --py --sys-prefix jupyter_vapor_widget

When actively developing your extension for JupyterLab, run the command:

    $ jupyter labextension develop --overwrite jupyter_vapor_widget

Then you need to rebuild the JS when you make a code change:

    $ cd js
    $ yarn run build

You then need to refresh the JupyterLab page when your javascript changes.
