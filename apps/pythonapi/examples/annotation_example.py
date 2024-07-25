# %% [md]
#
# # Annotations
#
# You can annotate your renderings using Vapor's build in annotations as well as MatPlotLib
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera
from vapor.utils import histogram

ses = session.Session()
data = example_utils.OpenExampleDataset(ses)

ren = data.NewRenderer(renderer.TwoDDataRenderer)
ren.SetVariableName(data.GetDataVarNames(2)[1])
ses.GetCamera().ViewAll()

# %% [md]
#
# ## Vapor Colorbars
#
# %%
# Show the basic colorbar annotation for a renderer
colorbar = ren.GetColorbarAnnotation()
colorbar.SetEnabled(True)
ses.Show()

# %%
# Customized colorbar annotation
colorbar.SetSize((0.2, 0.25))
colorbar.SetTitle("Colorbar Title")
colorbar.SetCornerPosition((0.95, 0.95))
ses.Show()

# %% [md]
#
# ## MatPlotLib Colorbars
#
# You can customize the histograms as you would MatPlotLib's `pylab.colorbar`
#
# %%
tf = ren.GetPrimaryTransferFunction()
tf.ShowMatPlotLibColorbar()

# %%
tf.ShowMatPlotLibColorbar(orientation="vertical", figsize=(1.5,4))

# %%
# The MatPlotLib Colorbars are automatically synced to Vapor's transfer functions
tf.LoadBuiltinColormap(tf.ListBuiltinColormaps()[7])
ses.Show()
tf.ShowMatPlotLibColorbar()

# %%
colorbar.SetEnabled(False)

# %% [md]
#
# ## MatPlotLib Histograms
#
# %%
histogram.ShowMatPlotLibHistogram(ses, ren)

# %% [md]
# ---
# You can customize the histograms as you would `matplotlib.pyplot.hist`
#
# %%
plt = histogram.GetMatPlotLibHistogram(ses, ren, color ="red")
plt.xlabel("X-Axis")
plt.ylabel("Y-Axis")
plt.title("Title\n", fontweight="bold")
plt.show()

# %% [md]
#
# ## Axis Annotations
#
# %%
axis = ses.GetAxisAnnotations()
axis.SetAxisAnnotationEnabled(True)
ses.Show()

# %%
axis.SetNumTics((5,5))
axis.SetAxisFontSize(24)
ses.Show()

# This example dataset is not geo-referenced
# axis.SetLatLonAxesEnabled(True)

# %%
axis.SetAxisAnnotationEnabled(False)

# %% [md]
#
# ## Scene Annotations
#
# Scene annotations are other annotations and rendering options that apply to the entire rendering such as background color.
#
# %%
scene = ses.GetSceneAnnotations()
scene.SetBackgroundColor((1,1,1))
scene.SetUseDomainFrame(False)
ses.Show()

# %%
scene.SetAxisArrowEnabled(True)
scene.SetAxisArrowSize(0.4)

scene.SetTimeType(scene.TimeAnnotationType.Timestep)
scene.SetTimeColor((0,0,0))
scene.SetTimeLLX(0.7)

ses.Show()