import cppyy
from . import config

for path in config.GetIncludeDirs():
    cppyy.add_include_path(path)

for path in config.GetLibraryDirs():
    # print(f"Add lib path '{path}'")
    cppyy.add_library_path(path)

cppyy.cppdef(config.GetCompileDefinitions())

cppyy.load_library('vapi')

# if platform.system() == "Darwin":
#     cppyy.cppdef("#define Darwin 1")

# from cppyy import include, gbl

class Link:
    from cppyy import gbl

    def include(self, path):
        # print("- include", path)
        return cppyy.include(path)

    def __getattr__(self, name):
        return getattr(cppyy.gbl, name)


import sys
sys.modules[__name__] = Link()
