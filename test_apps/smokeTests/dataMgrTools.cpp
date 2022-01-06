#include <sstream>

#include "gridTools.h"
#include "dataMgrTools.h"

void PrintDimensions(const VAPoR::DataMgr &dataMgr)
{
    vector<string> dimnames;
    dimnames = dataMgr.GetDimensionNames();
    cout << endl << "Dimensions:" << endl;
    for (int i = 0; i < dimnames.size(); i++) {
        VAPoR::DC::Dimension dimension;
        dataMgr.GetDimension(dimnames[i], dimension, -1);
        cout << "    " << dimension.GetName() << " = " << dimension.GetLength() << endl;
        cout << "    Time Varying: " << dimension.IsTimeVarying() << endl;
    }
    cout << endl;
}

void PrintMeshes(const VAPoR::DataMgr &dataMgr, bool verbose)
{
    vector<string> meshnames;
    cout << "Meshes:" << endl;
    meshnames = dataMgr.GetMeshNames();
    for (int i = 0; i < meshnames.size(); i++) {
        cout << "    " << meshnames[i] << endl;
        if (verbose) {
            VAPoR::DC::Mesh mesh;
            dataMgr.GetMesh(meshnames[i], mesh);
            cout << mesh;
        }
    }
    cout << endl;
}

void PrintCoordVariables(const VAPoR::DataMgr &dataMgr)
{
    cout << "Projection String:  " << dataMgr.GetMapProjection() << endl << endl;
    cout << "Coordinate Variables:" << endl;
    std::vector<std::string> coordVars = dataMgr.GetCoordVarNames();
    for (int i = 0; i < coordVars.size(); i++) { cout << "    " << coordVars[i] << endl; }
    cout << endl;
}

void PrintTimeCoordinates(const VAPoR::DataMgr &dataMgr)
{
    std::vector<double> timeCoords = dataMgr.GetTimeCoordinates();
    cout << "Time Coordinates:" << endl;
    auto oldPrecision = std::cout.precision(10);
    for (int i = 0; i < timeCoords.size(); i++) { cout << "    " << timeCoords[i] << endl; }
    std::cout.precision(oldPrecision);
    cout << endl;
    cout << "Time coordinate variable name: ";
    cout << dataMgr.GetTimeCoordVarName() << endl;
    cout << "Number of time steps: ";
    cout << dataMgr.GetNumTimeSteps() << endl;
}

void PrintCompressionInfo(const VAPoR::DataMgr &dataMgr, const std::string &varname)
{
    cout << "Compression Info for variable " << varname << ":" << endl;

    cout << "    Refinement Levels:  " << dataMgr.GetNumRefLevels(varname) << endl;

    std::stringstream   ss;
    std::vector<size_t> cRatios = dataMgr.GetCRatios(varname);
    for (size_t i = 0; i < cRatios.size(); i++) {
        if (i != 0) ss << " ";
        ss << cRatios[i];
    }
    std::string s = ss.str();
    cout << "    Compression Ratios: " << s << endl << endl;
}

void PrintVariables(const VAPoR::DataMgr &dataMgr, bool verbose, bool testVars)
{
    vector<string> vars;

    for (int d = 1; d < 4; d++) {
        vars = dataMgr.GetDataVarNames(d);
        if (!vars.size()) continue;
        cout << d << "D variables: " << endl;
        ;
        for (int i = 0; i < vars.size(); i++) {
            cout << "    " << vars[i] << endl;
            if (verbose) {
                VAPoR::DC::DataVar datavar;
                dataMgr.GetDataVarInfo(vars[i], datavar);
                cout << datavar;
            }
        }
        cout << endl;
    }
}

void TestVariables(VAPoR::DataMgr &dataMgr, bool silenceTime)
{
    vector<string> vars;
    for (int d = 1; d < 4; d++) {
        vars = dataMgr.GetDataVarNames(d);
        if (vars.size()) {
            std::string varName = vars[0];
            PrintCompressionInfo(dataMgr, varName);
            VAPoR::CoordType minExt, maxExt;
            dataMgr.GetVariableExtents(0, varName, -1, -1, minExt, maxExt);

            // Reduce extents to test, while maintaining the same origin
            for (int i = 0; i < minExt.size(); i++) {
                double center = (maxExt[i] + minExt[i]) / 2.0;
                double width = (maxExt[i] - minExt[i]) / 32.0;
                minExt[i] = center - (width / 2.0);
                maxExt[i] = center + (width / 2.0);
            }

            VAPoR::Grid *grid = dataMgr.GetVariable(0, varName, -1, -1, minExt, maxExt);
            if (!grid) {
                cerr << "Failed to read variable " << varName << endl;
                exit(1);
            }
            double rms;
            size_t numMissingValues;
            size_t disagreements;
            CompareIndexToCoords(grid, rms, numMissingValues, disagreements);
            cout << "Grid test for " << d << "D variable " << varName << ":" << endl;
            cout << "    # Dimensions:       " << dataMgr.GetNumDimensions(varName) << endl;

            std::vector<size_t> dimLens;
            dataMgr.GetDimLens(varName, dimLens, 0);
            std::stringstream ss;
            for (size_t i = 0; i < dimLens.size(); i++) {
                if (i != 0) ss << " ";
                ss << dimLens[i];
            }
            std::string s = ss.str();
            cout << "    Dimension Lengths:  " << s << endl;
            cout << "    Topology Dimension: " << dataMgr.GetVarTopologyDim(varName) << endl;
            cout << endl;
            PrintStats(rms, numMissingValues, disagreements, 0.0, silenceTime);
        }
    }
}

int TestDataMgr(const std::string &fileType, size_t memsize, size_t nthreads, const std::vector<std::string> &files, const std::vector<std::string> &options, bool silenceTime)
{
    VAPoR::DataMgr dataMgr(fileType, memsize, nthreads);
    int            rc = dataMgr.Initialize(files, options);
    if (rc < 0) {
        cerr << "Failed to intialize WRF DataMGR" << endl;
        return -1;
    }

    PrintDimensions(dataMgr);
    PrintMeshes(dataMgr);
    PrintVariables(dataMgr);
    TestVariables(dataMgr, silenceTime);
    PrintCoordVariables(dataMgr);
    PrintTimeCoordinates(dataMgr);

    return 0;
}
