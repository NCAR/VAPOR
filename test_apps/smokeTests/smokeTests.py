#! /usr/local/bin/python3
#! /usr/bin/python3

import subprocess
import difflib
import argparse

parser = argparse.ArgumentParser()
parser.add_argument( 
    '--makeBaseline', 
    default=False, 
    required=False 
)
parser.add_argument( 
    '--testDataRoot', 
    default="/Users/pearse/Data/smokeTestData/", 
    required=False 
)
parser.add_argument( 
    '--binaryRoot', 
    default="/Users/pearse/VAPOR_gridTests/build/bin", 
    required=False 
)
args = parser.parse_args()

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

#program = "/VAPOR/build/bin/testGrid -dims " + grid
#program = "/Users/pearse/VAPOR_gridTests/build/bin/testGrid"
gridProgram    = args.binaryRoot + "/testGrid"
dataMgrProgram = args.binaryRoot + "/testDataMgr"

def testGrid( grid ):
    programOutput  = subprocess.check_output( [ gridProgram, " -dims ", grid ] )
    outputFileName = grid + ".txt"
    outputFile     = open( outputFileName, "w" )

    outputFile.write( programOutput.decode("utf-8") )
    outputFile.close()
    
    print( outputFileName + " written" )

    return outputFileName

def testGrids():
    diffFile = "testResults/gridResults.txt"
    diff = open( diffFile, "w" )

    mismatches = 0

    for grid in gridSizes:
        gridFile = resultsDir + grid

        # If we're making a baseline file, generate it, and then skip tests 
        if ( args.makeBaseline):
            resultsFile = testGrid( gridFile + "_baseline" )
            continue
        
        resultsFile = testGrid( gridFile )
        
        baselineFile = gridFile + "_baseline.txt"
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
    
    print( "\n" + diffFile + " written" )
    diff.close()

    print( "   Grid tests resulted in " + str(mismatches) + " mismatches\n" )
    if ( mismatches > 0 ):
        return -1
        #exit(-1)
    else:
        return 0
        #exit(0)

def testDataMgr( dataMgrType, dataMgr ):
    command = [ dataMgrProgram, "-fileType", dataMgrType, dataMgr ]
    programOutput  = subprocess.check_output( command )
    
    if ( args.makeBaseline ):
        outputFileName = resultsDir + dataMgrType + "_baseline.txt"
    else:
        outputFileName = resultsDir + dataMgrType + ".txt"
    outputFile = open( outputFileName, "w" )
    outputFile.write( programOutput.decode("utf-8") )
    outputFile.close()
    
    print( outputFileName + " written" )

    return outputFileName

def testDataMgrs():
    diffFile = "testResults/dataMgrResults.txt"
    diff = open( diffFile, "w" )
    
    mismatches = 0

    for dataType, dataMgrFile in dataMgrs.items():
        #dataMgrFile = resultsDir + dataMgrFile

        # If we're making a baseline file, generate it, and then skip tests 
        if ( args.makeBaseline):
            resultsFile = testDataMgr( dataType, dataMgrFile )
            continue

        resultsFile = testDataMgr( dataType, dataMgrFile )
        
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
    
    print( diffFile + " written" )
    diff.close()

    print( "    DataMgr tests resulted in " + str(mismatches) + " mismatches" )
    if ( mismatches > 0 ):
        return -1
        #exit(-1)
    else:
        return 0
        #exit(0)
        
def main():
    print()
    grid    = testGrids()
    dataMgr = testDataMgrs()

if __name__ == "__main__":
    main()
