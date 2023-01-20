.. _vapor.session.TransferFunction:


vapor.session.TransferFunction
------------------------------


Help on class TransferFunction in vapor.session:

vapor.session.TransferFunction = class TransferFunction(vapor.params.ParamsWrapper)
 |  vapor.session.TransferFunction(p: cppyy.gbl.VAPoR.ParamsBase)
 |  
 |  Wraps VAPoR::MapperFunction
 |  Parent class for TransferFunction and IsoControl, supports positioning histogram over color/opacity maps as well as a set of isovalues (as with Contours)
 |  
 |  Method resolution order:
 |      TransferFunction
 |      vapor.params.ParamsWrapper
 |      vapor.smartwrapper.SmartWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetMatPlotLibColorbar(self, axes='auto', figsize=(9, 1.5), **kwargs)
 |      Shows a colorbar for Vapor's transfer function using pylab.colorbar.
 |      This function takes the same kwargs as pylab.colorbar and those
 |      parameters are passed onto the matplotlib function.
 |  
 |  GetMatPlotLibColormap(self)
 |  
 |  GetMaxMapValue = getMaxMapValue(...)
 |      float VAPoR::MapperFunction::getMaxMapValue()
 |      Obtain maximum mapping (histo) value
 |  
 |  GetMinMapValue = getMinMapValue(...)
 |      float VAPoR::MapperFunction::getMinMapValue()
 |      Obtain minimum mapping (histo) value
 |  
 |  GetMinMaxMapValue = getMinMaxMapValue(...)
 |      vector<double> VAPoR::MapperFunction::getMinMaxMapValue()
 |      Obtain min and max mapping (histo) values
 |  
 |  GetOpacityScale = getOpacityScale(...)
 |      double VAPoR::MapperFunction::getOpacityScale()
 |      Identify the current opacity scale factor
 |  
 |  LoadBuiltinColormap(self, name: str) -> None
 |      See ListBuiltinColormaps
 |  
 |  LoadColormapFromFile(...)
 |      int VAPoR::MapperFunction::LoadColormapFromFile(string path)
 |  
 |  LoadFromFile(...)
 |      int VAPoR::MapperFunction::LoadFromFile(string path)
 |          Load a transfer function from a file,
 |      Parameters
 |          path Path of input file
 |  
 |  SetColorHSVControlPoints(self, cp: list[tuple[float, tuple[float, float, float]]])
 |      Sets opacities for x,y values where MinMapValue<=x<=MaxMapValue and 0<=y<=1
 |      Warning: The points are stored as normalized coordinates so if the MapValue range changes
 |      it will not be reflected in the mapping range
 |  
 |  SetColorHSVList(self, colors: list[tuple[float, float, float]])
 |      Sets colormap as equally spaced control points
 |  
 |  SetColorNormalizedHSVControlPoints(self, cp: list[tuple[float, tuple[float, float, float]]])
 |      Sets colormap for normalized data values
 |  
 |  SetColorNormalizedRGBControlPoints = SetColorNormalizedHSVControlPoints(self, cp: list[tuple[float, tuple[float, float, float]]])
 |      Sets colormap for normalized data values
 |  
 |  SetColorRGBControlPoints = SetColorHSVControlPoints(self, cp: list[tuple[float, tuple[float, float, float]]])
 |      Sets opacities for x,y values where MinMapValue<=x<=MaxMapValue and 0<=y<=1
 |      Warning: The points are stored as normalized coordinates so if the MapValue range changes
 |      it will not be reflected in the mapping range
 |  
 |  SetColorRGBList = SetColorHSVList(self, colors: list[tuple[float, float, float]])
 |      Sets colormap as equally spaced control points
 |  
 |  SetMaxMapValue = setMaxMapValue(...)
 |      void VAPoR::MapperFunction::setMaxMapValue(float val)
 |  
 |  SetMinMapValue = setMinMapValue(...)
 |      void VAPoR::MapperFunction::setMinMapValue(float val)
 |  
 |  SetOpacityControlPoints(self, cp: list[tuple[float, float]])
 |      Sets opacities for x,y values where MinMapValue<=x<=MaxMapValue and 0<=y<=1
 |      Warning: The points are stored as normalized coordinates so if the MapValue range changes
 |      it will not be reflected in the mapping range
 |  
 |  SetOpacityList(self, opacities: list[float])
 |      Sets opacities as equally spaced control points
 |  
 |  SetOpacityNormalizedControlPoints(self, cp: list[tuple[float, float]])
 |      Sets opacities for normalized x,y values
 |  
 |  SetOpacityScale = setOpacityScale(...)
 |      void VAPoR::MapperFunction::setOpacityScale(double val)
 |          Specify an opacity scale factor applied to all opacity maps
 |      Parameters
 |          val opacity scale factor
 |  
 |  ShowMatPlotLibColorbar(self, axes='auto', figsize=(9, 1.5), **kwargs)
 |      Shows a colorbar for Vapor's transfer function using pylab.colorbar.
 |      This function takes the same kwargs as pylab.colorbar and those
 |      parameters are passed onto the matplotlib function.
 |  
 |  ----------------------------------------------------------------------
 |  Class methods defined here:
 |  
 |  ListBuiltinColormaps() -> list[str] from vapor.smartwrapper.SmartWrapperMeta
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from vapor.params.ParamsWrapper:
 |  
 |  __init__(self, p: cppyy.gbl.VAPoR.ParamsBase)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Class methods inherited from vapor.smartwrapper.SmartWrapper:
 |  
 |  __subclasses_rec__() from vapor.smartwrapper.SmartWrapperMeta
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from vapor.smartwrapper.SmartWrapper:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)

