#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>
#include <vapor/DCCF.h>
#include <vapor/DCMPAS.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    int                     nthreads;
    int                     numts;
    int                     memsize;
    std::vector<string>     vars;
    OptionParser::Boolean_T datamgr;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"numts", 1, "-1", "Number of timesteps to be included in the VDC. Default (-1) includes all timesteps."},
                                         {
                                             "memsize",
                                             1,
                                             "2000",
                                             "Cache size in MBs (if -datamgr used)",
                                         },
                                         {"vars", 1, "",
                                          "Colon delimited list of 3D variable names (compressed) "
                                          "to be included in "
                                          "the VDC"},
                                         {"datamgr", 0, "", "Get data from second data source via DataMgr"},
                                         {"quiet", 0, "", "Don't print individual variable results"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},  {"numts", Wasp::CvtToInt, &opt.numts, sizeof(opt.numts)},
                                        {"memsize", Wasp::CvtToInt, &opt.memsize, sizeof(opt.memsize)},     {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},
                                        {"datamgr", Wasp::CvtToBoolean, &opt.datamgr, sizeof(opt.datamgr)}, {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},          {NULL}};

string ProgName;

DC *DCCreate(string ftype)
{
    if (ftype.compare("vdc") == 0) {
        return (new VDCNetCDF(opt.nthreads));
    } else if (ftype.compare("wrf") == 0) {
        return (new DCWRF());
    } else if (ftype.compare("cf") == 0) {
        return (new DCCF());
    } else if (ftype.compare("mpas") == 0) {
        return (new DCMPAS());
    } else {
        MyBase::SetErrMsg("Invalid data collection format : %s", ftype.c_str());
        return (NULL);
    }
}

float *Buffer1;
size_t BufSize1 = 0;
float *Buffer2;
size_t BufSize2 = 0;

void computeLMax(const float *buf1, const float *buf2, size_t nelements, double &lmax, double &min, double &max, bool hasMissing, float mv)
{
    lmax = 0.0;
    min = 0.0;
    max = 0.0;
    if (nelements < 1) return;

    size_t index = 0;
    while (hasMissing && buf1[index] == mv && index < nelements) index++;

    if (index >= nelements) return;

    min = max = buf1[index];

    for (; index < nelements; index++) {
        if (hasMissing && buf1[index] == mv) continue;

        if (min > buf1[index]) { min = buf1[index]; }
        if (max < buf1[index]) { max = buf1[index]; }

        double diff = fabs(buf1[index] - buf2[index]);
        if (diff > lmax) { lmax = diff; }
    }
}

int get_var(DC *dc, size_t ts, string varname, float *Buffer) { return (dc->GetVar(ts, varname, -1, -1, Buffer)); }

int get_var(DataMgr *data_mgr, size_t ts, string varname, float *Buffer)
{
    Grid *grid = data_mgr->GetVariable(ts, varname, -1, -1);
    if (!grid) return (-1);

    Grid::Iterator itr;
    Grid::Iterator enditr = grid->end();
    for (itr = grid->begin(); itr != enditr; ++itr, ++Buffer) { *Buffer = *itr; }

    delete grid;
    return (0);
}

template<class S, class T> bool compare(S *dc1, T *dc2, size_t nts, string varname, double &nlmax_all)
{
    vector<size_t> dims;
    int            rc = dc1->GetDimLens(varname, dims);
    if (rc < 0) return (false);

    vector<size_t> dims2;
    rc = dc2->GetDimLens(varname, dims2, -1);
    if (rc < 0) return (false);

    VAssert(dims == dims2);

    size_t nelements = 1;
    for (int i = 0; i < dims.size(); i++) { nelements *= dims[i]; }

    if (BufSize1 < nelements) {
        if (Buffer1) delete[] Buffer1;
        Buffer1 = new float[nelements];
        BufSize1 = nelements;
        if (Buffer2) delete[] Buffer2;
        Buffer2 = new float[nelements];
        BufSize2 = nelements;
    }

    nlmax_all = 0.0;
    for (int ts = 0; ts < nts; ts++) {
        rc = get_var(dc1, ts, varname, Buffer1);
        if (rc < 0) return (false);

        DC::DataVar datavar;
        bool        ok = dc1->GetDataVarInfo(varname, datavar);
        VAssert(ok);

        float mv = 0.0;
        bool  hasMissing = datavar.GetHasMissing();
        if (hasMissing) mv = datavar.GetMissingValue();

        rc = get_var(dc2, ts, varname, Buffer2);
        if (rc < 0) return (false);

        double lmax, min, max;
        computeLMax(Buffer1, Buffer2, nelements, lmax, min, max, hasMissing, mv);
        if ((max - min) != 0.0) { lmax /= (max - min); }
        if (lmax > nlmax_all) { nlmax_all = lmax; }
    }

    return (true);
}

template<class S, class T> int process(S *dc1, const vector<string> &files1, T *dc2, const vector<string> &files2)
{
    int rc = dc1->Initialize(files1, vector<string>());
    if (rc < 0) return (1);

    rc = dc2->Initialize(files2, vector<string>());
    if (rc < 0) return (1);

    vector<string> varnames;
    if (opt.vars.size()) {
        varnames = opt.vars;
    } else {
        varnames = dc1->GetDataVarNames();
    }

    double max_nlmax = 0;
    bool   success = true;
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dc1->GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        VAssert(nts >= 0);

        if (!opt.quiet) { cout << "Testing variable " << varnames[i] << endl; }

        double nlmax;
        bool   ok = compare(dc1, dc2, nts, varnames[i], nlmax);
        if (!ok) {
            cout << "failed!" << endl;
            success = false;
            break;
        }
        if (!opt.quiet) { cout << "	NLmax = " << nlmax << endl; }
        if (nlmax > max_nlmax) { max_nlmax = nlmax; }
    }
    cout << "Max NLmax = " << max_nlmax << endl;

    return success ? 0 : 1;
}

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { exit(1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { exit(1); }

    if (argc < 6 || opt.help) {
        cerr << "Usage: " << ProgName << " source_ftype secondary_ftype source_files... -- secondary_files... " << endl;
        cerr << "Valid file types: vdc, wrf, cf, mpas" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }

    argc--;
    argv++;

    string ftype1 = argv[0];
    argc--;
    argv++;

    string ftype2 = argv[0];
    argc--;
    argv++;

    vector<string> files1;
    string         sep("--");
    while (*argv && string(*argv) != sep) {
        files1.push_back(*argv);
        argv++;
    }
    if (*argv) argv++;

    vector<string> files2;
    while (*argv) {
        files2.push_back(*argv);
        argv++;
    }

    DC *dc1 = NULL;
    dc1 = DCCreate(ftype1);
    if (!dc1) return (1);

    if (opt.datamgr) {
        DataMgr *datamgr2 = new DataMgr(ftype2, opt.memsize, opt.nthreads);
        return (process(dc1, files1, datamgr2, files2));
    } else {
        DC *dc2 = DCCreate(ftype2);
        if (!dc2) return (1);

        return (process(dc1, files1, dc2, files2));
    }
}
