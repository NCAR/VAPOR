import sys, os
import site
import re
from pathlib import Path
from . import cmake

sourcePaths = [
    cmake.SOURCE_DIR,
    cmake.SOURCE_DIR + "/lib/osgl",
    cmake.SOURCE_DIR + "/lib/osgl/glad",
    cmake.BINARY_DIR,
    cmake.THIRD_PARTY_DIR,
]

# Files installed using setup(data_files) are placed in one of the following two locations:
#   sys.prefix for system installations
#   site.USER_BASE for user installations
# Conda installs files using setup(data_files) in a different location than pip.
# Conda installs them in the module root like package_data

installPaths = [
    site.USER_BASE,
    sys.prefix,
]
installPaths = [Path(p)/'vapor' for p in installPaths]

modulePaths = [Path(__file__).parent]

condaPaths = [os.getenv('CONDA_PREFIX', "/")]

def PathExists(path):
    try:
        return path.exists()
    except PermissionError:
        return False

roots = sourcePaths + installPaths + modulePaths + condaPaths
allRoots = roots.copy()
roots = map(Path, roots)
roots = filter(PathExists, roots)
roots = [*roots]

# print("Resource Roots:\n\t" + "\n\t".join(map(str, roots)))

if not roots:
    print("Error: Could not find any valid resource paths from", allRoots)
    quit(1)

def GetAllResources(relPath):
    """For source builds where there can be, for example, multiple lib dirs"""
    ret = []
    for root in roots:
        if (root / relPath).exists():
            ret.append(str(root / relPath))
    if ret:
        return ret
    raise FileNotFoundError

def GetResource(relPath):
    for root in roots:
        if (root / relPath).exists():
            return str(root / relPath)
    raise FileNotFoundError
    return None

def GetResourceSafe(relPath):
    relPath = str(relPath) # If called from C++ this may be something other than a python str
    ret = ""
    try: ret = GetResource(relPath)
    except Exception: ret = ""
    if ret == None: ret = ""
    return ret

def GetDoxygenRoot():
    try: return GetResource('share/doc/xml')
    except FileNotFoundError: pass
    try: return GetResource('doc/xml')
    except FileNotFoundError: pass
    return None

def GetLibraryDirs():
    return GetAllResources('lib')

def GetIncludeDirs():
    return GetAllResources('include')

def GetCompileDefinitions(debug=False):
    cmakeDefLists = [
        cmake.VAPI_COMPILE_DEFS,
        cmake.GLOBAL_COMPILE_DEFS,
    ]
    cmakeDefLists = filter(lambda x: x and not x.endswith('-NOTFOUND'), cmakeDefLists)
    cmakeDefList = ';'.join(cmakeDefLists)
    defs = cmakeDefList.split(';')
    defs = filter(None, defs)
    cmds = ['#define ' + ' '.join(filter(bool, re.match(r'(\w+)=?(.*)', d).groups())) for d in defs]
    if debug:
        print('\n'.join([f"{d} -> '{s}'" for d,s in zip(defs, cmds)]))
    code = '\n'.join(cmds)
    return code


def IsRunningFromIPython():
    try:
        __IPYTHON__
        return True
    except NameError:
        return False
