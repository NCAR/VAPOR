from . import link
from .params import *

link.include('vapor/ColorbarPbase.h')

class ColorbarAnnotation(ParamsWrapper, wrap=link.VAPoR.ColorbarPbase):
    _wrap = FuncWrapperStrList("""
        GetCornerPosition
        SetCornerPosition
        GetSize
        SetSize
        GetTitle
        SetTitle
        IsEnabled
        SetEnabled
        GetFontSize
        SetFontSize
        GetNumTicks
        SetNumTicks
        GetNumDigits
        SetNumDigits
        GetBackgroundColor
        SetBackgroundColor
    """)
    _tags = ParamsTagWrapperList("""
        bool UseScientificNotationTag
    """)


link.include('vapor/AnnotationParams.h')

class SceneAnnotation(ParamsWrapper, wrap=link.VAPoR.AnnotationParams):
    _wrap = FuncWrapperStrList("""
        GetDomainColor
        SetDomainColor
        GetUseDomainFrame
        SetUseDomainFrame
        GetUseRegionFrame
        SetUseRegionFrame
        GetRegionColor
        GetRegionColor
        GetBackgroundColor
        SetBackgroundColor
        GetCurrentAxisDataMgrName
        SetCurrentAxisDataMgrName
        SetAxisFontSize
        GetAxisFontSize
        GetTimeLLX
        SetTimeLLX
        GetTimeLLY
        SetTimeLLY
        GetTimeColor
        SetTimeColor
        GetTimeType
        SetTimeType
        GetTimeSize
        SetTimeSize
        GetAxisArrowEnabled
        GetAxisArrowSize
        GetAxisArrowXPos
        GetAxisArrowYPos
        SetAxisArrowEnabled
        SetAxisArrowSize
        SetAxisArrowXPos
        SetAxisArrowYPos
    """)
    _tags = ParamsTagWrapperList("""
    """)

    class TimeAnnotationType:
        NoAnnotation = 0
        Timestep = 1
        User = 2
        Formatted = 3


link.include('vapor/AxisAnnotation.h')

class AxisAnnotation(ParamsWrapper, wrap=link.VAPoR.AxisAnnotation):
    _wrap = FuncWrapperStrList("""
        SetAxisAnnotationEnabled
        GetAxisAnnotationEnabled
        GetAxisBackgroundColor
        GetAxisBackgroundColor
        SetAxisBackgroundColor
        GetAxisColor
        SetAxisColor
        SetNumTics
        GetNumTics
        SetAxisOrigin
        GetAxisOrigin
        SetMinTics
        GetMinTics
        SetMaxTics
        GetMaxTics
        SetTicSize
        GetTicSize
        SetXTicDir
        GetXTicDir
        SetYTicDir
        GetYTicDir
        SetZTicDir
        GetZTicDir
        SetTicDirs
        GetTicDirs
        GetTicWidth
        SetTicWidth
        GetAxisTextHeight
        SetAxisTextHeight
        GetAxisDigits
        SetAxisDigits
        SetLatLonAxesEnabled
        GetLatLonAxesEnabled
        GetShowAxisArrows
        SetShowAxisArrows
        SetAxisFontSize
        GetAxisFontSize
    """)
    _tags = ParamsTagWrapperList("""
    """)
