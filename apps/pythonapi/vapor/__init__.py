from . import link
import sys
import hdf5plugin

link.include('vapor/Version.h')
vaporVersion = link.Wasp.Version()
print("Vapor", vaporVersion.GetVersionString())

print(f"Python {sys.version.split(' ')[0]} ({sys.prefix})")

link.include('vapor/Log.h')
link.Log.InfoLevelEnabled = False

link.include('vapor/Session.h')
link.Session.SetWaspMyBaseErrMsgFilePtrToSTDERR()

from . import config
link.include('vapor/ResourcePath.h')
link.Wasp.RegisterResourceFinder(config.GetResourceSafe)

link.include('vapor/MyPython.h')
link.Wasp.MyPython.IsRunningFromPython = True

link.include('vapor/GLContextProvider.h')
ctx = link.GLContextProvider.CreateContext()
print("OpenGL", ctx.GetVersion())

