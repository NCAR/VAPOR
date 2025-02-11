import wurlitzer, re
with wurlitzer.pipes() as (out, err):
    import cppyy
print(out.read(), end="")
print("".join(l for l in err.read().splitlines() if "(ignoring for now)" not in l), end="")


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

    @staticmethod
    def FixModuleOwnership(Class):
        """
        Classes that directly inherit from CPPYY C++ classes have an incorrect __module__ attr
        """
        import inspect
        callerModule = inspect.getmodule(inspect.stack()[1][0])
        Class.__module__ = callerModule.__name__
        return Class

    def __getattr__(self, name):
        return getattr(cppyy.gbl, name)


import sys
sys.modules[__name__] = Link()
