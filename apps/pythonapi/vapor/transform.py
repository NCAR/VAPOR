from . import link
from .params import *

link.include('vapor/Transform.h')

class Transform(ParamsWrapper, wrap=link.VAPoR.Transform):
    _wrap = FuncWrapperStrList("""
        GetRotations
        SetRotations
        GetTranslations
        SetTranslations
        GetScales
        SetScales
        GetOrigin
        SetOrigin
    """)
    _tags = ParamsTagWrapperList("""
        
    """)
