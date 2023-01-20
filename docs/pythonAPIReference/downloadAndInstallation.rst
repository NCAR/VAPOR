Download and Installation
-------------------------

The easiest way to download and install VAPOR Python is using conda, which can be downloaded at `Anaconda.org <https://anaconda.org/>`_.

It is recommended to start with a new conda environment. In the example below we create a new environment named vapor_python.

.. code-block:: console

    conda create -n vapor_python

Activate the environment

.. code-block:: console

    conda activate vapor_python

Install the package

.. code-block:: console

    conda install -c conda-forge -c ncar-vapor vapor

Now run examples in the following directories like so:

Jupyter Notebooks:

.. code-block:: console

    cd $CONDA_PREFIX/lib/python3.9/site-packages/vapor/example_notebooks/
    jupyter notebook

Python Scripts:

.. code-block:: console

    $CONDA_PREFIX/lib/python3.9/site-packages/vapor/examples/
    ./workflow_example.py
