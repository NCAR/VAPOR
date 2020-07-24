# On OSX, ld builds libraries that contain hard coded path to their dependencies.
# After installation on a client machiene, these paths become invalid unless we 
# modify them to reference @rpath.  This script adds @rpath!


# This script uses the following OSX tools:
#
# otool -L <target-library>
#   Lists dependent libraries on <target-library>
# 
# install_name_tool
#   Allows us to change library and binary dependency references
#

import os
import sys
import stat
import subprocess


executable_dir   = "/Users/pearse/VAPOR/build/bin"
vaporLibrary_dir = "/Users/pearse/VAPOR/build/lib"
thirdParty_dir   = "/usr/local/VAPOR-Deps/2019-Aug/lib"

changeDependencyCommand = "/usr/bin/install_name_tool -change "
changeLibraryIDCommand  = "/usr/bin/install_name_tool -id "

executable = stat.S_IEXEC | stat.S_IXGRP | stat.S_IXOTH

def modifyPath( filename, changeFileID):

    # ask otool for a list of our target library's reference paths
    output = subprocess.check_output("otool -L " + filename, shell=True)
    output = output.decode("utf-8")

    for line in output.splitlines():  

        # remove indentations and colons
        line = line.replace("\t", "")
        line = line.replace(":", "")

        # remove library description that otool reports to us
        # such as /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1252.250.1)
        #                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        line = line.split()[0]

        # remove descriptions in parenthesis, attached to third-party libraries
        # such as /usr/local/VAPOR-Deps/2019-Aug/lib/libjpeg.a(jaricom.o)
        #                                                     ^^^^^^^^^^^
        line = line.split('(')[0]                    

        # isolate the path from the line
        location = line.split('/')[0:-1]
        location = '/'.join(location)

        # isolate the library name from the line
        #
        library = line.split('/')[-1]

        if ( location == thirdParty_dir ):
            #if ( (changeFileID == True) and "dylib" in filename ):
            if ( changeFileID == True ):
                print(          changeLibraryIDCommand + "@rpath/" + library + " " + filename )
                subprocess.run( changeLibraryIDCommand + "@rpath/" + library + " " + filename, shell=True )
            if ( changeFileID == False):
                print(          changeDependencyCommand + line + " @rpath/" + library + " " + filename )
                subprocess.run( changeDependencyCommand + line + " @rpath/" + library + " " + filename, shell=True )

# traverse root directory, and list directories as dirs and files as files
def fixLibsInDir( directory, changeFileID ):
    for root, dirs, files in os.walk(directory):
        path = root.split(os.sep)
        for file in files:
            filename = '/'.join(path) + '/' + file
            if os.path.isfile(filename) and os.access(filename, os.X_OK) and "dylib" in filename:
                modifyPath( filename, changeFileID )

# for changing rpath of our third-party libraries, such as libjpeg.dylib
fixLibsInDir( thirdParty_dir, True )
