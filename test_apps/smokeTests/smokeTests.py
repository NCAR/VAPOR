#! /usr/local/bin/python3
#! /usr/bin/python3

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
    '--makeBaseline', 
    nargs=1,
    type=str,
    default=False, 
    required=False,
    metavar='false',
    help='Boolean that makes these test results the baseline on which future'
    + ' tests will be compared.  If no baseline file exists, this will automatically '
    + ' be set to true.'
)
parser.add_argument( 
    '--testDataRoot', 
    nargs=1,
    type=str,
    default="/Users/pearse/Data/smokeTestData/", 
    required=False,
    metavar='/path/to/data',
    help='Directory where DataMgr test data is stored.'
)
parser.add_argument( 
    '--binaryRoot', 
    nargs=1,
    type=str,
    default="/Users/pearse/VAPOR_gridTests/build/bin", 
    required=False,
    metavar='/path/to/binaries',
    help='Directory where binary test programs (testGrid, testDataMgr) are stored.'
)
args = parser.parse_args()

#
#  Default directories and test data
#

gridSizes = [
    "1x1x1",
    "1x8x8",
    "8x1x8",
    "8x8x1",
    "7x7x7",
    "8x8x8"
]

resultsDir = "testResults/"

dataMgrs = {
    #"mpas" : (args.testDataRoot + "hist.mpas-o.0001-01-01_00.00.00.nc")
    "wrf"  : (args.testDataRoot + "wrfout_d02_2005-08-29_05"),
    "cf"   : (args.testDataRoot + "24Maycontrol.04000.000000.nc"),
    "vdc"  : (args.testDataRoot + "katrina_1timeStep.vdc"),
}

gridProgram        = args.binaryRoot + "/testGrid"
dataMgrProgram     = args.binaryRoot + "/testDataMgr"
gridResultsFile    = resultsDir + "gridResults.txt"
dataMgrResultsFile = resultsDir + "dataMgrResults.txt"

#
#  Tests
#

def testGrid( grid, makeBaseline=False ):
    print( "testing " + grid + " grid" )

    programOutput  = subprocess.check_output( [ gridProgram, " -dims ", grid ] )

    if ( makeBaseline ):
        outputFileName = resultsDir + grid + "_baseline.txt"
    else:
        outputFileName = resultsDir + grid + ".txt"
    
    outputFile = open( outputFileName, "w" )
    outputFile.write( programOutput.decode("utf-8") )
    outputFile.close()
    
    print( "  " + str(makeBaseline) + " " + outputFileName + " written\n" )

    return outputFileName

def testGrids( makeBaseline ):
    if ( os.path.isfile(gridResultsFile) ):
        print( gridResultsFile )
        diff = open( gridResultsFile, "w" )
    else:
        print( "got here" )
        makeBaseline = True
        os.mkdir( resultsDir )
        diff = open( gridResultsFile, "a" )

    mismatches = 0

    for grid in gridSizes:

        resultsFile = testGrid( grid, makeBaseline )
       
        # If we're making a baseline file, skip comparisons
        if ( makeBaseline ):
            continue
        
        baselineFile = grid + "_baseline.txt"
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
    
    print( "Summary file " + gridResultsFile + " written" )
    diff.close()

    print( "  Grid tests resulted in " + str(mismatches) + " mismatches\n" )
    if ( mismatches > 0 ):
        return -1
    else:
        return 0

def testDataMgr( dataMgrType, dataMgr, makeBaseline=False ):
    print( "Testing " + dataMgrType + " with " + dataMgr )
    
    command = [ dataMgrProgram, "-fileType", dataMgrType, dataMgr ]
    programOutput = subprocess.check_output( command )
    
    if ( makeBaseline ):
        outputFileName = resultsDir + dataMgrType + "_baseline.txt"
    else:
        outputFileName = resultsDir + dataMgrType + ".txt"

    outputFile = open( outputFileName, "w" )
    outputFile.write( programOutput.decode("utf-8") )
    outputFile.close()
    
    print( "  " + outputFileName + " written" )

    return outputFileName

def testDataMgrs( makeBaseline ):
    if ( os.path.isfile( dataMgrResultsFile ) ):
        diff = open( dataMgrResultsFile, "w" )
    else:
        makeBaseline = True
        diff = open( dataMgrResultsFile, "a" )
    
    mismatches = 0

    for dataType, dataFile in dataMgrs.items():
        #dataMgrFile = resultsDir + dataMgrFile

        # If we're making a baseline file, generate it, and then skip comparisons 
        if ( makeBaseline ):
            resultsFile = testDataMgr( dataType, dataFile )
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
    
    print( dataMgrResultsFile + " written" )
    diff.close()

    print( "    DataMgr tests resulted in " + str(mismatches) + " mismatches" )
    if ( mismatches > 0 ):
        return -1
    else:
        return 0
        
def main():
    print()
    makeBaseline = args.makeBaseline
    
    #if ( os.path.isfile(gridResultsFile) ):
     

    for grid in gridSizes:
        baselineFile = resultsDir + grid + "_baseline.txt"
        if ( os.path.isfile( baselineFile ) ):
            makeBaseline = True
            print( "Warning: No grid baseline file found for the " + grid
                + " grid.  Setting --makeBaseline to True." )

    for dataType, dataFile in dataMgrs.items():
        print( dataType + " " + dataFile )
        continue
    
    grid    = testGrids( makeBaseline )
    dataMgr = testDataMgrs( makeBaseline )

if __name__ == "__main__":
    main()
