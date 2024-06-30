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
DCP = "dcp"
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

    GetMeshNames
    GetMesh
    GetDimLens
    """)

    def __init__(self, dataMgr:link.VAPoR.DataMgr, id:str, ses):
        super().__init__(dataMgr)
        self.id = id
        self.ses = ses

    def NewRenderer(self, Class: Renderer) -> Renderer:
        return self.ses.NewRenderer(Class, self.id)

    def GetName(self):
        return str(self.id)
    
    def __str__(self):
        output = []
        # Dataset Name
        output.append(f"Dataset: {self.GetName()}")
        # Dimensions
        output.append("Dimensions:")
        for dim in self.GetDimensionNames():
            output.append(f"  {dim}: {self.GetDimensionLength(dim, 0)}")
        # Coordinates
        coord_var_names = self.GetCoordVarNames()
        if len(coord_var_names) > 0:
            output.append(f"Coordinate Variable Names: {coord_var_names}")
        # Variables
        output.append("Data Variables:")
        for var in self.GetDataVarNames():
            output.extend([
                f"  {var}",
                f"    Dimensionality: {self.GetVarGeometryDim(var)}",
                f"    Number of Timesteps: {self.GetNumTimeSteps(var)}",
                f"    Coordinates: {self.GetVarCoordVars(var, True)}",
                f"    Data Range: {self.GetDataRange(var)}"
            ])

        return "\n".join(output)

    def __repr__(self):
        return self.__str__()

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
        self._wrappedInstance.AddRegularData(name, np.float32(arr), tuple(reversed(arr.shape)))
        # TODO: Only clear necessary renderers
        self.ses.ce.ClearAllRenderCaches()


    def AddXArrayData(self, varName:str, arr:xr.DataArray):
        """
        Vapor supports grids commonly used in earth science data.
        It is recommended to import more complex datasets directly using Session.OpenDataset() as this will ensure coordinates and time varying data are handled automatically.
        Since xarray does not distinguish temporal dimensions your data will be interpeded as len(n.dims) space-dimensional, therefore arr must only contain spacial dimensions.
        Vapor expects data to be in order='C' with X as the fastest varying dimension.
        """

        assert len(arr.coords) == 0 or len(arr.coords) >= len(arr.dims)

        if not arr.coords:
            return self.AddNumpyData(varName, arr.data)
        
        self.__checkNameValid(varName)
        # assert arr.dtype == np.float32
        dc = self._wrappedInstance.GetDC()
        dimNames = []
        dimNameMap = {}
        xDims = list(arr.sizes.items())
        # DC::Mesh requires dimensions to be specified in reverse order
        # i.e. if data is slowest to fastest, DC::Mesh expects them in fastest to slowest
        xDims.reverse()

        for name,length in xDims:
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
            mappedDims.reverse() # DC.CoordVar expects these in fastest to slowest
            coord = DC.CoordVar(genName, "m", DC.FLOAT, periodic, axis, uniformHint, mappedDims, timeDim)
            xCoordData = xCoord.data.astype(np.float32, copy=False)
            dc.AddCoordVar(coord, np.float32(xCoordData))
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
