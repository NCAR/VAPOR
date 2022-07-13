# import functools
#
# class Inner():
#     def __init__(self):
#         self.x = 0
#
#     def SetX(self, x):
#         self.x = x
#
#     def GetX(self):
#         return self.x
#
# class Outer():
#     def __init__(self):
#         self.inner = Inner()
#
# expose = ["SetX", "GetX"]
# for f in expose:
#
#     def wrapper(func):
#         @functools.wraps(func)
#         def wrapper_do_twice(*args, **kwargs):
#             return func(*args, **kwargs)
#         return wrapper_do_twice
#         # return func(*args, **kwargs)
#
#     func = getattr(Inner, f)
#
#     setattr(Outer, f, wrapper(func))
#     print("setattr(Outer, {}, {}".format(f, wrapper))
#     # setattr(Outer, "SetX", lambda self, x: self.inner.SetX(x))
#     # setattr(Outer, "GetX", lambda self: self.inner.GetX())
#     # setattr(self, f, getattr(self.inner, f))
#
# out = Outer()
# help(out)
#
# out.inner.SetX(3)
# print(out.inner.GetX())
#
# out.SetX(5)
# print(out.GetX())

#############################################################
import functools

print("============ Define Classes ============")

def log(func):
    @functools.wraps(func)
    def wrapper_debug(*args, **kwargs):
        args_repr = [repr(a) for a in args]                      # 1
        kwargs_repr = [f"{k}={v!r}" for k, v in kwargs.items()]  # 2
        signature = ", ".join(args_repr + kwargs_repr)           # 3
        value = func(*args, **kwargs)
        print(f"Call {func.__name__}({signature}) = {value!r}")
        return value
    return wrapper_debug

# class Params():
#     def Other(self):
#         print("Other")
#
# class SubParams(Params):
#     def __init__(self):
#         self.en = False
#     def SetEnabled(self, e):
#         self.en = e
#     def IsEnabled(self):
#         return self.en
#
# def funcWrapper(func):
#     @functools.wraps(func)
#     def f(self, *args, **kwargs):
#         ret = func(self._params, *args, **kwargs)
#         print(f"Call {func.__name__}({', '.join([repr(a) for a in args[1:]])}) = {ret!r}", )
#         return ret
#     return f
#
# class ParamsWrapperMeta(type):
#     def __init__(cls, *args, **kwargs):
#         super().__init__(*args, **kwargs)
#
#         for f in cls._wrap:
#             print("wrap", f)
#             func = getattr(Params, f)
#             setattr(cls, f, funcWrapper(func))
#
# class ParamsWrapperMeta2(type):
#     @log
#     def __new__(cls, clsname, bases, clsdict):
#         wrapped = {f:funcWrapper(getattr(Params, f)) for f in clsdict['_wrap']}
#         clsdict.update(wrapped)
#
#         # for f in clsdict['_wrap']:
#         #     print("wrap", f)
#         #     func = getattr(Params, f)
#         #     clsdict.update({f:funcWrapper(func)})
#
#         return type.__new__(cls, clsname, bases, dict(clsdict))
#
#     @log
#     def __init__(cls, *args, **kwargs):
#         super().__init__(*args, **kwargs)
#
# class ParamsWrapper(object, metaclass=ParamsWrapperMeta2):
#     _paramsCls = Params
#     _wrap = []
#
#     def __init__(self, p:Params):
#         self._params = p

# class Renderer(ParamsWrapper):
#     _paramsCls = SubParams
#     _wrap = ["SetEnabled", "IsEnabled"]



# print("============ Test Script ============")

# help(Renderer)
# print([f for f in dir(Renderer) if not f.startswith('__')])

# r = Renderer(Params())
# r.SetEnabled(True)
# r.IsEnabled()
# print("r._params.en =", r._params.en)


import re


class ParentClass():
    def printTest(self):
        """Prints test"""
        print(f"Test from {self.__class__.__name__}")

class ChildClass(ParentClass):
    def printTest(self):
        """Overloaded prints test"""
        print(f"Overloaded test from {self.__class__.__name__}")

    def printTest2(self):
        """Prints test 2"""
        print(f"Test 2 from {self.__class__.__name__}")


from vapor.smartwrapper import *

class ParentWrapped(SmartWrapper, wrap=ParentClass):
    printTest = FuncWrapper()
    # a, b = [FuncWrapper]*2
    # c, d, *_ = FuncWrappers()
    pass

class ChildWrapped(ParentWrapped, wrap=ChildClass):
    _ = FuncWrapperWrapAll()
    printTestRenamed = FuncWrapperRename("printTest")
    # printTest2 = FuncWrapper()



print()
print("============ Test Script ============")

pc = ParentClass()
cc = ChildClass()
pw = ParentWrapped(pc)
cw = ChildWrapped(cc)

pc.printTest()
cc.printTest()
pw.printTest()
cw.printTest()
cw.printTestRenamed()

# help(ChildWrapped)