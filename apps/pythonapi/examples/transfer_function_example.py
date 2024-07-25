# %% [md]
#
# # Transfer Functions
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera, transferfunction
from vapor.utils import histogram

ses = session.Session()
data = example_utils.OpenExampleDataset(ses)

ren = data.NewRenderer(renderer.VolumeRenderer)
ses.GetCamera().LookAt((32, 120, 120), (32, 32, 32))
ses.Show()

# %% [md]
#
# We created a volume rendering however it is fully opaque.
# We can use a transfer function to adjust the visible portions.
# Before we adjust the opacity map of the TF, we get a histogram to help us determine what we want to hide.
#
# %%
histogram.ShowMatPlotLibHistogram(ses, ren)

# %% [md]
#
# Usually we want to hide the most common value so below we construct an opacity map that accomplishes this.
#
# %%
# List of x,y pairs where x is the data value and y is the opacity for that data value
opacities = [(-0.3, 1), (-0.1, 0), (0.1, 0), (0.3, 1)]

# %% [md]
#
# We can get the matplotlib histogram plot and add our opacity map to it to compare.
#
# %%
plt = histogram.GetMatPlotLibHistogram(ses, ren)
plt.plot(*zip(*opacities))
plt.show()

# %% [md]
#
# Now we apply the map to the transfer function
#
# %%
# Renderers can have multiple transfer functions.
# GetPrimaryTransferFunction returns the one that is usually the most useful.
# You can use `tf.GetTransferFunction(var_name)` to get other transfer functions.
tf = ren.GetPrimaryTransferFunction()
tf.SetOpacityControlPoints(opacities)
ses.Show()

# %% [md]
#
# You can adjust the colormap in a similar fashion. Use `help(tf)` for more information.
# Vapor includes a list of built-in colormaps and these can be applied with `tf.LoadBuiltinColormap(name)`
#
# ## Builtin Colormaps
#
# %%
tf.LoadBuiltinColormap("Sequential/BlackBodyExtended")
ses.Show()

# %% [md]
#
# ## List of All Builtin Colormaps
#
# %%

ses.DeleteRenderer(ren)
ren = data.NewRenderer(renderer.TwoDDataRenderer)
tf = ren.GetPrimaryTransferFunction()

for cmap in transferfunction.TransferFunction.ListBuiltinColormaps():
    tf.LoadBuiltinColormap(cmap)

    print(cmap)
    tf.ShowMatPlotLibColorbar()