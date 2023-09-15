from distutils.core import setup
from pathlib import Path
import itertools
import sys, os

import cmake

print("========================================================================")
import sys; print(f"Python Version = {sys.version.split(' ')[0]} ({sys.prefix})")
if cmake.BUILD_TYPE.lower() != "release":
    print("WARNING building wheel with non-release build")
print("========================================================================")


def GenerateSetupDataFilesFormattedListForDir(root, prefix=""):
    root = Path(root)
    prefix = Path(prefix)
    files = filter(Path.is_file, root.glob('**/*'))
    files = sorted(files, key=lambda f:f.parent)
    ret = [(str(prefix / g.relative_to(root)), [str(p) for p in f]) for g, f in itertools.groupby(files, lambda f:f.parent)]
    return ret


def PrintSetupDataFilesFormattedList(l):
    for target, files in l:
        print(target)
        for f in files:
            print(f"\t{f}")


cpackComponents = list(Path(cmake.CPACK_STAGING_DIR).iterdir())
cpackFormattedFiles = [GenerateSetupDataFilesFormattedListForDir(c, prefix='vapor') for c in cpackComponents]
cpackFormattedFiles = list(itertools.chain(*cpackFormattedFiles))


def IsCondaBuild():
    if os.environ.get('CONDA_BUILD', None) == '1': return True
    # if 'conda-bld' in sys.executable.lower(): return True
    # if 'conda-bld' in os.environ.get('PIP_CACHE_DIR', None).lower(): return True
    return False


install_requires=[
    'cppyy',
    'xarray',
    'scipy', # Required for NetCDF support in xarray
    'matplotlib',
    'ipython',
    'jupyter',
    ]

# Disable pip requirements for conda build
# Otherwise, conda needs to have these packages in its build requirements
# and they need to be installed during build-time otherwise pip will try to
# install them and break
if IsCondaBuild():
    install_requires = []

setup(name='vapor',
      version=cmake.VERSION_STRING,
      description='NCAR Vapor Python Interface',
      long_description=f"Detailed Version: {cmake.VERSION_STRING_FULL}.{cmake.BUILD_TYPE}",
      author='Vapor Team',
      author_email='vapor@ucar.edu',
      url='https://www.vapor.ucar.edu',
      packages=['vapor'],
      install_requires=install_requires,
      # Duplicates symlinked libraries (adds ~200MB to install)
      # Based on python mailing lists, this is intentional and currently the only option (https://discuss.python.org/t/symbolic-links-in-wheels/1945/12)
      data_files=cpackFormattedFiles,
      # data_files=[
          # ('vapor/lib', list(map(str, chain(cmake_lib_dir.glob("*.dylib"), cmake_lib_dir.glob("*.so"))))),
          # ('vapor/doc', [str(cmake_doc_dir)]),
          # ('vapor', cmakeInstallFiles),
      # ],
      # package_data = {'vapor': [f"{cmake_lib_dir}/*.dylib"]},
      # package_data = {'': ["/Users/stasj/Work/vapor-xcode/lib/Debug/libvdc.dylib"]},
      # package_data = {'': ["*.dat", "test_folder/*.dat"]},
      )
