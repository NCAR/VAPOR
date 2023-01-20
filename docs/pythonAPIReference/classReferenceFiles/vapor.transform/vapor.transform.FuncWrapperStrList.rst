.. _vapor.transform.FuncWrapperStrList:


vapor.transform.FuncWrapperStrList
----------------------------------


Help on class FuncWrapperStrList in vapor.transform:

vapor.transform.FuncWrapperStrList = class FuncWrapperStrList(FuncWrapper)
 |  vapor.transform.FuncWrapperStrList(slist: str)
 |  
 |  Method resolution order:
 |      FuncWrapperStrList
 |      FuncWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  EnsureNamesStartUppercase(self)
 |  
 |  GetFunctionNamesToWrap(self, cls, name)
 |  
 |  GetWrappedFunctionName(self, memberName, func)
 |  
 |  MakeList(self, s: str)
 |  
 |  __init__(self, slist: str)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from FuncWrapper:
 |  
 |  GetFunctionsToWrap(self, cls, name)
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from FuncWrapper:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)

