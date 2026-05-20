# %% [md]
#
# # Rendering Numpy data with Vapor
#
# Vapor supports rendering 2D and 3D numpy data.
# In order to pass Numpy data to Vapor, create a data set of the type `vapor.dataset.PYTHON`
# This can also be done with the convenience function `Session.CreatePythonDataset()`.
# You can add numpy arrays as variables to that dataset by using `Dataset.AddNumpyData`.
#
# These variables can then be rendered normally using any of Vapor's renderers.
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera
import numpy as np
from math import sqrt

ses = session.Session()
data = ses.CreatePythonDataset()

# %%

# Create a 2D numpy array and add it to vapor's dataset

np_array = example_utils.SampleFunctionOnRegularGrid(
    lambda x,y: abs(sqrt((x-0.5)**2+(y-0.5)**2)-0.4)<0.02 or y,
    shape=(64,65)
)

data.AddNumpyData("variable_name", np_array)

print(np_array)

# %%

# Create a renderer for the data

ren = data.NewRenderer(renderer.TwoDDataRenderer)
ren.SetVariableName("variable_name")

# %%

# Show the rendering

ses.GetCamera().ViewAll()
ses.Show()
