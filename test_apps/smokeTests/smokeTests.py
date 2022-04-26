#! /usr/bin/env python3

import sys
import os
import subprocess
import difflib
import argparse

#
#  Argument Parser
#

parser = argparse.ArgumentParser(
    "A test driver for the DataMgr and Grid classes"
)
parser.add_argument( 
    '-makeBaseline', 
    default=False,
    dest='makeBaseline',
    action='store_true',
    help='This flag makes these test results the baseline on which future'
    + ' tests will be compared.  If no baseline file exists, this will automatically '
    + ' be set to true.'
)
parser.add_argument( 
    '-testDataRoot', 
    nargs=1,
    type=str,
    default="~/Data/smokeTestData", 
    required=False,
    metavar='/path/to/data',
    help='Directory where DataMgr test data is stored.'
)
parser.add_argument( 
    '-binaryRoot', 
    nargs=1,
    type=str,
    default="~/VAPOR/build/test_binaries", 
    required=False,
    metavar='/path/to/binaries',
    help='Directory where binary test programs (testGrid, testDataMgr) are stored.'
)
parser.add_argument( 
    '-resultsDir', 
    nargs=1,
    type=str,
    default="~/VAPOR/test_apps/smokeTests/testResults", 
    required=False,
    metavar='/path/to/write/results/to',
    help='Directory where test results are stored.'
)
parser.add_argument( 
    '-silenceTime',
    default=False,
    dest='silenceTime',
    action='store_true',
    help='This flag sliences the elapsed time from the printed results.'
)
args = vars(parser.parse_args())

#
#  Default directories and test data
#

gridSizes = [
#    "1:1:1",
    "2:2:2",
    "4:2:2",
    "8:2:2",
#    "1:8:8",
#    "8:1:8",
#    "8:8:1",
    "7:7:7",
    "8:8:8"
]

resultsDir = os.path.expanduser("".join( args['resultsDir'] ))
if (resultsDir[-1] != r'/'):
    resultsDir += r'/'

testDataRoot = os.path.expanduser("".join( args['testDataRoot'] ))
if (testDataRoot[-1] != r'/'):
    testDataRoot += r'/'

binaryRoot = os.path.expanduser("".join( args['binaryRoot'] ))
if (binaryRoot[-1] != r'/'):
    binaryRoot += r'/'

if( args['silenceTime'] ): silenceTime = "-silenceTime"
else: silenceTime = ""

print("resultsDir " + resultsDir )
print("testDataRoot " + testDataRoot)
print("binaryRoot " + binaryRoot )
print("silenceTime " + str(silenceTime))

dataMgrs = {
    #"mpas" : (testDataRoot + "hist.mpas-o.0001-01-01_00.00.00.nc")
    "wrf"  : (testDataRoot + "wrfout_d02_2005-08-29_05"),
    "cf"   : (testDataRoot + "24Maycontrol.04000.000000.nc"),
    "vdc"  : (testDataRoot + "katrina_1timeStep.vdc"),
}

gridProgram        = binaryRoot + "testGrid"
dataMgrProgram     = binaryRoot + "testDataMgr"
gridResultsFile    = resultsDir + "gridResults.txt"
dataMgrResultsFile = resultsDir + "dataMgrResults.txt"

#
#  Tests
#

def testGrid( grid ):

    rc = 0

    print( "Testing " + grid + " grid" )

    print("  Command: " + gridProgram + " -dims " + grid + " " + silenceTime )
    programOutput  = subprocess.run( 
        [ gridProgram, "-dims", grid, silenceTime ], 
        stdout=subprocess.PIPE,  
        universal_newlines=True 
    )

    outputFileName = resultsDir + grid + ".txt"
    try:
        with open( outputFileName, "w" ) as outputFile:
            outputFile.write( programOutput.stdout )
            outputFile.close()
            print( "  " + outputFileName + " written" )
    except IOError:
        print( "Unable to write to file " + outputFileName )
        sys.exit(-1)

    if ( programOutput.returncode != 0 ):
        rc = 1
        print( "  Test failed with exit code " + str(programOutput.returncode) )
    else:
        print( "  Test passed\n" )

    return rc

def testDataMgr( dataMgrType, dataMgr, makeBaseline=False ):
    print( "Testing " + dataMgrType + " with " + dataMgr )
    command = []
    if( silenceTime != "" ):
        command = [ dataMgrProgram, silenceTime, "-fileType", dataMgrType, dataMgr ]
    else:
        command = [ dataMgrProgram, "-fileType", dataMgrType, dataMgr ]
    print( "  Command: " + " ".join(command) )
    programOutput = subprocess.check_output( command )
    
    if ( makeBaseline ):
        outputFileName = resultsDir + dataMgrType + "_baseline.txt"
    else:
        outputFileName = resultsDir + dataMgrType + ".txt"

    try:
        with open( outputFileName, "w" ) as outputFile:
            outputFile = open( outputFileName, "w" )
            outputFile.write( programOutput.decode("utf-8") )
            outputFile.close()
            print( "  " + outputFileName + " written\n" )
    except IOError:
        print( "Unable to write to file " + outputFileName )
        sys.exit(-1)
    
    return outputFileName

def testDataMgrs( makeBaseline ):
    diff = open( dataMgrResultsFile, "w" )
    
    mismatches = 0

    for dataType, dataFile in dataMgrs.items():

        # If we're making a baseline file, generate it, and then skip comparisons 
        if ( makeBaseline ):
            resultsFile = testDataMgr( dataType, dataFile, makeBaseline )
            continue

        resultsFile = testDataMgr( dataType, dataFile )
        
        baselineFile = resultsDir + dataType + "_baseline.txt"
        baseline = open( baselineFile, "r" )
        results  = open( resultsFile, "r" )
        
        # Note - we are not reading the last line of these files, since it's the
        # runtime for the test ( hence the [:-1] specification )
        baselineLines = baseline.readlines()[:-1]
        resultsLines = results.readlines()[:-1]
        
        for line in difflib.unified_diff(resultsLines, baselineLines, resultsFile, baselineFile):
            diff.write( line )
            mismatches+=1

        baseline.close()
        results.close()
    diff.close()

    if ( makeBaseline == False ):    
        print( dataMgrResultsFile + " written" )
        print( "\n    DataMgr tests resulted in " + str(mismatches) + " mismatches\n" )
    else:
        print( "Baseline files have been generated.  Re-run smokeTests.py to run comparions.\n" )

    if ( mismatches > 0 ):
        return -1
    else:
        return 0
        
def main():
    makeBaseline = args['makeBaseline']

    rc = 0

    if ( makeBaseline == True ): 
        print( "    Warning: Some or all baseline files for running DataMgr tests were missing.  "
               "    These files are needed as comparisons for the results of the current series "
               "    of tests, versus a known working build (the baseline).\n"
               "       Generating baseline files in the results directory....\n"
        )

    if ( os.path.isdir( resultsDir ) == False ):    
        os.mkdir( resultsDir )

    for grid in gridSizes:
        if ( testGrid(grid) != 0):
            print ("  See artifact file " + grid + ".txt or " + resultsDir + grid + ".txt for mismatches")
            print ("  Failed assertions, if any, are shown above.\n" )
            rc = 1
 
    for dataType, dataFile in dataMgrs.items():
        baselineFile = resultsDir + dataType + "_baseline.txt"
        if ( os.path.isfile( baselineFile ) == False ):
            makeBaseline = True
        continue
 
    dataMgr = testDataMgrs( makeBaseline )
    if ( dataMgr != 0 ):
        print( "DataMgr tests failed.  Results are not identical to the baseline." )
        rc = 1
    else:
        print( "DataMgr tests passed" )

    sys.exit(rc)

if __name__ == "__main__":
    main()
