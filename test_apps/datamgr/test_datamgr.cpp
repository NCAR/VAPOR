#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    int                     nts;
    int                     ts0;
    int                     loop;
    int                     memsize;
    int                     level;
    int                     lod;
    int                     nthreads;
    string                  varname;
    string                  savefilebase;
    string                  ftype;
    std::vector<double>     minu;
    std::vector<double>     maxu;
    OptionParser::Boolean_T dump;
    OptionParser::Boolean_T tgetvalue;
    OptionParser::Boolean_T nogeoxform;
    OptionParser::Boolean_T novertxform;
    OptionParser::Boolean_T verbose;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nts", 1, "1", "Number of timesteps to process"},
                                         {"ts0", 1, "0", "First time step to process"},
                                         {"loop", 1, "1", "Number of loops to execute"},
                                         {"memsize", 1, "2000", "Cache size in MBs"},
                                         {"level", 1, "0", "Multiresution refinement level. Zero implies coarsest resolution"},
                                         {"lod", 1, "0", "Level of detail. Zero implies coarsest resolution"},
                                         {"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"varname", 1, "", "Name of variable"},
                                         {"savefilebase", 1, "", "Base path name to output file"},
                                         {"ftype", 1, "vdc", "data set type (vdc|wrf|cf|mpas)"},
                                         {"minu", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying domain min extents in user coordinates (X0:Y0:Z0)"},
                                         {"maxu", 1, "",
                                          "Colon delimited 3-element vector "
                                          "specifying domain max extents in user coordinates (X1:Y1:Z1)"},
                                         {"verbose", 0, "", "Verobse output"},
                                         {"tgetvalue", 0, "", "Apply Grid:;GetValue test"},
                                         {"dump", 0, "", "Dump variable coordinates and data"},
                                         {"nogeoxform", 0, "", "Do not apply geographic transform (projection to PCS"},
                                         {"novertxform", 0, "", "Do not apply to convert pressure, etc. to meters"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {"quiet", 0, "", "Operate quitely"},
                                         {"debug", 0, "", "Debug mode"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nts", Wasp::CvtToInt, &opt.nts, sizeof(opt.nts)},
                                        {"ts0", Wasp::CvtToInt, &opt.ts0, sizeof(opt.ts0)},
                                        {"loop", Wasp::CvtToInt, &opt.loop, sizeof(opt.loop)},
                                        {"memsize", Wasp::CvtToInt, &opt.memsize, sizeof(opt.memsize)},
                                        {"level", Wasp::CvtToInt, &opt.level, sizeof(opt.level)},
                                        {"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
                                        {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
                                        {"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
                                        {"savefilebase", Wasp::CvtToCPPStr, &opt.savefilebase, sizeof(opt.savefilebase)},
                                        {"ftype", Wasp::CvtToCPPStr, &opt.ftype, sizeof(opt.ftype)},
                                        {"minu", Wasp::CvtToDoubleVec, &opt.minu, sizeof(opt.minu)},
                                        {"maxu", Wasp::CvtToDoubleVec, &opt.maxu, sizeof(opt.maxu)},
                                        {"verbose", Wasp::CvtToBoolean, &opt.verbose, sizeof(opt.verbose)},
                                        {"dump", Wasp::CvtToBoolean, &opt.dump, sizeof(opt.dump)},
                                        {"tgetvalue", Wasp::CvtToBoolean, &opt.tgetvalue, sizeof(opt.tgetvalue)},
                                        {"nogeoxform", Wasp::CvtToBoolean, &opt.nogeoxform, sizeof(opt.nogeoxform)},
                                        {"novertxform", Wasp::CvtToBoolean, &opt.novertxform, sizeof(opt.novertxform)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
                                        {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
                                        {NULL}};

const char *ProgName;

void ErrMsgCBHandler(const char *msg, int) { cerr << ProgName << " : " << msg << endl; }
void print_info(DataMgr &datamgr, bool verbose)
{
    vector<string> dimnames;
    dimnames = datamgr.GetDimensionNames();
    cout << "Dimensions:" << endl;
    for (int i = 0; i < dimnames.size(); i++) {
        DC::Dimension dimension;
        datamgr.GetDimension(dimnames[i], dimension);
        cout << "\t" << dimension.GetName() << " = " << dimension.GetLength() << endl;
    }

    vector<string> meshnames;
    cout << "Meshes:" << endl;
    meshnames = datamgr.GetMeshNames();
    for (int i = 0; i < meshnames.size(); i++) {
        cout << "\t" << meshnames[i] << endl;
        if (verbose) {
            DC::Mesh mesh;
            datamgr.GetMesh(meshnames[i], mesh);
            cout << mesh;
        }
    }

    vector<string> vars;

    for (int d = 1; d < 4; d++) {
        cout << d << "D variables: ";
        vars = datamgr.GetDataVarNames(d);
        for (int i = 0; i < vars.size(); i++) {
            cout << vars[i] << endl;

            if (verbose) {
                DC::DataVar datavar;
                datamgr.GetDataVarInfo(vars[i], datavar);
                cout << datavar;
            }
        }
        cout << endl;
    }
    cout << endl << endl;
}

void test_node_iterator(const Grid *g, vector<double> minu, vector<double> maxu)
{
    cout << "Node Iterator Test ----->" << endl;

    Grid::ConstNodeIterator itr;
    Grid::ConstNodeIterator enditr = g->ConstNodeEnd();

    float t0 = GetTime();
    itr = g->ConstNodeBegin(minu, maxu);

    size_t count = 0;
    for (; itr != enditr; ++itr) { count++; }
    cout << "count: " << count << endl;
    cout << "time: " << GetTime() - t0 << endl;
    cout << endl;
}

void test_get_value(Grid *g)
{
    cout << "Get Value Test ----->" << endl;

    g->SetInterpolationOrder(1);

    Grid::ConstIterator itr = g->cbegin();
    Grid::ConstIterator enditr = g->cend();

    Grid::ConstCoordItr c_itr = g->ConstCoordBegin();
    Grid::ConstCoordItr c_enditr = g->ConstCoordEnd();

    const float epsilon = 0.000001;

    float  t0 = GetTime();
    size_t ecount = 0;
    for (; itr != enditr; ++itr, ++c_itr) {
        float v0 = *itr;

        float v1 = g->GetValue(*c_itr);

        if (v0 != v1) {
            if (v0 == 0.0) {
                if (abs(v1) > epsilon) {
                    ecount++;
                    v1 = g->GetValue(*c_itr);
                }
            } else {
                if (abs((v1 - v0) / v0) > epsilon) {
                    ecount++;
                    v1 = g->GetValue(*c_itr);
                }
            }
        }
    }
    cout << "error count: " << ecount << endl;
    cout << "time: " << GetTime() - t0 << endl;
    cout << endl;
}

void dump(const Grid *g)
{
    vector<size_t> dims = g->GetDimensions();
    vector<size_t> min(dims.size(), 0);
    vector<size_t> max;
    for (int i = 0; i < dims.size(); i++) { max.push_back(dims[i] - 1); }

    vector<size_t> index = min;
    vector<double> coord;

    while (index != max) {
        g->GetUserCoordinates(index, coord);
        float v = g->GetValueAtIndex(index);

        for (int i = 0; i < dims.size(); i++) { cout << coord[i] << " "; }
        cout << v << endl;

        if (index[0] == max[0]) { cout << endl; }

        index = IncrementCoords(min, max, index);
    }
}

void process(FILE *fp, DataMgr &datamgr, string vname, int loop, int ts)
{
    vector<double> timecoords;
    datamgr.GetTimeCoordinates(timecoords);

    if (!opt.savefilebase.empty()) {
        char   buf[4 + 1];
        string path(opt.savefilebase);
        path.append(".");
        sprintf(buf, "%4.4d", loop);
        path.append(buf);
        path.append(".");
        sprintf(buf, "%4.4d", ts);
        path.append(buf);

        fp = fopen(path.c_str(), "w");
        if (!fp) { cerr << "Can't open output file " << path << endl; }
    }
    if (!datamgr.VariableExists(ts, vname, opt.level, opt.lod)) {
        cerr << "Variable " << vname << " does not exist" << endl;
        return;
    }

    vector<double> minu, maxu;
    if (opt.minu.size()) {
        VAssert(opt.minu.size() == opt.maxu.size());
        minu = opt.minu;
        maxu = opt.maxu;
    } else {
        int rc = datamgr.GetVariableExtents(ts, vname, opt.level, minu, maxu);
        if (rc < 0) exit(1);
    }

    Grid *g;
    g = datamgr.GetVariable(ts, vname, opt.level, opt.lod, minu, maxu, false);

    if (!g) { exit(1); }

    if (opt.dump) { dump(g); }

    if (fp) {
        Grid::Iterator itr;
        Grid::Iterator enditr = g->end();
        float          v;
        for (itr = g->begin(); itr != enditr;) {
            v = *itr;
            fwrite(&v, sizeof(v), 1, fp);
            itr += 8;
        }
        fclose(fp);
    }

    g->GetUserExtents(minu, maxu);
    test_node_iterator(g, minu, maxu);

    if (opt.tgetvalue) { test_get_value(g); }

    vector<double> rvec;
    datamgr.GetDataRange(ts, vname, opt.level, opt.lod, 1, rvec);
    cout << "Data Range (stride==1): [" << rvec[0] << ", " << rvec[1] << "]" << endl;

    datamgr.GetDataRange(ts, vname, opt.level, opt.lod, 2, rvec);
    cout << "Data Range (stride==2): [" << rvec[0] << ", " << rvec[1] << "]" << endl;

    datamgr.GetDataRange(ts, vname, opt.level, opt.lod, 4, rvec);
    cout << "Data Range (stride==4): [" << rvec[0] << ", " << rvec[1] << "]" << endl;

    datamgr.GetDataRange(ts, vname, opt.level, opt.lod, 8, rvec);
    cout << "Data Range (stride==8): [" << rvec[0] << ", " << rvec[1] << "]" << endl;

    vector<size_t> dims = g->GetDimensions();
    cout << "Grid dimensions: [ ";
    for (int i = 0; i < dims.size(); i++) { cout << dims[i] << " "; }
    cout << "]" << endl;

    g->GetUserExtents(minu, maxu);
    cout << "Min user extents: [";
    for (int i = 0; i < minu.size(); i++) cout << minu[i] << " ";
    cout << "]" << endl;

    cout << "Max user extents: [";
    for (int i = 0; i < maxu.size(); i++) cout << maxu[i] << " ";
    cout << "]" << endl;

    cout << "Has missing data : " << g->HasMissingData() << endl;
    if (g->HasMissingData()) {
        cout << "Missing data value : " << g->GetMissingValue() << endl;
        Grid::Iterator itr;
        Grid::Iterator enditr = g->end();
        float          mv = g->GetMissingValue();
        int            count = 0;
        for (itr = g->begin(); itr != enditr; ++itr) {
            if (*itr == mv) count++;
        }
        cout << "Num missing values : " << count << endl;
    }
    cout << "Grid type: " << g->GetType() << endl;

    cout << setprecision(16) << "User time: " << timecoords[ts] << endl;
    cout << endl;
    delete g;
}

int main(int argc, char **argv)
{
    OptionParser op;
    double       timer = 0.0;
    string       s;

    ProgName = FileUtils::LegacyBasename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (opt.minu.size() && opt.minu.size() != opt.maxu.size()) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] metafiles " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.debug) { MyBase::SetDiagMsgFilePtr(stderr); }

    if (argc < 2) {
        cerr << "Usage: " << ProgName << " [options] vdcmaster " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    vector<string> files;
    for (int i = 1; i < argc; i++) { files.push_back(argv[i]); }

    vector<string> options;
    if (!opt.nogeoxform) { options.push_back("-project_to_pcs"); }
    if (!opt.novertxform) { options.push_back("-vertical_xform"); }
    DataMgr datamgr(opt.ftype, opt.memsize, opt.nthreads);
    int     rc = datamgr.Initialize(files, options);
    if (rc < 0) exit(1);

    print_info(datamgr, opt.verbose);

    string vname = opt.varname;
    if (vname.empty()) { return (0); }

    FILE *fp = NULL;

    cout << "Variable name : " << vname << endl;

    int nts = datamgr.GetNumTimeSteps(vname);

    for (int l = 0; l < opt.loop; l++) {
        cout << "Processing loop " << l << endl;

        for (int ts = opt.ts0; ts < opt.ts0 + opt.nts && ts < nts; ts++) {
            cout << "Processing time step " << ts << endl;

            process(fp, datamgr, vname, l, ts);
        }
    }

    if (!opt.quiet) { fprintf(stdout, "total process time : %f\n", timer); }

    exit(0);
}
