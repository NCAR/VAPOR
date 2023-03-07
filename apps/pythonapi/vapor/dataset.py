from . import link
from .smartwrapper import *
from .renderer import *
from .transform import *
import numpy as np
import xarray as xr

link.include('vapor/PythonDataMgr.h')
link.include('vapor/DCRAM.h')
link.include('vapor/XmlNode.h')
link.include('vapor/GUIStateParams.h')

VDC = "vdc"
WRF = "wrf"
CF = "cf"
MPAS = "mpas"
BOV = "bov"
UGRID = "ugrid"
PYTHON = "ram"

class Dataset(SmartWrapper, wrap=link.VAPoR.DataMgr):
    _wrap = FuncWrapperStrList("""
    GetDimensionNames
    GetDimensionLength
    GetDataVarNames
    GetCoordVarNames
    GetTimeCoordVarName
    GetVarGeometryDim
    GetVarTopologyDim
    GetVarCoordVars
    GetNumTimeSteps
    IsTimeVarying
    """)

    def __init__(self, dataMgr:link.VAPoR.DataMgr, id:str, ses):
        super().__init__(dataMgr)
        self.id = id
        self.ses = ses

    def NewRenderer(self, Class: Renderer) -> Renderer:
        return self.ses.NewRenderer(Class, self.id)

    def GetName(self):
        return str(self.id)

    def __repr__(self):
        return f'Dataset("{self.GetName()}")'

    def GetTransform(self):
        pm = self.ses.ce.GetParamsMgr()
        vp = pm.GetViewpointParams(self.ses.GetPythonWinName())
        t = vp.GetTransform(self.id)
        return Transform(t)

    def GetDataRange(self, varname: str, atTimestep: int = 0):
        c_range = link.std.vector[link.double]()
        self._wrappedInstance.GetDataRange(atTimestep, varname, 0, 0, c_range)
        return list(c_range)


    @staticmethod
    def GetDatasetTypes():
        return [VDC, WRF, CF, MPAS, BOV, UGRID]


DC = link.VAPoR.DC

class PythonDataset(Dataset, wrap=link.VAPoR.PythonDataMgr):
    def __checkNameValid(self, name):
        if not link.VAPoR.XmlNode.IsValidXMLElement(name):
            raise Exception(f"The variable name '{name}' must be a valid XML tag, i.e. [a-Z0-9_-]+")


    def AddNumpyData(self, name:str, arr:np.ndarray):
        """
        Vapor expects data to be in order='C' with X as the fastest varying dimension.
        You can swap your axes with np.swapaxes(data, 0, -1).
        """
        self.__checkNameValid(name)
        # assert arr.dtype == np.float32
        if arr.__array_interface__['strides']:
            arr = arr.copy() # Flatten data
        self._wrappedInstance.AddRegularData(name, np.float32(arr), arr.shape)
        # TODO: Only clear necessary renderers
        self.ses.ce.ClearAllRenderCaches()


    def AddXArrayData(self, varName:str, arr:xr.DataArray):
        """
        Vapor supports grids commonly used in earth science data.
        Vapor expects data to be in order='C' with X as the fastest varying dimension.
        You can swap your axes with np.swapaxes(data, 0, -1).
        """
        self.__checkNameValid(varName)
        # assert arr.dtype == np.float32
        dc = self._wrappedInstance.GetDC()
        dimNames = []
        dimNameMap = {}
        for name,length in arr.sizes.items():
            genName = f"__{varName}_dim_{name}"
            dim = DC.Dimension(genName, length)
            dc.AddDimension(dim)
            dimNames += [genName]
            dimNameMap[name] = genName

        coordNames = []
        for axis,name in enumerate(arr.coords):
            genName = f"__{varName}_coord_{name}"
            xCoord = arr[name]
            # assert xCoord.dtype == np.float32
            periodic = [False]*len(xCoord.dims)
            uniformHint = False
            timeDim = ""
            mappedDims = [dimNameMap[d] for d in xCoord.dims]
            coord = DC.CoordVar(genName, "m", DC.FLOAT, periodic, axis, uniformHint, mappedDims, timeDim)
            dc.AddCoordVar(coord, np.float32(xCoord.data))
            coordNames += [genName]

        meshGenName = f"__{varName}_mesh_{DC.Mesh.MakeMeshName(dimNames)}"
        mesh = DC.Mesh(meshGenName, dimNames, coordNames)
        dc.AddMesh(mesh)

        periodic = [False] * len(arr.dims)
        timeCoordVar = ""
        var = DC.DataVar(varName, "", DC.FLOAT, periodic, mesh.GetName(), timeCoordVar, DC.Mesh.NODE)

        dc.AddDataVar(var, np.float32(arr.data));

        for v in [varName]+coordNames:
            self._wrappedInstance.ClearCache(v)

        # TODO: Only clear necessary renderers
        self.ses.ce.ClearAllRenderCaches()
