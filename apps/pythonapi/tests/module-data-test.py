import numpy as np

arr = np.arange(32**3, dtype=np.float32).reshape(32, 32, 32)
print("Type =", arr.dtype)
print(type(arr))
# exit(0)

from vapor import session, renderer, dataset, camera

ses = session.Session()

data = ses.OpenDataset(dataset.PYTHON, "dataset_name")
data.AddNumpyData("data_1", arr)

ren = data.NewRenderer(renderer.WireFrameRenderer)
ren.SetVariableName("data_1")

cam = ses.GetCamera()
# cam.ViewAll()
cam.LookAt((50, 50, 40), (16, 16, 16))
# cam.LookAt((3, 3, 2), (0.5, 0.5, 0.5))
ses.Render("out-data-test-1.png")


arr = np.arange(32**3*2, dtype=np.float32).reshape(32, 32*2, 32)
data.AddNumpyData("data_1", arr)
ren.SetVariableName("data_1")
ses.Render("out-data-test-2.png")
ren.SetEnabled(False)

arr = np.arange(32**2, dtype=np.float32).reshape(32, 32)
data.AddNumpyData("data_2d_1", arr)
ren = data.NewRenderer(renderer.TwoDDataRenderer)
ren.SetVariableName("data_2d_1")
ses.Render("out-data-test-3.png")

import xarray as xr
xrd = xr.open_dataset("/Users/stasj/Work/data/time/time_01.nc")
data.AddXArrayData("XR_Sphere", xrd.sphere)
ren.SetVariableName("XR_Sphere")
cam.ViewAll()
ses.Render("out-data-test-4.png")

def gen2d(w,h,f):
    ay = []
    for y in range(0,h):
        ax = []
        for x in range(0,w):
            ax += [f(x,y)]
        ay += [ax]
    return ay

from math import cos, sin, pi

w = h = 8
curveVar = xr.DataArray(
    np.random.randn(8, 8),
    dims=("x", "y"),
    coords={
        "x_coord": xr.DataArray(gen2d(8,8,lambda x,y: cos(y/h*pi) * (x+(w+1))), dims=("x", "y")),
        "y_coord": xr.DataArray(gen2d(8,8,lambda x,y: sin(y/h*pi) * (x+(w+1))), dims=("x", "y"))
    })


data.AddXArrayData("XR_Curve", curveVar)
ren.SetVariableName("XR_Curve")
cam.ViewAll()
ses.Render("out-data-test-5.png")
