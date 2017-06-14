#!/usr/bin/env python

####################################################################
#                                                                  #
#         Copyright (C)  2016                                      #
#         University Corporation for Atmospheric Research          #
#         All Rights Reserved                                      #
#                                                                  #
#  File:        vdccp.py                                           #
#                                                                  #
#  Author:      Stanislaw Jaroszynski                              #
#               National Center for Atmospheric Research           #
#               PO 3000, Boulder, Colorado                         #
#                                                                  #
#  Date:        June  2016                                         #
#                                                                  #
####################################################################

import os
import sys
import argparse
import shutil
import errno

################## Utility Functions and Classes ###################

#
# Prints error message and exits program if condition fails
#
def Assert(cond, mesg):
    if not cond:
        print "vdccp.py:", mesg
        quit()

#
# Holds information about a single variable or coordinate
#
class VDCVar():
    def __init__(s, name):
        s.name = name
        s.timesteps = []
        s.minFrame = -1
        s.maxFrame = -1
        s.minCompression = -1
        s.maxCompression = -1

#
# Holds information about a file's path relative to the database and its size in bytes
#
class File():
    def __init__(s, relPath):
        global srcDir
        s.relPath = relPath
        s.size = os.path.getsize(os.path.join(srcDir, relPath))

######################## Global Variables ##########################
#
# Variables and coordinates are stored seperately because they have
# different options and follow different rules for copying.
# Min/max frames and compression is stored both for each variable
# as well as a global min/max.
#

minFrameGlobal = sys.maxint
maxFrameGlobal = -sys.maxint - 1
minCompressionGlobal = sys.maxint
maxCompressionGlobal = -sys.maxint - 1
totalSizeGlobal = 0
totalCopySize = 0

allVars = []
allCoords = []
copyVars = []
copyCoords = []
filesToCopy = []

######################## Primary Functions #########################

#
# Returns object containing argument strings and flags
#
def SetupAndParseArgs():
    ap = argparse.ArgumentParser(prog="vdccp.py", description="Copies a user defined portion of a vdc database.")
    ap.add_argument("source", type=argparse.FileType("r"), help="Source database.nc file.")
    ap.add_argument("destination", nargs="?", help="Destination directory.")
    ap.add_argument("-v", "--var", action="append", help="Variable(s) to be copied. All coordinates selected by default and can be removed with -x. Multiple can be grouped together if colon separated. If none specified, all variables selected.")
    ap.add_argument("-x", "--exclude", action="append", help="Variable(s) or coordinate(s) to be excluded. Multiple can be grouped together if colon separated. Overrides any variables added with -v.")
    ap.add_argument("-s", "--start-frame", default=0, type=int, help="Start frame. Default first frame. (Frames do not correspond directly to time steps. A single frame can contain multiple time steps.")
    ap.add_argument("-e", "--end-frame", type=int, help="End frame. Default last frame. Overrides -d. If neither -e or -d is specified, the entire data set after the start is copied.")
    ap.add_argument("-d", "--frame-count", type=int, help="Number of time frames from Start to copy. Overridden by -e.")
    ap.add_argument("-c", "--compression", type=int, help="Compression level to copy. Default maximum detail.")
    ap.add_argument("-f", "--force", action="store_true", help="Overwrites existing data if necessary.")
    ap.add_argument("-n", "--dry-run", action="store_true", help="Show what would have been transferred.")
    ap.add_argument("--coord-start-frame", default=0, type=int, help="Start frame for coordinates. All coordinate frames copied by default. -s does not affect coordinate frames.")
    ap.add_argument("--coord-end-frame", default=sys.maxint, type=int, help="End frame for coordinates. All coordinate frames copied by default. -s does not affect coordinate frames.")
    ap.add_argument("--info", action="store_true", help="Prints available variables, coordinates, compression levels, and maximum/minimum timesteps for data set and exits.")
    ap.add_argument("--info-var", nargs=1, action="append", help="Prints available variables, coordinates, compression levels, and maximum/minimum timesteps for a specific variable and exits.")
    ap.add_argument("--version", action="version", version="%(prog)s 0.9")
    return ap.parse_args()

#
# Create utility strings with relative and absolute paths to folders
# used. Assert that the expected database folders exist.
#
def GeneratePertinentPaths():
    global vdcName
    global srcDir
    global relDataDir
    global relVarDir
    global relCoordDir
    global dataDir
    global varDir
    global coordDir
    global destDir
    global hasVarDir
    global hasCoordDir

    vdcName = os.path.splitext(os.path.basename(args.source.name))[0]
    srcDir = os.path.dirname(os.path.realpath(args.source.name))
    relDataDir = vdcName + "_data"
    relVarDir = os.path.join(relDataDir, "data")
    relCoordDir = os.path.join(relDataDir, "coordinates")
    dataDir = os.path.join(srcDir, relDataDir)
    varDir = os.path.join(srcDir, relVarDir)
    coordDir = os.path.join(srcDir, relCoordDir)
    destDir = args.destination
    hasVarDir = False
    hasCoordDir = False
    
    if os.path.isdir(dataDir) and os.path.isdir(varDir): hasVarDir = True
    else: print "vdccpy.py: Warning: Data is missing"
    if os.path.isdir(coordDir): hasCoordDir = True

#
# Obtains the min/max frame and compressions for a var and updates
# global values if necessary. Information is determined only from
# file names.
# Variable directory argument is to allow the same function to be
# used for both variables and coordinates.
#
def getVarInfo(var, directory):
    global minFrameGlobal
    global maxFrameGlobal
    global minCompressionGlobal
    global maxCompressionGlobal
    global totalSizeGlobal
    for fn in os.listdir(os.path.join(directory, var.name)):
        if fn[0] == ".": continue # Ignores Finder metadata in OS X
        totalSizeGlobal += os.path.getsize(os.path.join(directory, var.name, fn))
        fn, comp = os.path.splitext(fn)
        comp = comp[3:]
        if comp == "": comp = 0
        else: comp = int(comp)
        time = int(os.path.splitext(fn)[1][1:])

        if not time in var.timesteps:
            var.timesteps.append(time)
        if var.minFrame == -1 or var.minFrame > time: var.minFrame = time
        if var.maxFrame == -1 or var.maxFrame < time: var.maxFrame = time
        if var.minCompression == -1 or var.minCompression > comp: var.minCompression = comp
        if var.maxCompression == -1 or var.maxCompression < comp: var.maxCompression = comp
    minFrameGlobal = min(minFrameGlobal, var.minFrame)
    maxFrameGlobal = max(maxFrameGlobal, var.maxFrame)
    minCompressionGlobal = min(minCompressionGlobal, var.minCompression)
    maxCompressionGlobal = max(maxCompressionGlobal, var.maxCompression)

#
# Obtains variable metadata for all variables and coordinates
#
def ScanVariableMetadata():
    global allVars
    global allCoords
    global totalSizeGlobal

    global varDir
    global coordDir
    global hasVarDir
    global hasCoordDir

    if hasVarDir:
        for fn in os.listdir(varDir):
            if fn[0] == ".": continue
            var = VDCVar(fn)
            getVarInfo(var, varDir)
            allVars.append(var)
    
    if hasCoordDir:
        for fn in os.listdir(coordDir):
            if fn[0] == ".": continue
            var = VDCVar(fn)
            getVarInfo(var, coordDir)
            allCoords.append(var)
    
    totalSizeGlobal /= (2**30) * 1.0 # Convert from bytes to GB

#
# Bound frames to available and set to bounds if not specified.
#
def CheckArgumentValidity(args):
    if args.end_frame == None:
        if args.frame_count != None:
            args.end_frame = args.start_frame + args.frame_count
        else:
            args.end_frame = maxFrameGlobal
    if args.start_frame < minFrameGlobal: args.start_frame = minFrameGlobal
    if args.end_frame > maxFrameGlobal: args.end_frame = maxFrameGlobal
    if args.compression == None: args.compression = maxCompressionGlobal
    if args.coord_start_frame < minFrameGlobal: args.coord_start_frame = minFrameGlobal
    if args.coord_end_frame > maxFrameGlobal: args.coord_end_frame = maxFrameGlobal
    if destDir == None and not args.info and not args.info_var:
        Assert(0, "Error: Copy destination not set.")

#
# Create list of variables to be copied from args. If none specified,
# add all variables.
# All coordinates are added by default.
#
def AddVariablesFromArguments():
    global args
    global copyVars
    global copyCoords

    copyCoords = allCoords[:] # All coordinates copied by default
    copyVarsNames = []
    if args.var != None:
        for varName in args.var:
            if ':' in varName:
                copyVarsNames += varName.split(':')
            else:
                copyVarsNames.append(varName)
        copyVarsNames = list(set(copyVarsNames))
        for var in allVars:
            for name in copyVarsNames:
                if var.name == name:
                    copyVars.append(var)
                    copyVarsNames.remove(name)
                    break
        Assert(len(copyVarsNames) == 0, "Error: Variable(s) {0} not found.".format('"'+copyVarsNames[0]+'"' if len(copyVarsNames) == 1 else copyVarsNames))
    else:
        copyVars = allVars[:]

#
# Remove any excluded variables or coordinates from list.
#
def ExcludeVariablesFromArguments():
    global args
    global copyVars
    global copyCoords

    excludeVarsNames = []
    if args.exclude != None:
        for varName in args.exclude:
            if ':' in varName:
                excludeVarsNames += varName.split(':')
            else:
                excludeVarsNames += varName.split(':')
        excludeVarsNames = list(set(excludeVarsNames))
        checkList = excludeVarsNames[:]
        for var in allVars + allCoords:
            for name in checkList:
                if var.name == name:
                    checkList.remove(name)
                    break
        Assert(len(checkList) == 0, "Error: Variable(s) {0} not found.".format('"'+checkList[0]+'"' if len(checkList) == 1 else checkList))
        for name in excludeVarsNames:
            for var in copyVars:
                if var.name == name:
                    copyVars.remove(var)
                    break
            for coord in copyCoords:
                if coord.name == name:
                    copyCoords.remove(coord)
                    break

def PrintDatabaseInfo():
        print "name:", vdcName
        print "time frames:", minFrameGlobal, "-", maxFrameGlobal
        print "compression levels:", minCompressionGlobal, "-", maxCompressionGlobal
        print "total size: {0:.2f}Gb".format(totalSizeGlobal)
        print "variables:", [var.name for var in allVars]
        print "coordinates:", [var.name for var in allCoords]

def PrintVariableInfo(name):
        search = [x for x in allVars + allCoords if x.name == name]
        Assert(len(search) == 1, "Error: variable \"{0}\" not found.".format(name))
        var = search[0]
        print "Variable \"{0}\":".format(var.name)
        print "\ttime frames: {0} - {1}".format(var.minFrame, var.maxFrame)

def CreateCopyFileList():
    global copyVars
    global copyCoords
    global vdcName
    global relVarDir
    global relCoordDir
    global totalCopySize

    # Add .nc database metadata file.
    filesToCopy.append(File(vdcName + ".nc"))

    # Add variable files
    for var in copyVars:
        for frame in xrange(max(var.minFrame, args.start_frame), min(var.maxFrame, args.end_frame) + 1):
            for level in xrange(var.minCompression, min(args.compression, var.maxCompression) + 1):
                relPath = os.path.join(relVarDir, var.name, "{0}.{1:04}.nc{2}".format(var.name, frame, "" if level == 0 else level))
                if os.path.exists(os.path.join(srcDir, relPath)):
                    filesToCopy.append(File(relPath))
    
    # Add coordinate files
    for coord in copyCoords:
        for frame in xrange(max(coord.minFrame, args.coord_start_frame), min(coord.maxFrame, args.coord_end_frame) + 1):
            for level in xrange(coord.minCompression, coord.maxCompression + 1):
                relPath = os.path.join(relCoordDir, coord.name, "{0}.{1:04}.nc{2}".format(coord.name, frame, "" if level == 0 else level))
                if os.path.exists(os.path.join(srcDir, relPath)):
                    filesToCopy.append(File(relPath))
    
    # Get total size of files to copy to show progress.
    for x in filesToCopy: totalCopySize += x.size

#
# Copy all files from list. If --dry-run specified, only print file
# copy info. If file already exists, show prompt to overwrite unless
# --force. Any missing directories are created. After a file is
# copied, its size is added to copiedSize and this is used to show
# a percent copied.
#
def CopyFiles():
    global filesToCopy
    global srcDir
    global destDir
    global totalCopySize
    global args

    copiedSize = 0
    for f in filesToCopy:
        destPath = os.path.join(destDir, f.relPath)
        if not os.path.exists(os.path.dirname(destPath)) and not args.dry_run:
                # Race condition where folder is created
                try: os.makedirs(os.path.dirname(destPath))
                except OSError as err:
                    if err.errno != errno.EEXIST:
                        raise
        if os.path.exists(destPath) and not args.force and not args.dry_run:
            response = raw_input("File \"{0}\" already exists. Overwrite? (y/n)".format(destPath))
            if not (response == "y" or response == "Y"):
                copiedSize += f.size
                continue
        sys.stdout.write("{0:3d}% Copying {1}\n".format(copiedSize * 100 / totalCopySize, f.relPath))
        if not args.dry_run:
            shutil.copy(os.path.join(srcDir, f.relPath), destPath)
        copiedSize += f.size
    
    print "100%"

############################### Main ############################### 

def main():
    global args
    args = SetupAndParseArgs()
    GeneratePertinentPaths()
    ScanVariableMetadata()
    CheckArgumentValidity(args)
    AddVariablesFromArguments()
    ExcludeVariablesFromArguments()

    if args.info or args.info_var != None:
        if args.info:
            PrintDatabaseInfo()
        if args.info_var != None:
            for name in [x[0] for x in args.info_var]:
                PrintVariableInfo(name)
        quit()

    CreateCopyFileList()
    CopyFiles()

if __name__ == "__main__":
    main()
