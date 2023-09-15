import re
import functools
from . import cppyyDoxygenWrapper
import cppyy


class FuncWrapper():
    def GetFunctionsToWrap(self, cls, name):
        names = self.GetFunctionNamesToWrap(cls, name)
        return [getattr(cls, name) for name in names]

    def GetFunctionNamesToWrap(self, cls, name) -> list[str]:
        assert hasattr(cls, name)
        return [name]

    def GetWrappedFunctionName(self, memberName, func) -> str:
        return func.__name__


class FuncWrapperRename(FuncWrapper):
    def __init__(self, toWrapName:str):
        self._toWrapName = toWrapName

    def GetFunctionNamesToWrap(self, cls, name) -> list[str]:
        assert hasattr(cls, self._toWrapName)
        return [self._toWrapName]

    def GetWrappedFunctionName(self, memberName, func) -> str:
        return memberName


class FuncWrapperWrapAll(FuncWrapper):
    def GetFunctionNamesToWrap(self, cls, name):
        return [f for f in dir(cls) if not f.startswith('_') and callable(getattr(cls, f))]


class FuncWrapperRegex(FuncWrapper):
    def __init__(self, regex):
        self._r = re.compile(regex)

    def GetFunctionNamesToWrap(self, cls, name):
        return [f for f in dir(cls) if self._r.fullmatch(f) is not None and callable(getattr(cls, f))]


class FuncWrapperStrList(FuncWrapper):
    def __init__(self, slist:str):
        self._l = self.MakeList(slist)
        self._ensureNamesStartUppercase = False

    def MakeList(self, s: str):
        return list(filter(None, [str.strip(s) for s in s.split('\n')]))

    def GetFunctionNamesToWrap(self, cls, name):
        for name in self._l:
            assert hasattr(cls, name)
        return self._l.copy()

    def GetWrappedFunctionName(self, memberName, func):
        name = super().GetWrappedFunctionName(memberName, func)
        if self._ensureNamesStartUppercase:
            name = name[0].upper() + name[1:]
        return name

    def EnsureNamesStartUppercase(self):
        self._ensureNamesStartUppercase = True
        return self


def FuncWrappers(n: int = 32):
    for x in range(n):
        yield FuncWrapper


class SmartWrapperMeta(cppyyDoxygenWrapper.CPPYYDoxygenWrapperMeta):
    __wrappedToWrapperRegistry = {
        cppyy.gbl.std.string: lambda s: str(s),
        cppyy.gbl.std.vector[str]: lambda v: list(map(str, v)),
        cppyy.gbl.std.vector[int]: lambda v: list(v),
        cppyy.gbl.std.vector[float]: lambda v: list(v),
        cppyy.gbl.std.vector[cppyy.gbl.long]: lambda v: list(v),
        cppyy.gbl.std.vector[cppyy.gbl.double]: lambda v: list(v),
    }

    def __new__(cls, clsname, bases, clsdict:dict, wrap:type=None):
        clsdict['_wrappedClass'] = wrap
        for name in list(clsdict.keys()):
            val = clsdict[name]
            if isinstance(val, FuncWrapper):
                assert wrap is not None
                toWrap = val.GetFunctionsToWrap(wrap, name)
                del clsdict[name]
                wrappedFunctions = [cls._MakeFunctionWrapper(wrap, f) for f in toWrap]
                wrappedDict = {val.GetWrappedFunctionName(name, f):f for f in wrappedFunctions}
                clsdict.update(wrappedDict)
        return super(SmartWrapperMeta, cls).__new__(cls, clsname, bases, clsdict, wrap=wrap)

    def __init__(cls, *args, **kwargs):
        cls.__wrappedToWrapperRegistry[cls._wrappedClass] = cls
        super().__init__(*args, **kwargs)

    @classmethod
    def AutoWrap(cls, toWrap, *args, **kwargs):
        if type(toWrap) in cls.__wrappedToWrapperRegistry:
            return cls.__wrappedToWrapperRegistry[type(toWrap)](toWrap, *args, **kwargs)
        return toWrap

    @classmethod
    def _MakeFunctionWrapper(cls, pCls, func):
        # assert hasattr(pCls, func.__name__)

        @functools.wraps(func)
        def f(self, *args, **kwargs):
            ret = func(self._wrappedInstance, *args, **kwargs)
            return cls.AutoWrap(ret)

        return super()._MakeFunctionWrapper(pCls, f)


class SmartWrapper(metaclass=SmartWrapperMeta):
    def __init__(self, toWrap):
        if not type(toWrap) == self._wrappedClass:
            raise ValueError(f"{type(toWrap)} is not {self._wrappedClass}")
        self._wrappedInstance = toWrap

    @classmethod
    def __subclasses_rec__(cls):
        return set(cls.__subclasses__()).union([s for c in cls.__subclasses__() for s in c.__subclasses_rec__()])


class EnumWrapperMeta(type):
    def __repr__(self):
        return "Enum Class"

def EnumWrapper(Class:type):
    d = dict(Class.__dict__)
    d['__doc__'] = "\n" + "Enum with the following options:" + "\n    ".join([""]+[*filter(lambda x: not x.startswith("__"), d.keys())])
    return EnumWrapperMeta(Class.__name__, Class.__bases__, d)