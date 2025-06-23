from . import link
from .params import *
from .common import *
from . import config
from typing import Iterable, Any
import functools
from pathlib import Path
import pylab as pl
import numpy as np

link.include('vapor/MapperFunction.h')

class TransferFunction(ParamsWrapper, wrap=link.VAPoR.MapperFunction):
    _wrap = FuncWrapperStrList("""
        getOpacityScale
        setOpacityScale
        getMinMapValue
        setMinMapValue
        getMaxMapValue
        setMaxMapValue
        getMinMaxMapValue

        LoadFromFile
        LoadColormapFromFile
        """).EnsureNamesStartUppercase()

    def LoadBuiltinColormap(self, name:str) -> None:
        """See ListBuiltinColormaps"""
        return self.LoadColormapFromFile(config.GetResource(f'share/palettes/{name}.tf3'))

    @classmethod
    def ListBuiltinColormaps(cls) -> list[str]:
        root = Path(config.GetResource(f'share/palettes'))
        return [f"{category.name}/{color.stem}" for category in root.iterdir() for color in category.iterdir()]

    def __normalizeControlPoints(self, cp: list[tuple[float, Any]]):
        rMin = self.GetMinMapValue()
        rMax = self.GetMaxMapValue()
        normalize = lambda x: (x - rMin) / (rMax - rMin) if rMax - rMin > 0 else x
        return [(normalize(x), y) for x, y in cp]

    @staticmethod
    def __enumerateNormDist(l: list[float]):
        if not l: return []
        if len(l) == 1: return [(0.5, l[0])]
        return [(n / (len(l) - 1), v) for n, v in enumerate(l)]

    @staticmethod
    def __swapEachXY(l: list):
        return [(y, x) for x, y in l]

    @staticmethod
    def __flatten(l: list):
        def flatten(l):
            for i in l:
                if isinstance(i, Iterable):
                    for j in flatten(i):
                        yield j
                else:
                    yield i
        return list(flatten(l))

    def __rgbToHsv(self, rgb):
        if len(rgb) != 3:
            raise ValueError("rgb list must have 3 values")
        return [*self._params.rgbToHsv(rgb)]

    def SetOpacityNormalizedControlPoints(self, cp: list[tuple[float, float]]):
        """Sets opacities for normalized x,y values"""
        oMap = self._params.GetOpacityMap(0)
        # cp = [y0, x0, y1, x1, ...]
        oMap.SetControlPoints(self.__flatten(self.__swapEachXY(cp)))

    def SetOpacityList(self, opacities: list[float]):
        """Sets opacities as equally spaced control points"""
        self.SetOpacityNormalizedControlPoints(self.__enumerateNormDist(opacities))

    def SetOpacityControlPoints(self, cp: list[tuple[float, float]]):
        """
        Sets opacity points for x,y values where MinMapValue<=x<=MaxMapValue and 0<=y<=1
        Expects a list of form [[x1, y1], [x2, y2], ...] with x representing data values and y representing opacity values
        Warning: The points are stored as normalized coordinates so if the MapValue range changes
        it will not be reflected in the mapping range
        """
        self.SetOpacityNormalizedControlPoints(self.__normalizeControlPoints(cp))
    
    def GetOpacityControlPoints(self):
        """
        Returns opacity points in the form [[x1, y1], [x2, y2], ...] 
        with x representing data values and y representing opacity values
        """
        return np.array(self._params.GetOpacityMap(0).GetControlPoints()).reshape(-1, 2)

    def SetColorNormalizedHSVControlPoints(self, cp: list[tuple[float, Vec3]]):
        """Sets colormap for normalized data values"""
        cMap: link.VAPoR.ColorMap = self._params.GetColorMap()
        # cp = [h0, s0, v0, x0, h1, s1, v1, x1, ...]
        cMap.SetControlPoints(self.__flatten(self.__swapEachXY(cp)))

    def SetColorHSVList(self, colors: list[Vec3]):
        """Sets colormap as equally spaced control points"""
        self.SetColorNormalizedHSVControlPoints(self.__enumerateNormDist(colors))

    def ReverseColormap(self):
        """Reverses the colormap"""
        self.SetColorRGBList([(r, g, b) for r, g, b, _ in list(reversed(self.GetMatPlotLibColormap().colors))])

    def SetColorHSVControlPoints(self, cp: list[tuple[float, Vec3]]):
        """
        Sets opacities for x,y values where MinMapValue<=x<=MaxMapValue and 0<=y<=1
        Warning: The points are stored as normalized coordinates so if the MapValue range changes
        it will not be reflected in the mapping range
        """
        self.SetColorNormalizedHSVControlPoints(self.__normalizeControlPoints(cp))

    def __rgbEquivalent(func):
        def dec1(name):
            @functools.wraps(func)
            def dec(self, cp):
                if len(cp[0]) == 2:
                    return func(self, [(v, self.__rgbToHsv(rgb)) for v, rgb in cp])
                else:
                    return func(self, [self.__rgbToHsv(rgb) for rgb in cp])
            return dec
        return dec1

    @__rgbEquivalent(SetColorHSVList)
    def SetColorRGBList(self): pass

    @__rgbEquivalent(SetColorNormalizedHSVControlPoints)
    def SetColorNormalizedRGBControlPoints(self): pass

    @__rgbEquivalent(SetColorHSVControlPoints)
    def SetColorRGBControlPoints(self):pass

    def GetMatPlotLibColormap(self):
        from matplotlib.colors import ListedColormap
        lut = self._params.makeLut()
        lut = [[*lut[i:i + 3], 1] for i in range(0, len(lut), 4)]
        return ListedColormap(lut)

    def GetMatPlotLibColorbar(self, axes="auto", figsize=(9,1.5), **kwargs):
        """
        Shows a colorbar for Vapor's transfer function using pylab.colorbar.
        This function takes the same kwargs as pylab.colorbar and those
        parameters are passed onto the matplotlib function.
        """
        cmap = self.GetMatPlotLibColormap()

        # a = [[0, 1]]
        a = [self.GetMinMaxMapValue()]
        pl.figure(figsize=figsize)
        img = pl.imshow(a, cmap=cmap)
        pl.gca().set_visible(False)

        kwargs.setdefault("orientation", "horizontal")

        if axes=="auto":
            if kwargs['orientation'] == "horizontal":
                cax = pl.axes([0.1, 0.2, 0.8, 0.6])
            elif kwargs['orientation'] == "vertical":
                cax = pl.axes([0.2, 0.1, 0.6, 0.8])
            else:
                raise ValueError
        else:
            cax = axes

        kwargs.setdefault("cax", cax)

        cbar = pl.colorbar(**kwargs)
        # pl.savefig("c.png")
        return cbar

    def ShowMatPlotLibColorbar(self, axes="auto", figsize=(9, 1.5), **kwargs):
        """
        Shows a colorbar for Vapor's transfer function using pylab.colorbar.
        This function takes the same kwargs as pylab.colorbar and those
        parameters are passed onto the matplotlib function.
        """
        self.GetMatPlotLibColorbar(axes, figsize, **kwargs)
        pl.show()