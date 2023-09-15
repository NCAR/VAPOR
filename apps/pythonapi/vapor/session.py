from . import link
from .renderer import Renderer
from .renderer import *
from .dataset import *
from .camera import *
from .annotations import *

import PIL.Image

link.include('vapor/Session.h')
link.include('vapor/RenderManager.h')

@link.FixModuleOwnership
class Session(link.Session):
    def __init__(self):
        super().__init__()
        self.ce = super()._controlExec

    def NewRenderer(self, Class:Renderer, datasetName:str) -> Renderer:
        id = super().NewRenderer(Class.VaporName, datasetName)
        if not id: return None
        p = self.GetRenderer(id)
        p.SetEnabled(True)
        return p

    def DeleteRenderer(self, renderer:Renderer):
        super().DeleteRenderer(renderer.id)

    def GetRenderer(self, name):
        c_win = link.std.string()
        c_dat = link.std.string()
        c_typ = link.std.string()
        self.ce.RenderLookup(name, c_win, c_dat, c_typ)
        p = self.ce.GetRenderParams(c_win, c_dat, c_typ, name)
        assert p
        return SmartWrapper.AutoWrap(p, name)

    def GetRenderers(self) -> list[Renderer]:
        return [self.GetRenderer(name) for name in self.GetRendererNames()]

    def OpenDataset(self, datasetType:str, files:list[str]):
        """
        Open a dataset of type datasetType from a list of files.
        A list of supported dataset types can be retrieved from Dataset.GetDatasetTypes()
        """
        files = [files] if type(files) is not list else files
        files = [*map(str, files)]
        name = super().OpenDataset(datasetType, files)
        if (len(name) == 0): return None
        return self.GetDataset(name)

    def CreatePythonDataset(self):
        """
        Creates a python dataset or returns one if it already exists for the current session.
        """
        name = "PYTHON_RAM_DATASET"
        if self.GetDataset(name):
            return self.GetDataset(name)
        return self.OpenDataset(PYTHON, name)

    def GetDataset(self, name) -> Dataset:
        dataMgr = self.ce.GetDataStatus().GetDataMgr(name)
        if not dataMgr: return None
        return SmartWrapper.AutoWrap(dataMgr, name, self)

    def GetDatasets(self):
        return [self.GetDataset(name) for name in self.GetDatasetNames()]

    def GetCamera(self):
        return Camera(self.ce)

    def RenderToImage(self, fast=False) -> PIL.Image:
        from PIL import Image
        from array import array
        width, height = self._renderManager.GetResolution()
        nBytes = width * height * 3
        buf = array('B', [0] * nBytes)
        address, length = buf.buffer_info()
        assert length == nBytes
        self.Render(f":RAM:{address:x}", fast)
        return Image.frombytes("RGB", (width, height), buf.tobytes())

    def Show(self):
        from IPython.display import display

        img = self.RenderToImage()

        if config.IsRunningFromIPython():
            display(img)
        else:
            img.show()

    def SetResolution(self, width, height):
        self._renderManager.SetResolution(width, height)

    def GetSceneAnnotations(self) -> SceneAnnotation:
        pm = self.ce.GetParamsMgr()
        ap = pm.GetAnnotationParams(self.GetPythonWinName())
        return SceneAnnotation(ap)

    def GetAxisAnnotations(self) -> AxisAnnotation:
        return AxisAnnotation(self.GetSceneAnnotations()._params.GetAxisAnnotation())

