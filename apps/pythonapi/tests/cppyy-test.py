import cppyy

cppyy.add_include_path('/Users/stasj/Work/vapor/apps/vapi')
cppyy.add_include_path('/Users/stasj/Work/vapor/include')
cppyy.add_include_path('/usr/local/VAPOR-Deps/2019-Aug/include/')

cppyy.load_library('/Users/stasj/Work/vapor-xcode/lib/Debug/libcommon.dylib')

# class Session:
#     def __init__(self):
#         print("New Session")

cppyy.include('vapor/Version.h')
vaporVersion = cppyy.gbl.Wasp.Version()
print(type(vaporVersion).__cpp_name__, "=", vaporVersion.GetVersionString())

cppyy.load_library('/Users/stasj/Work/vapor-xcode/lib/Debug/libvapi.dylib')

cppyy.include('vapor/MyPython.h')
cppyy.gbl.Wasp.MyPython.IsRunningFromPython = True

cppyy.include('GLContextProvider.h')
ctx = cppyy.gbl.GLContextProvider.CreateContext()
print("GL Version =", ctx.GetVersion())

cppyy.include('Session.h')
session = cppyy.gbl.Session()
session.Load("/Users/stasj/Work/sessions/time.vs3")
session.Render("/Users/stasj/Work/out-python-cppyy.png")

print("HERERRER ====")
exit(0)
