#!/usr/bin/python

####################################################################
#
#         Copyright (C)  2015                                      
#   University Corporation for Atmospheric Research                
#         All Rights Reserved                                      
#
#  File:        vaporLSF.py
#
#  Author:      Scott Pearse
#               National Center for Atmospheric Research
#               PO 3000, Boulder, Colorado
#
#  Date:        July 2015
#
####################################################################

import os
import sys
import subprocess
sys.path.append("..")
from vaporBatchFuncs import *

##################### USER DEFINED VARIABLES #######################


# The variable 'vaporBinDir' needs to point to where vapor's binary
# applications have been installed
vaporBinDir = '/glade/p/DASG/pearse/parallelConverter/vapor-2.4.2/bin'

# The variables first, last, and myId are used to divide a set
# of files amongst the tasks in a given batch job.  These are 
# derived from environment variables that are assigned by the
# scheduler being used.  These variables must be indexed starting
# at zero.
# first - the index of the first task in the batch submission
# last - the index of the last task in the batch submission
# myId - the index of the current task
try:
	first = 0
	last = int(os.environ["LSB_JOBINDEX_END"]) -1
	myId = int(os.environ["LSB_JOBINDEX"]) -1
except:
	print "Unable to locate array job environment variables.  Aboring."
	usage()
	sys.exit(-1)
	
###################################################################


def main():
	args = sys.argv
	myVDF = ''
	grid = ''

	rc, vaporFlags, myTool, files, grid = parseVapor(args)
	if rc != 1:
		print "Input error.  Aborting."
		usage()
		return

	myFiles, myVDF = allocateFiles(files,first,last,myId)
	if myVDF == '':
		print "Could not locate .vdf metadata file.  Aborting."
		return
	if grid != '':
		myFiles.append(grid)
 
	call = generateCall(myVDF, myFiles, myTool, vaporFlags, vaporBinDir)
	subprocess.call(call, shell=True)

if __name__ == '__main__':
	main()
