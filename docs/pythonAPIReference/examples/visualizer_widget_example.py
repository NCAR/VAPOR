# %% [md]
#
# # Visualizer Widgets
#
# Visualizer widgets allow you to interactively explore a session as you would in a Vapor GUI visualizer.
# This notebook shows how to use visualizer widgets and how to add additional dynamic parameter inputs.
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera

ses = session.Session()
data = example_utils.OpenExampleDataset(ses)

# %% [md]
#
# ## Render an Iso Surface
#
# %%
ren = data.NewRenderer(renderer.VolumeIsoRenderer)
ren.SetVariableName(data.GetDataVarNames(3)[0]) # Set to first 2D data variable
ren.SetIsoValues([ren.GetIsoValues()[0]+0.1])

ses.GetCamera().ViewAll()
ses.Show()

# %% [md]
#
# ## Create a visualizer to explore the scene
#
# Try dragging the image to rotate the view.
# Hover over the visualizer to see the full controls.
#
# %%
from jupyter_vapor_widget import *

viz = VaporVisualizerWidget(ses)
viz

# %% [md]
#
# ## Add an interactive iso value slider using **ipywidgets**
#
# %%
tf = ren.GetPrimaryTransferFunction()
dataRange = tf.GetMinMaxMapValue()

def sliderChanged(change):
    ren.SetIsoValues([change.new])
    viz.Render(fast=True)

slider = widgets.FloatSlider(value=ren.GetIsoValues()[0], min=dataRange[0], max=dataRange[1], step=(dataRange[1]-dataRange[0])/100)
slider.observe(sliderChanged, names='value')

widgets.VBox([
    viz,
    widgets.HBox([widgets.Label("Iso value:"), slider])
])
