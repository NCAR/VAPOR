#!/bin/python

#$ -S /bin/python
# -m bes
# -noshell /bin/python

import os
import sys
import subprocess

def usage():
	print "Usage:"
	print "[scheduler options] parallelScript vaporTool : [vaporToolFlags] [-files fileList.txt] : [inputFiles vdfFile]"
	print ""
	print "[scheduler options]    - Options taken by the scheduler (SGE, LSF, etc)"
	print "                         to set up the environment for the array job."
	print "vaporTool              - One of Vapor's data conversion tools, such as"
	print "                         wrf2vdf, grib2vdf, etc."
	print "[vaporToolFlags]       - Vapor's data conversion tools each have their"
	print "                         own set of options that can be applied for "
	print "                         data conversion (such as -numts to specify the"
	print "                         number of timesteps to convert).  These options"
	print "                         may be also be applied here to be used by the"
	print "                         parallel script."
	print "[-files, fileList]     - Users can specify a text-file that contains a"
	print "                         list of data-files (fileList) they want to"
	print "                         perform their conversion on.  If used, the .vdf"
	print "                         file needs to be included as well."
	print "[-grid, gridFile]      - (For ROMS datasets only) ROMS requires a grid"
	print "                         file for each individual conversion that is run."
	print "				ROMS conversions must specify that grid file here."
	print "[inputFiles, .vdfFile] - If the -files flag is not applied, users may"
	print "                         list their data files on the command line."


def parseVapor(args):

	vaporFlags = []
	grid = ''

	# remove vapor[tool].py from argument list
	args.pop(0)
	myTool = ''
	if args.count(":") != 2:
		print "Command line must contain two ':' delimiters"
		return -1,[],'',[]


	# remove the vapor tool and its delimiting ':' from argument list
	i = args.index(':')
	myTool = args[i-1]
	args.pop(i)
	i = args.index(myTool)
	args.pop(i)

	# parse all command-line flags that are going to be applied
	# to the *2vdf script.	A ':' can separate user flags from 
	# user files.  If the '-inputFiles' flag is used, then files
	# are listed in a single text file and any ':' will be ignored
	try:
		i = args.index(':')
		vaporFlags = args[0:i]
		myFiles = args[i+1:]
	except:
		vaporFlags = args

	if "-files" in vaporFlags:
		i = args.index('-files')
		inFile = args[i+1]
		f = open(inFile,'r')
		myFiles = f.read().splitlines()
		vaporFlags.remove('-files')
		vaporFlags.remove(inFile)

	if "-grid" in vaporFlags:
		i = args.index('-grid')
		grid = args[i+1]
		vaporFlags.remove('-grid')
		vaporFlags.remove(grid)

	vaporFlags = ' '.join(vaporFlags)

	if len(myFiles) < 2:
		print "No input files specified."
		return -1, [],'',[]

	# 'args' is now just a list of our input files
	return 1, vaporFlags, myTool, myFiles, grid

def allocateFiles(files,first,last,myId):
	# identify our .vdf file within our 'files' list
	myVDF = ''
	r = range(0,len(files))
	for i in r:
		if '.vdf' in files[i]:
			myVDF = files[i]
			files.pop(i)
			break

	stride = (len(files)) / (last - first + 1)
	if stride == 0: stride = 1
	myFiles = files[myId*stride : (myId+1)*stride]
	leftovers = (len(files))%stride

	if (myId == last) and (leftovers != 0):
		myFiles = myFiles + files[(leftovers*-1):]
	return myFiles, myVDF

def generateCall(myVDF, myFiles, myTool, flags, binDir):
	command = myTool + ' ' + flags + ' ' 
	if 'wrf2vdf' in myTool:
		command += ' ' + myVDF + ' ' + ' '.join(myFiles)
	else:
		command += ' '.join(myFiles) + ' ' + myVDF

	sourceString = "sh;. " + binDir + r"/vapor-setup.sh;"
	call = sourceString + command
	print call
	return call
