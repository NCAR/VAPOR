# %% [md]
#
# # Opening Datasets
#
# Vapor supports a variety of scientific data formats.
# This notebook shows how to open a dataset and query its metadata.
#
# %%
import example_utils
from vapor import session, renderer, dataset, camera

# %%
print("Supported dataset types:", dataset.Dataset.GetDatasetTypes())

# %%
ses = session.Session()
data = example_utils.OpenExampleDataset(ses)

# Examples of opening real data
#
# data = ses.OpenDataset(dataset.WRF, ["data/wrf_out.0001", "data/wrf_out.0002"])
# data = ses.OpenDataset(dataset.VDC, ["master.vdc"])
# data = ses.OpenDataset(dataset.MPAS, ["x1.static.nc", "diag.2021-03-04_10.30.00.nc"])

# %% [md]
#
# ## Dump the dataset metadata
#
# %%
print("Time Coordinate Variable Name:", data.GetTimeCoordVarName())
print("Coordinate Variable Names:", data.GetCoordVarNames())

print("Dimensions:")
for dim in data.GetDimensionNames():
    print(f"  {dim}:", data.GetDimensionLength(dim, 0))

print("Data Variables:")
for var in data.GetDataVarNames():
    print(f"  {var}")
    print(f"    Time Varying:", bool(data.IsTimeVarying(var)))
    print(f"    Dimensionality:", data.GetVarGeometryDim(var))
    print(f"    Coordinates:", data.GetVarCoordVars(var, True))
    print("     Data Range:", data.GetDataRange(var))

# %% [md]
#
# ## Render the first 2D variable as a wireframe
#
# %%
ren = data.NewRenderer(renderer.WireFrameRenderer)
ren.SetVariableName(data.GetDataVarNames(2)[0]) # Set to first 2D data variable

ses.GetCamera().ViewAll()
ses.Show()
