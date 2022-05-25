from . import link
from .smartwrapper import *

link.include('vapor/ParamsBase.h')
ParamsBase = link.VAPoR.ParamsBase

class ParamsWrapper(SmartWrapper):
    def __init__(self, p:ParamsBase):
        self._params = p
        super().__init__(p)


class ParamsTagWrapper(FuncWrapper):
    def __init__(self, tag:str):
        self._tag = tag

    def __getAccessorRootName(self):
        tagName = self._tag
        if tagName.lower().endswith("tag"): tagName = tagName[0:-3]
        if tagName.startswith("_"): tagName = tagName[1:]
        tagName = tagName[0].upper() + tagName[1:]
        return tagName

    def _getSetter(wself, cls):
        raise NotImplementedError

    def _getGetter(wself, cls):
        raise NotImplementedError

    def GetFunctionsToWrap(self, cls, name:str):
        assert hasattr(cls, self._tag)
        setter = self._getSetter(cls)
        getter = self._getGetter(cls)
        setter.__name__ = "Set" + self.__getAccessorRootName()
        getter.__name__ = "Get" + self.__getAccessorRootName()
        setter.__doxygen_name__ = self._tag
        getter.__doxygen_name__ = self._tag
        return [setter, getter]


class ParamsTagWrapperLong(ParamsTagWrapper):
    def __init__(self, tag:str):
        self._tag = tag

    def _getSetter(wself, cls):
        # Self is params class
        def setter(self, value: int):
            return self.SetValueLong(getattr(cls, wself._tag), "", value)
        return setter

    def _getGetter(wself, cls):
        # Self is params class
        def getter(self) -> int:
            return self.GetValueLong(getattr(cls, wself._tag), 0)
        return getter


class ParamsTagWrapperBool(ParamsTagWrapper):
    def __init__(self, tag:str):
        self._tag = tag

    def _getSetter(wself, cls):
        # Self is params class
        def setter(self, value: bool):
            return self.SetValueLong(getattr(cls, wself._tag), "", value)
        return setter

    def _getGetter(wself, cls):
        # Self is params class
        def getter(self) -> bool:
            return self.GetValueLong(getattr(cls, wself._tag), 0)
        return getter


class ParamsTagWrapperDouble(ParamsTagWrapper):
    def __init__(self, tag:str):
        self._tag = tag

    def _getSetter(wself, cls):
        # Self is params class
        def setter(self, value: float):
            return self.SetValueDouble(getattr(cls, wself._tag), "", value)
        return setter

    def _getGetter(wself, cls):
        # Self is params class
        def getter(self) -> float:
            return self.GetValueDouble(getattr(cls, wself._tag), 0)
        return getter


class ParamsTagWrapperString(ParamsTagWrapper):
    def __init__(self, tag:str):
        self._tag = tag

    def _getSetter(wself, cls):
        # Self is params class
        def setter(self, value: str):
            return self.SetValueString(getattr(cls, wself._tag), "", value)
        return setter

    def _getGetter(wself, cls):
        # Self is params class
        def getter(self) -> str:
            return self.GetValueString(getattr(cls, wself._tag), 0)
        return getter


class ParamsTagWrapperList(FuncWrapper):
    """
    Takes following format:
        long FirstTag
        long SecondTag
        string ThirdTag
    """
    def __init__(self, slist:str):
        self._l = self.MakeList(slist)

    def MakeList(self, s: str):
        return list(filter(None, [str.strip(s) for s in s.split('\n')]))

    def __makeWrapper(self, typ, tag):
        if   typ == "long":
            return ParamsTagWrapperLong(tag)
        elif typ == "bool":
            return ParamsTagWrapperBool(tag)
        elif typ == "double":
            return ParamsTagWrapperDouble(tag)
        elif typ == "string":
            return ParamsTagWrapperString(tag)
        else:
            raise TypeError

    def GetFunctionsToWrap(self, cls, name:str):
        return [func for entry in self._l for typ, tag in [entry.split()] for func in self.__makeWrapper(typ, tag).GetFunctionsToWrap(cls, name)]
        # for entry in self._l:
        #     print(entry, "|", entry.split())
        for entry in self._l:
            print(entry.split())
            typ, tag = entry.split()
            for func in self.__makeWrapper(typ, tag).GetFunctionsToWrap(cls, name):
                print(func)