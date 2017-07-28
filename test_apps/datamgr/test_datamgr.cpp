#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    int nts;
    int ts0;
    int loop;
    int memsize;
    int level;
    int lod;
    int nthreads;
    string varname;
    string savefilebase;
    string ftype;
    std::vector<float> extents;
    OptionParser::Boolean_T verbose;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"nts", 1, "10", "Number of timesteps to process"},
    {"ts0", 1, "0", "First time step to process"},
    {"loop", 1, "10", "Number of loops to execute"},
    {"memsize", 1, "2000", "Cache size in MBs"},
    {"level", 1, "0", "Multiresution refinement level. Zero implies coarsest resolution"},
    {"lod", 1, "0", "Level of detail. Zero implies coarsest resolution"},
    {"nthreads", 1, "0", "Specify number of execution threads "
                         "0 => use number of cores"},
    {"varname", 1, "", "Name of variable"},
    {"savefilebase", 1, "", "Base path name to output file"},
    {"ftype", 1, "vdc", "data set type (vdc|wrf)"},
    {"extents", 1, "", "Colon delimited 6-element vector "
                       "specifying domain extents in user coordinates (X0:Y0:Z0:X1:Y1:Z1)"},
    {"verbose", 0, "", "Verobse output"},
    {"help", 0, "", "Print this message and exit"},
    {"quiet", 0, "", "Operate quitely"},
    {"debug", 0, "", "Debug mode"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"nts", Wasp::CvtToInt, &opt.nts, sizeof(opt.nts)},
    {"ts0", Wasp::CvtToInt, &opt.ts0, sizeof(opt.ts0)},
    {"loop", Wasp::CvtToInt, &opt.loop, sizeof(opt.loop)},
    {"memsize", Wasp::CvtToInt, &opt.memsize, sizeof(opt.memsize)},
    {"level", Wasp::CvtToInt, &opt.level, sizeof(opt.level)},
    {"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
    {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
    {"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
    {"savefilebase", Wasp::CvtToCPPStr, &opt.savefilebase, sizeof(opt.savefilebase)},
    {"ftype", Wasp::CvtToCPPStr, &opt.ftype, sizeof(opt.ftype)},
    {"extents", Wasp::CvtToFloatVec, &opt.extents, sizeof(opt.extents)},
    {"verbose", Wasp::CvtToBoolean, &opt.verbose, sizeof(opt.verbose)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
    {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
    {NULL}};

const char *ProgName;

void ErrMsgCBHandler(const char *msg, int) {
    cerr << ProgName << " : " << msg << endl;
}
void print_info(DataMgr &datamgr, bool verbose) {

    vector<string> dimnames;
    dimnames = datamgr.GetDimensionNames();
    cout << "Dimensions:" << endl;
    for (int i = 0; i < dimnames.size(); i++) {
        DC::Dimension dimension;
        datamgr.GetDimension(dimnames[i], dimension);
        cout << "\t" << dimension.GetName() << " = " << dimension.GetLength()
             << endl;
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
        vars = datamgr.GetDataVarNames(d, true);
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
    cout << endl
         << endl;
}

int main(int argc, char **argv) {

    OptionParser op;
    double timer = 0.0;
    string s;

    ProgName = Basename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (opt.extents.size() && opt.extents.size() != 6) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] metafiles " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.debug) {
        MyBase::SetDiagMsgFilePtr(stderr);
    }

    if (argc < 2) {
        cerr << "Usage: " << ProgName << " [options] vdcmaster " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    vector<string> files;
    for (int i = 1; i < argc; i++) {
        files.push_back(argv[i]);
    }

    DataMgr datamgr(opt.ftype, opt.memsize, opt.nthreads);
    int rc = datamgr.Initialize(files);
    if (rc < 0)
        exit(1);

    print_info(datamgr, opt.verbose);

    string vname = opt.varname;
    if (vname.empty()) {
        return (0);
    }

    FILE *fp = NULL;

    cout << "Variable name : " << vname << endl;

    int nts = datamgr.GetNumTimeSteps(vname);
    vector<double> timecoords;
    datamgr.GetTimeCoordinates(timecoords);

    for (int l = 0; l < opt.loop; l++) {
        cout << "Processing loop " << l << endl;

        for (int ts = opt.ts0; ts < opt.ts0 + opt.nts && ts < nts; ts++) {
            cout << "Processing time step " << ts << endl;

            if (!opt.savefilebase.empty()) {
                char buf[4 + 1];
                string path(opt.savefilebase);
                path.append(".");
                sprintf(buf, "%4.4d", l);
                path.append(buf);
                path.append(".");
                sprintf(buf, "%4.4d", ts);
                path.append(buf);

                fp = fopen(path.c_str(), "w");
                if (!fp) {
                    cerr << "Can't open output file " << path << endl;
                }
            }
            if (!datamgr.VariableExists(ts, vname, opt.level, opt.lod)) {
                cerr << "Variable " << vname << " does not exist" << endl;
                break;
            }

            vector<double> min, max;
            if (opt.extents.size()) {
                assert(opt.extents.size() == 6);
                for (int i = 0; i < 3; i++) {
                    min.push_back(opt.extents[i]);
                    max.push_back(opt.extents[i + 3]);
                }
            } else {
                int rc = datamgr.GetVariableExtents(
                    ts, vname, opt.level, min, max);
                if (rc < 0)
                    exit(1);
            }

            StructuredGrid *rg;
            rg = datamgr.GetVariable(
                ts, vname, opt.level, opt.lod, min, max, false);

            if (!rg) {
                exit(1);
            }

            if (fp) {
                StructuredGrid::Iterator itr;
                float v;
                for (itr = rg->begin(); itr != rg->end(); ++itr) {
                    v = *itr;
                    fwrite(&v, sizeof(v), 1, fp);
                }
                fclose(fp);
            }

            float r[2];
            rg->GetRange(r);
            cout << "Data Range : [" << r[0] << ", " << r[1] << "]" << endl;

            size_t dims[3];
            rg->GetDimensions(dims);
            cout << "Grid dimensions: [" << dims[0] << ", " << dims[1] << ", " << dims[2] << "]" << endl;

            vector<double> minu, maxu;
            rg->GetUserExtents(minu, maxu);
            cout << "Min user extents: [";
            for (int i = 0; i < minu.size(); i++)
                cout << minu[i] << " ";
            cout << "]" << endl;

            cout << "Max user extents: [";
            for (int i = 0; i < maxu.size(); i++)
                cout << maxu[i] << " ";
            cout << "]" << endl;

            cout << "Has missing data : " << rg->HasMissingData() << endl;
            if (rg->HasMissingData()) {

                cout << "Missing data value : " << rg->GetMissingValue() << endl;
                StructuredGrid::Iterator itr;
                float mv = rg->GetMissingValue();
                int count = 0;
                for (itr = rg->begin(); itr != rg->end(); ++itr) {
                    if (*itr == mv)
                        count++;
                }
                cout << "Num missing values : " << count << endl;
            }

            cout << setprecision(16) << "User time: " << timecoords[ts] << endl;
            cout << endl;
        }
    }

    if (!opt.quiet) {

        fprintf(stdout, "total process time : %f\n", timer);
    }

    exit(0);
}
