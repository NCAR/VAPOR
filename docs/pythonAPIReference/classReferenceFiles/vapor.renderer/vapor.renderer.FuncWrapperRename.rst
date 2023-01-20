.. _vapor.renderer.FuncWrapperRename:


vapor.renderer.FuncWrapperRename
--------------------------------


Help on class FuncWrapperRename in vapor.renderer:

vapor.renderer.FuncWrapperRename = class FuncWrapperRename(FuncWrapper)
 |  vapor.renderer.FuncWrapperRename(toWrapName: str)
 |  
 |  Method resolution order:
 |      FuncWrapperRename
 |      FuncWrapper
 |      builtins.object
 |  
 |  Methods defined here:
 |  
 |  GetFunctionNamesToWrap(self, cls, name) -> list[str]
 |  
 |  GetWrappedFunctionName(self, memberName, func) -> str
 |  
 |  __init__(self, toWrapName: str)
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

