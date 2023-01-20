# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

def fetchPythonAPIExamples():
    # Fetch python API examples
    subprocess.call("git checkout origin/main -- ../apps/pythonapi/examples", shell=True)
    subprocess.call("mv ../apps/pythonapi/examples pythonAPIReference", shell=True)

    # Get all .py files in /examples
    files = os.listdir("pythonAPIReference/examples")
    files = [ fi for fi in files if fi.endswith(".py") ]

    # Generate python api examples in pythonAPIReference/examples.rst
    exampleFile = open("pythonAPIReference/examples.rst", "w+")
    exampleFile.writelines(["Examples\n________\n\n"])
    exampleFile.writelines(["Once you've installed VAPOR's Python API, you can find the examples below in the following directory:\n\n"])
    exampleFile.writelines([".. code-block:: console\n\n"])
    exampleFile.writelines(["    $CONDA_PREFIX/lib/python3.9/site-packages/vapor/examples\n\n"])
    exampleFile.writelines([".. toctree::\n\n"])
    for fi in files:
        rstFileName = "examples/" + fi[0:-3] + ".rst"
        exampleFile.writelines(["   " + rstFileName + "\n"])
        rstFile = open("pythonAPIReference/" + rstFileName, "w+")
        rstFile.writelines([fi + "\n"])
        rstFile.writelines(["-" * len(fi) + "\n\n"])
        rstFile.writelines([".. literalinclude:: " + fi + "\n"])
        rstFile.close()
    exampleFile.close()

#def generatePythonAPIClassReference():
#    import vapor
#    classReferenceFileName = "pythonAPIReference/classReference.rst"
#    classReferenceFile = open(classReferenceFileName, "w")
#    classReferenceFile.writelines([".. _classReference:\n\n"])
#    classReferenceFile.writelines(["Class Reference\n"])
#    classReferenceFile.writelines(["_______________\n\n"])
#    classReferenceFile.writelines([".. toctree::\n"])
#    classReferenceFile.writelines(["   :maxdepth: 1\n\n"])
#
#    classReferenceFiles = "pythonAPIReference/classReferenceFiles"
#    if (os.path.isdir(classReferenceFiles)):
#        import shutil
#        shutil.rmtree(classReferenceFiles)
#    os.mkdir(classReferenceFiles)
#
#    package=vapor
#    for importer, modName, ispkg in pkgutil.walk_packages(path=package.__path__,
#                                                          prefix=package.__name__+'.',
#                                                          onerror=lambda x: None):
#        mod = __import__(modName, fromlist=["vapor"])
#        classReferenceFile.writelines(["   classReferenceFiles//" + modName + ".rst\n"])
#
#        modDir = classReferenceFiles + "//" + modName + "//"
#        os.mkdir(modDir)
#
#        modFileName = modDir[0:-2] + ".rst"
#        modFile = open(modFileName, "w")
#        modFile.writelines([".. _" + modName + ":\n\n"])
#        modFile.writelines([modName + "\n"])
#        modFile.writelines([str("-" * len(modName)) + "\n\n"])
#        modFile.writelines([".. toctree::\n"])
#        modFile.writelines(["   :maxdepth: 1\n\n"])
#
#        classes = dict([(name, cls) for name, cls in mod.__dict__.items() if isinstance(cls, type)])
#
#        for myClass in classes:
#            className = modName + "." + myClass
#            out = sys.stdout
#            classFileName = modDir + className + ".rst"
#
#            # Add this class's .rst file to moduleFile toctree
#            modFile.writelines(["   " + modName + "//" + className + ".rst\n"])
#
#            # write file through stdout, since help() outputs through stdout
#            sys.stdout = open(classFileName, "w")
#            #print(":orphan:")
#            print(".. _" + className + ":")
#            print("\n")
#            print(className)
#            print("-" * len(className))
#            print("\n")
#            help(className)
#            sys.stdout.close()
#            sys.stdout = out

import os
import sys
import sphinx_rtd_theme
import subprocess
import pkgutil

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

# Add vapor_utils to path and vapor_wrf modules for python engine documentation
sys.path.insert(0, os.path.abspath('vaporApplicationReference/otherTools'))

# -- Project information -----------------------------------------------------

project = ' '
copyright = '2023 University Corporation for Atmospheric Research'
author = ''

# The short X.Y version
version = ''
# The full version, including alpha/beta/rc tags
release = '3.8.0'


#breathe_projects = { "myproject": "/Users/pearse/vapor2/targets/common/doc/library/xml" }
#breathe_default_project = "myproject"

extensions = [
    'sphinx.ext.imgmath', 
    'sphinx.ext.todo', 
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx_gallery.gen_gallery',
    #'jupyter_sphinx.execute'
    #'breathe'
    #'wheel'
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

language = 'en'

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = None


html_logo = "_images/vaporLogoBlack.png"
html_favicon = "_images/vaporVLogo.png"

html_theme = "sphinx_book_theme"
html_theme_options = dict(
    # analytics_id=''  this is configured in rtfd.io
    # canonical_url="",
    repository_url="https://github.com/NCAR/VAPOR",
    repository_branch="main",
    path_to_docs="doc",
    use_edit_page_button=True,
    use_repository_button=True,
    use_issues_button=True,
    home_page_in_toc=False,
    extra_navbar="",
    navbar_footer_text="",
    extra_footer=""
)

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']


# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'Vapordoc'

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'Vapor.tex', 'Vapor Documentation',
     'John Clyne, Scott Pearse, Samuel Li, Stanislaw Jaroszynski', 'manual'),
]


# -- Options for manual page output ------------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'vapor', 'Vapor Documentation',
     [author], 1)
]


# -- Options for Texinfo output ----------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'Vapor', 'Vapor Documentation',
     author, 'Vapor', 'One line description of project.',
     'Miscellaneous'),
]

# -- Options for Epub output -------------------------------------------------

# Bibliographic Dublin Core info.
epub_title = project

###
### Configure Sphinx-gallery
###

# Download example data 
import requests
from pathlib import Path
home = str(Path.home())
simpleNC = home + "/simple.nc"
dataFile = "https://github.com/NCAR/VAPOR-Data/raw/main/netCDF/simple.nc"
response = requests.get(dataFile, stream=True)
with open(simpleNC, "wb") as file:
  for chunk in response.iter_content(chunk_size=1024):
    if chunk:
      file.write(chunk)

# Set plotly renderer to capture _repr_html_ for sphinx-gallery
try:
    import plotly.io as pio
    pio.renderers.default = 'sphinx_gallery'
except ImportError:
    pass

# suppress warnings
import warnings

# filter Matplotlib 'agg' warnings
warnings.filterwarnings("ignore",
                        category=UserWarning,
                        message='Matplotlib is currently using agg, which is a'
                        ' non-GUI backend, so cannot show the figure.')

# filter seaborn warnings
warnings.filterwarnings("ignore",
                        category=UserWarning,
                        message='As seaborn no longer sets a default style on'
                        ' import, the seaborn.apionly module is'
                        ' deprecated. It will be removed in a future'
                        ' version.')

# Configure sphinx-gallery plugin
from sphinx_gallery.sorting import ExampleTitleSortKey

sphinx_gallery_conf = {
    'examples_dirs': ['dataFormatRequirements/netCDF', 'vaporApplicationReference/imageRenderer'],  # path to your example scripts
    'gallery_dirs': ['dataFormatRequirements/netCDF/examples', 'vaporApplicationReference/imageRenderer'],  # path to where to save gallery generated output
    #'examples_dirs': ['data/netCDF', 'vaporApplicationReference/imageRenderer', 'pythonAPIReference/examples'],  # path to your example scripts
    #'gallery_dirs': ['data/netCDF/examples', 'vaporApplicationReference/imageRenderer', 'pythonAPIReference/examples'],  # path to where to save gallery generated output
    'within_subsection_order': ExampleTitleSortKey,
    'matplotlib_animations': True,
}

#fetchPythonAPIExamples()
#generatePythonAPIClassReference()

'''print("*******************")
print("*******************")
print("*******************")
print("*******************")
print("*******************")
subprocess.call("pwd", shell=True)
subprocess.call("which python", shell=True)
subprocess.call("python --version", shell=True)
subprocess.call("echo prefix: ${CONDA_PREFIX}", shell=True)
subprocess.call("ls ${CONDA_PREFIX}/include", shell=True)
subprocess.call("ls /include", shell=True)'''

'''test = r'CONDA_PREFIX=/home/docs/checkouts/readthedocs.org/user_builds/vapor/conda/pythonhelp python -c "exec(\"import vapor\")"'
print(test)
subprocess.call(test, shell=True)'''

'''#subprocess.call("python -c \"exec\("import vapor\n\"\)\"", shell=True)
subprocess.call("/home/docs/checkouts/readthedocs.org/user_builds/vapor/conda/pythonhelp/bin/python makePythonClassReference.py", shell=True)
subprocess.call("/home/docs/checkouts/readthedocs.org/user_builds/vapor/conda/pythonhelp/bin/python docs/makePythonClassReference.py", shell=True)'''
