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

ses = session.Session()
data = ses.CreatePythonDataset()

# %%

# Create a 2D numpy array and add it to vapor's dataset

np_array = np.arange(64**2).reshape((64,64))
data.AddNumpyData("variable_name", np_array)

print(np_array)

# %%

# Create a renderer for the data

ren = data.NewRenderer(renderer.WireFrameRenderer)
ren.SetVariableName("variable_name")

# %%

# Show the rendering

ses.GetCamera().ViewAll()
ses.Show()
