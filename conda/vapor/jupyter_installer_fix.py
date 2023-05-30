import argparse
import os
import shutil

parser = argparse.ArgumentParser()
parser.add_argument('-n', '--dryRun', action='store_true')
parser.add_argument('-w', '--widgetDir', required=True)
parser.add_argument('-o', '--outRootDir', required=True)
args = parser.parse_args()
print(args)

widgetRoot = args.widgetDir
widgetSetup = f"{widgetRoot}/setup.py"
root = args.outRootDir

assert(os.path.isdir(widgetRoot))

def nullFunc(ret=None):
    return lambda *args, **kwargs: ret

import setuptools
setuptools.setup = nullFunc()

import jupyter_packaging
jupyter_packaging.create_cmdclass = nullFunc(ret=dict())
jupyter_packaging.combine_commands = nullFunc()
jupyter_packaging.install_npm = nullFunc()

with open(widgetSetup) as f:
    exec(compile(f.read(), widgetSetup, "exec"))

# print("data_files_spec =", data_files_spec)

toInstall = jupyter_packaging.get_data_files(data_specs=data_files_spec, top=widgetRoot)

for outDir, files in toInstall:
    outDir = root + "/" + outDir
    if not args.dryRun:
        os.makedirs(outDir, exist_ok=True)

    for file in files:
        print(f"Install {file} in {outDir}")
        if not args.dryRun:
            shutil.copy(widgetRoot+"/"+file, outDir)
