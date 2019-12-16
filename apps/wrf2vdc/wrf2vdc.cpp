#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>
#include <vapor/FileUtils.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    int                     nthreads;
    int                     numts;
    std::vector<string>     vars;
    std::vector<string>     xvars;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"numts", 1, "-1", "Number of timesteps to be included in the VDC. Default (-1) includes all timesteps."},
                                         {"vars", 1, "",
                                          "Colon delimited list of variable names "
                                          "to be copied to the VDC"},
                                         {"xvars", 1, "",
                                          "Colon delimited list of variable names "
                                          "to exclude from copying the VDC"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)}, {"numts", Wasp::CvtToInt, &opt.numts, sizeof(opt.numts)},
                                        {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},          {"xvars", Wasp::CvtToStrVec, &opt.xvars, sizeof(opt.xvars)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},         {NULL}};

// Return a new vector containing elements of v1 with any elements from
// v2 removed
//
vector<string> remove_vector(vector<string> v1, vector<string> v2)
{
    vector<string> newvec;
    for (auto it = v1.begin(); it != v1.end(); ++it) {
        if (find(v2.begin(), v2.end(), *it) == v2.end()) { newvec.push_back(*it); }
    }
    return (newvec);
}

string ProgName;

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { return (1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { return (1); }

    if (argc < 3) {
        cerr << "Usage: " << ProgName << " wrffiles... master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (0);
    }

    argc--;
    argv++;
    vector<string> wrffiles;
    for (int i = 0; i < argc - 1; i++) wrffiles.push_back(argv[i]);
    string master = argv[argc - 1];

    VDCNetCDF vdc(opt.nthreads);

    size_t         chunksize = 1024 * 1024 * 4;
    vector<size_t> bs;
    int            rc = vdc.Initialize(master, vector<string>(), VDC::A, bs, chunksize);
    if (rc < 0) return (1);

    DCWRF dcwrf;
    rc = dcwrf.Initialize(wrffiles, vector<string>());
    if (rc < 0) { return (1); }

    vector<string> varnames = dcwrf.GetCoordVarNames();
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dcwrf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        VAssert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;
            int rc = vdc.CopyVar(dcwrf, ts, varnames[i], -1, -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                return (1);
            }
        }
    }

    if (opt.vars.size()) {
        varnames = opt.vars;
    } else {
        varnames = dcwrf.GetDataVarNames();
    }

    varnames = remove_vector(varnames, opt.xvars);

    int estatus = 0;
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dcwrf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        VAssert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;

            int rc = vdc.CopyVar(dcwrf, ts, varnames[i], -1, -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                estatus = 1;
            }
        }
    }

    return estatus;
}
