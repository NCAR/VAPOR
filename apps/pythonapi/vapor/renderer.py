from . import link
from .params import *
from .transferfunction import *
from . import config
from .annotations import *
from .transform import *
from pathlib import Path

link.include('vapor/ControlExecutive.h')

link.include('vapor/Box.h')
class BoundingBox():
    def __init__(self, toWrap:link.VAPoR.Box):
        self._params = toWrap

    def SetExtents(self, min, max):
        """Sets the region extents. min and max can be 2 or 3-element vectors depending on the dimension of the region."""
        self._params.SetExtents(min, max)

    def GetExtents(self):
        """
        Returns a tuple (min, max) containing the respective region bounds
        """
        min = link.gbl.std.vector[link.gbl.double]()
        max = link.gbl.std.vector[link.gbl.double]()
        self._params.GetExtents(min, max)
        return list(min), list(max)


RenderParams = link.VAPoR.RenderParams

class Renderer(ParamsWrapper, wrap=RenderParams):
    _wrap = FuncWrapperStrList("""
    SetEnabled
    IsEnabled
    
    GetVariableName
    SetAuxVariableNames
    GetAuxVariableNames
    SetFieldVariableNames
    GetFieldVariableNames
    SetXFieldVariableName
    SetYFieldVariableName
    SetZFieldVariableName
    GetXFieldVariableName
    GetYFieldVariableName
    GetZFieldVariableName
    SetHeightVariableName
    GetHeightVariableName
    SetColorMapVariableName
    GetColorMapVariableName
    
    SetUseSingleColor
    UseSingleColor
    
    GetTransform
    ResetUserExtentsToDataExents
    
    SetRefinementLevel
    GetRefinementLevel
    SetCompressionLevel
    GetCompressionLevel
    """)
    VaporName = None

    def __init__(self, renderParams:link.VAPoR.RenderParams, id:str):
        super().__init__(renderParams)
        self.id = id

    def GetTransferFunction(self, varname: str) -> TransferFunction:
        return TransferFunction(self._params.GetMapperFunc(varname))

    def GetPrimaryTransferFunction(self) -> TransferFunction:
        """Returns the transfer function for the primary rendered variable.
        This is usually the variable that is being colormapped and would be
        represented by the colorbar."""
        return self.GetTransferFunction(self._params.GetActualColorMapVariableName())

    def SetVariableName(self, name:str):
        # newDim = self._params._dataMgr.GetNumDimensions(name)
        # oldDim = self._params._dataMgr.GetNumDimensions(self.GetVariableName())
        # if newDim != oldDim:
        #     self.__setDimensions(newDim)
        self._params.SetVariableName(name)
        self._params.ResetUserExtentsToDataExents()

    def GetRenderRegion(self) -> BoundingBox:
        return BoundingBox(self._params.GetBox())

    def GetColorbarAnnotation(self) -> ColorbarAnnotation:
        return ColorbarAnnotation(self._params.GetColorbarPbase())

    def SetDimensions(self, dim:int):
        assert dim == 2 or dim == 3
        self._params.BeginGroup("Change dim")
        if dim == 2:
            self._params.GetBox().SetPlanar(True)
            self._params.GetBox().SetOrientation(link.VAPoR.Box.XY)
        else:
            self._params.GetBox().SetPlanar(False)
            self._params.GetBox().SetOrientation(link.VAPoR.Box.XYZ)
        self._params.SetDefaultVariables(dim, True)
        self._params.EndGroup()


link.include('vapor/BarbParams.h')
link.include('vapor/BarbRenderer.h')
class BarbRenderer(Renderer, wrap=link.VAPoR.BarbParams):
    _wrap = FuncWrapperStrList("""
        GetGrid
        SetGrid
        GetLengthScale
        SetLengthScale
        GetLineThickness
        SetLineThickness
    """)
    _tags = ParamsTagWrapperList("""
        long _xBarbsCountTag
        long _yBarbsCountTag
        long _zBarbsCountTag
    """)


link.include('vapor/TwoDDataRenderer.h')
class TwoDDataRenderer(Renderer, wrap=link.VAPoR.TwoDDataParams):
    pass


link.include('vapor/ContourRenderer.h')
class ContourRenderer(Renderer, wrap=link.VAPoR.ContourParams):
    def GetIsoValues(self) -> list[float]:
        return self._params.GetIsoValues(self.GetVariableName())

    def SetIsoValues(self, values: list[float]):
        return self._params.SetIsoValues(self.GetVariableName(), values)


link.include('vapor/VolumeRenderer.h')
link.include('vapor/VolumeParams.h')
class VolumeRenderer(Renderer, wrap=link.VAPoR.VolumeParams):
    _wrap = FuncWrapperStrList("""
    GetAlgorithm
    SetAlgorithm
    
    GetSamplingMultiplier
    SetSamplingMultiplier
    
    SetLightingEnabled
    GetLightingEnabled
    SetPhongAmbient
    GetPhongAmbient
    SetPhongDiffuse
    GetPhongDiffuse
    SetPhongSpecular
    GetPhongSpecular
    SetPhongShininess
    GetPhongShininess
    """)
    _tags = ParamsTagWrapperList("""
        double VolumeDensityTag
        bool UseColormapVariableTag
    """)

    def SetAlgorithm(self, algorithm: str):
        """Manually set the rendering algorithm. This is usually done automatically. Valid values in GetAlgorithmNames()."""
        if algorithm not in self.GetAlgorithmNames():
            raise ValueError("Algorithm needs to be one of", self.GetAlgorithmNames())
        self._params.SetAlgorithmByUser(algorithm)

    def GetAlgorithmNames(self, types=link.VAPoR.VolumeParams.Type.DVR):
        return link.VAPoR.VolumeParams.GetAlgorithmNames(types)


link.include('vapor/VolumeIsoRenderer.h')
link.include('vapor/VolumeIsoParams.h')
class VolumeIsoRenderer(VolumeRenderer, wrap=link.VAPoR.VolumeIsoParams):
    _wrap = FuncWrapperStrList("""
    
    """)

    def GetAlgorithmNames(self):
        return super().GetAlgorithmNames(link.VAPoR.VolumeParams.Type.Iso)

    def GetIsoValues(self) -> list[float]:
        return self._params.GetIsoValues(self.GetVariableName())

    def SetIsoValues(self, values: list[float]):
        """Supports at most 4 simultaneous iso-surfaces per renderer"""
        return self._params.SetIsoValues(self.GetVariableName(), values)


link.include('vapor/FlowRenderer.h')
link.include('vapor/FlowParams.h')
class FlowRenderer(Renderer, wrap=link.VAPoR.FlowParams):
    _wrap = FuncWrapperStrList("""
        SetIsSteady
        GetIsSteady
        GetVelocityMultiplier
        SetVelocityMultiplier
        GetSteadyNumOfSteps
        SetSteadyNumOfSteps
        GetSeedGenMode
        SetSeedGenMode
        GetFlowDirection
        SetFlowDirection
        GetSeedInputFilename
        SetSeedInputFilename
        GetFlowlineOutputFilename
        SetFlowlineOutputFilename
        GetFlowOutputMoreVariables
        GetPeriodic
        SetPeriodic
        GetGridNumOfSeeds
        SetGridNumOfSeeds
        GetRandomNumOfSeeds
        SetRandomNumOfSeeds
        GetRakeBiasVariable
        SetRakeBiasVariable
        GetRakeBiasStrength
        SetRakeBiasStrength
        GetSeedInjInterval
        SetSeedInjInterval
    """)
    _tags = ParamsTagWrapperList("""
        long RenderTypeTag
        double RenderRadiusScalarTag
        bool RenderGeom3DTag
        bool RenderShowStreamDirTag
        
        long RenderGlyphTypeTag
        long RenderGlyphStrideTag
        bool RenderGlyphOnlyLeadingTag
        
        double RenderDensityFalloffTag
        double RenderDensityToneMappingTag
        
        bool RenderFadeTailTag
        long RenderFadeTailStartTag
        long RenderFadeTailStopTag
        long RenderFadeTailLengthTag
        
        double PhongAmbientTag
        double PhongDiffuseTag
        double PhongSpecularTag
        double PhongShininessTag
    """)

    FlowDir = EnumWrapper(link.VAPoR.FlowDir)
    FlowSeedMode = EnumWrapper(link.VAPoR.FlowSeedMode)
    RenderType = EnumWrapper(link.VAPoR.FlowParams.RenderType)
    GlpyhType = EnumWrapper(link.VAPoR.FlowParams.GlpyhType)

    def GetRakeRegion(self) -> BoundingBox:
        return BoundingBox(self._params.GetRakeBox())

    def GetIntegrationRegion(self) -> BoundingBox:
        return BoundingBox(self._params.GetIntegrationBox())


link.include('vapor/WireFrameRenderer.h')
link.include('vapor/WireFrameParams.h')
class WireFrameRenderer(Renderer, wrap=link.VAPoR.WireFrameParams):
    pass


link.include('vapor/ImageRenderer.h')
link.include('vapor/ImageParams.h')
class ImageRenderer(Renderer, wrap=link.VAPoR.ImageParams):
    _wrap = FuncWrapperStrList("""
        SetImagePath
        GetImagePath
        SetIsGeoRef
        GetIsGeoRef
        SetIgnoreTransparency
        GetIgnoreTransparency
    """)

    def ListBuiltinMaps(self) -> list[str]:
        tms = config.GetResource("share/images/NaturalEarth.tms")
        if not tms:
            return []
        tmsDir = Path(tms).parent
        return [f.stem for f in tmsDir.iterdir() if f.suffix == ".tms"]

    def SetBuiltinMap(self, name: str):
        path = config.GetResource(f"share/images/{name}.tms")
        self.SetImagePath(path)


link.include('vapor/SliceRenderer.h')
link.include('vapor/SliceParams.h')
class SliceRenderer(Renderer, wrap=link.VAPoR.SliceParams):
    _wrap = FuncWrapperStrList("""
        GetSlicePlaneRotation
        GetSlicePlaneOrigin
        GetSlicePlaneNormal
    """)
    _tags = ParamsTagWrapperList("""
        double XSlicePlaneOriginTag
        double YSlicePlaneOriginTag
        double ZSlicePlaneOriginTag
        double XSlicePlaneRotationTag
        double YSlicePlaneRotationTag
        double ZSlicePlaneRotationTag
        double SampleRateTag
        double SliceOffsetTag
        double SlicePlaneNormalXTag
        double SlicePlaneNormalYTag
        double SlicePlaneNormalZTag
        long SlicePlaneOrientationModeTag
    """)
    SlicePlaneOrientationMode = link.VAPoR.RenderParams.SlicePlaneOrientationMode


link.include('vapor/ModelRenderer.h')
link.include('vapor/ModelParams.h')
class ModelRenderer(Renderer, wrap=link.VAPoR.ModelParams):
    _wrap = FuncWrapperStrList("""
        
    """)
    _tags = ParamsTagWrapperList("""
        string FileTag
    """)
    SlicePlaneOrientationMode = link.VAPoR.RenderParams.SlicePlaneOrientationMode


for Class in Renderer.__subclasses_rec__():
    Class.VaporName = link.__getattr__(Class.__name__).GetClassType()


# print("Renderer:", [f for f in dir(Renderer) if not f.startswith('__')])
# print("TwoDDataRenderer:", [f for f in dir(TwoDDataRenderer) if not f.startswith('__')])
