
###########################################################
#                       NOTICE
#
# This is a utility file for running tests from source and
# can be ignored when normally using vapor
#
###########################################################

try:
    import vapor
except ImportError:
    import sys
    sys.path.append('..')


from inspect import signature
import numpy as np
from math import sin

def SampleFunctionOnRegularGrid(f, ext=None, shape=None):
    if not shape: shape = [64]*len(signature(f).parameters)
    if not ext: ext = [(0,1)]*len(signature(f).parameters)
    assert len(signature(f).parameters) == len(shape) and len(shape) == len(ext)
    d = []
    for i in np.ndindex(*reversed(shape)):
        d.append(f(*[v/s*(t-f)+f for v,s,(f,t) in zip(reversed(i), shape, ext)]))
    return np.asarray(d, dtype=np.float32).reshape(shape)


def OpenExampleDataset(session):
    data = session.CreatePythonDataset()
    data.AddNumpyData("U10", SampleFunctionOnRegularGrid(lambda x, y: sin(6 * x) + sin(6 * y)))
    data.AddNumpyData("V10", SampleFunctionOnRegularGrid(lambda x, y: (x + y) * np.exp(-5.0 * (x ** 2 + y ** 2)), ext=[(-1, 1)] * 2))
    data.AddNumpyData("V", SampleFunctionOnRegularGrid(lambda x, y, z: (x + y + z) * np.exp(-5.0 * (x ** 2 + y ** 2 + z ** 2)), ext=[(-1, 1)] * 3))
    return data