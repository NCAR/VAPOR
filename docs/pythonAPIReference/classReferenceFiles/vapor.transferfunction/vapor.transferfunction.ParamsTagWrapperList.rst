.. _vapor.transferfunction.ParamsTagWrapperList:


vapor.transferfunction.ParamsTagWrapperList
-------------------------------------------


Help on class ParamsTagWrapperList in vapor.transferfunction:

vapor.transferfunction.ParamsTagWrapperList = class ParamsTagWrapperList(vapor.smartwrapper.FuncWrapper)
 |  vapor.transferfunction.ParamsTagWrapperList(slist: str)
 |  
 |  Takes following format:
 |      long FirstTag
 |      long SecondTag
 |      string ThirdTag
 |  
 |  Method resolution order:
 |      ParamsTagWrapperList
 |      vapor.smartwrapper.FuncWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetFunctionsToWrap(self, cls, name: str)
 |  
 |  MakeList(self, s: str)
 |  
 |  __init__(self, slist: str)
 |      Initialize self.  See help(type(self)) for accurate signature.
 |  
 |  ----------------------------------------------------------------------
 |  Methods inherited from vapor.smartwrapper.FuncWrapper:
 |  
 |  GetFunctionNamesToWrap(self, cls, name) -> list[str]
 |  
 |  GetWrappedFunctionName(self, memberName, func) -> str
 |  
 |  ----------------------------------------------------------------------
 |  Data descriptors inherited from vapor.smartwrapper.FuncWrapper:
 |  
 |  __dict__
 |      dictionary for instance variables (if defined)
 |  
 |  __weakref__
 |      list of weak references to the object (if defined)

