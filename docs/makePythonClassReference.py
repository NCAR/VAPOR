#!/usr/bin/env python

import vapor
import pkgutil
import sys
import os

classReferenceFileName = "pythonAPIReference/classReference.rst"
classReferenceFile = open(classReferenceFileName, "w")
classReferenceFile.writelines([".. _classReference:\n\n"])
classReferenceFile.writelines(["Class Reference\n"])
classReferenceFile.writelines(["_______________\n\n"])
classReferenceFile.writelines([".. toctree::\n"])
classReferenceFile.writelines(["   :maxdepth: 1\n\n"])

classReferenceFiles = "pythonAPIReference/classReferenceFiles"
if (os.path.isdir(classReferenceFiles)):
    import shutil
    shutil.rmtree(classReferenceFiles)
os.mkdir(classReferenceFiles)

package=vapor
for importer, modName, ispkg in pkgutil.walk_packages(path=package.__path__,
                                                      prefix=package.__name__+'.',
                                                      onerror=lambda x: None):
    mod = __import__(modName, fromlist=["vapor"])
    classReferenceFile.writelines(["   classReferenceFiles//" + modName + ".rst\n"])

    modDir = classReferenceFiles + "//" + modName + "//"
    os.mkdir(modDir)

    modFileName = modDir[0:-2] + ".rst"
    modFile = open(modFileName, "w")
    modFile.writelines([".. _" + modName + ":\n\n"])
    modFile.writelines([modName + "\n"])
    modFile.writelines([str("-" * len(modName)) + "\n\n"])
    modFile.writelines([".. toctree::\n"])
    modFile.writelines(["   :maxdepth: 1\n\n"])

    classes = dict([(name, cls) for name, cls in mod.__dict__.items() if isinstance(cls, type)])

    for myClass in classes:
        className = modName + "." + myClass
        out = sys.stdout
        classFileName = modDir + className + ".rst"

        # Add this class's .rst file to moduleFile toctree
        modFile.writelines(["   " + modName + "//" + className + ".rst\n"])

        # write file through stdout, since help() outputs through stdout
        sys.stdout = open(classFileName, "w")
        #print(":orphan:")
        print(".. _" + className + ":")
        print("\n")
        print(className)
        print("-" * len(className))
        print("\n")
        help(className)
        sys.stdout.close()
        sys.stdout = out

modFile.close()
classReferenceFile.close()
print("END END END END END")
print("END END END END END")
print("END END END END END")
print("END END END END END")
print("END END END END END")
