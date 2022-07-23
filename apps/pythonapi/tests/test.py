# jupytext --to notebook test.py
# jupyter nbconvert --to notebook --inplace --execute test.ipynb
# 
# cat test.py | jupytext --to ipynb | jupyter nbconvert --stdin --execute --no-input --to html --output notebook.html
# jupyter nbconvert --to html --no-input --execute annotation_example.ipynb
# jupyter nbconvert --to pdf --execute annotation_example.ipynb

import numpy as np
from math import *
from vapor import session, renderer, dataset, camera, transferfunction, utils

# help(renderer.VolumeRenderer)

ses = session.Session()
data = ses.CreatePythonDataset()

from inspect import signature
def sample(f, ext=None, shape=None):
    if not shape: shape = [64]*len(signature(f).parameters)
    if not ext: ext = [(0,1)]*len(signature(f).parameters)
    assert len(signature(f).parameters) == len(shape) and len(shape) == len(ext)
    d = []
    for i in np.ndindex(*reversed(shape)):
        d.append(f(*[v/s*(t-f)+f for v,s,(f,t) in zip(reversed(i), shape, ext)]))
    return np.asarray(d, dtype=np.float32).reshape(shape)


# arr = sample(lambda x,y: sin(6*x)+sin(6*y))
arr = sample(lambda x,y: (x+y)*np.exp(-5.0*(x**2+y**2)), ext=[(-1,1)]*2)
data.AddNumpyData("data_2d_1", arr)
ren = data.NewRenderer(renderer.TwoDDataRenderer)
ren.SetVariableName("data_2d_1")
ren.GetColorbarAnnotation().SetEnabled(True)

# utils.ShowHistogram(ses, ren)
# utils.ShowHistogram(ses, ren, bins=5)

cam = ses.GetCamera()
cam.ViewAll()
tf = ren.GetPrimaryTransferFunction()
# tf.SetColorNormalizedHSVControlPoints([(0, (0,1,1)), (1, (0.5,1,1))])
# tf.SetColorNormalizedHSVControlPoints([(h/20, (h/20,1,1)) for h in range(0,20)])
# tf.SetColorNormalizedRGBControlPoints([(0, (1,0,0)), (1, (0,1,0))])
tf.SetColorRGBList([(1,0,0), (0,1,0), (0,0,1)])
tf.LoadBuiltinColormap("Sequential/matter")
# tf.ShowMatPlotLibColorbar()
# exit()
ses.Show()
ren.SetEnabled(False)
# ses.Render("out-data-test-3.png")

# ren = data.NewRenderer(renderer.ImageRenderer)
# for map in ren.ListBuiltinMaps():
#     ren.SetBuiltinMap(map)
#     ses.Show()
# ses.DeleteRenderer(ren)

ren = data.NewRenderer(renderer.FlowRenderer)
ren.SetFieldVariableNames(["data_2d_1"]*2)
ren.SetRenderType(ren.RenderType.RenderTypeSamples)
ren.SetRenderRadiusScalar(5)
ren.SetRenderGeom3D(True)
ren.SetGridNumOfSeeds([5,5])
ses.Show()
ses.DeleteRenderer(ren)


ren = data.NewRenderer(renderer.BarbRenderer)
ren.SetFieldVariableNames(["data_2d_1"]*2)
ren.GetRenderRegion().SetExtents([5,5],[40,40])
ren.SetXBarbsCount(3)
ses.Show()
ses.DeleteRenderer(ren)

ren = data.NewRenderer(renderer.ContourRenderer)
ren.SetVariableName("data_2d_1")
dmin = ren.GetPrimaryTransferFunction().GetMinMapValue()
dmax = ren.GetPrimaryTransferFunction().GetMaxMapValue()
isos = [dmin + (dmax-dmin)*i/40 for i in range(40)]
ren.SetIsoValues(isos)
ses.Show()
# ses.DeleteRenderer(ren)

# arr = sample(lambda x,y,z: sin(10*x)+sin(10*y)+sin(10*z))
arr = sample(lambda x,y,z: (x+y+z)*np.exp(-5.0*(x**2+y**2+z**2)), ext=[(-1,1)]*3)
data.AddNumpyData("data_3d_1", arr)
ren = data.NewRenderer(renderer.VolumeRenderer)
ren.SetVariableName("data_3d_1")
tf = ren.GetPrimaryTransferFunction()
# tf.SetOpacityList([0,0,1])
tf.SetOpacityList([1,0,1])

# cam.ViewAll()
cam.LookAt((-120,120,120), (32,32,32))
ses.Show()

ren.SetEnabled(False)
ren:renderer.VolumeIsoRenderer = data.NewRenderer(renderer.VolumeIsoRenderer)
print("Iso Algo=", ren.GetAlgorithm())
# ren.SetIsoValues([1.8])
ren.SetIsoValues([-0.1, 0.1])
ses.Show()
ses.DeleteRenderer(ren)

sa = ses.GetAxisAnnotations()
sa.SetAxisAnnotationEnabled(True)

ren:renderer.SliceRenderer = data.NewRenderer(renderer.SliceRenderer)
ses.Show()
ses.DeleteRenderer(ren)
