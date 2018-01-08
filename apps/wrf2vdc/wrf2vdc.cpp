#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    int                     nthreads;
    int                     numts;
    std::vector<string>     vars;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"numts", 1, "-1", "Number of timesteps to be included in the VDC. Default (-1) includes all timesteps."},
                                         {"vars", 1, "",
                                          "Colon delimited list of 3D variable names (compressed) "
                                          "to be included in "
                                          "the VDC"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
                                        {"numts", Wasp::CvtToInt, &opt.numts, sizeof(opt.numts)},
                                        {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {NULL}};

string ProgName;

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { exit(1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { exit(1); }

    if (argc < 3) {
        cerr << "Usage: " << ProgName << " wrffiles... master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(0);
    }

    argc--;
    argv++;
    vector<string> wrffiles;
    for (int i = 0; i < argc - 1; i++) wrffiles.push_back(argv[i]);
    string master = argv[argc - 1];

    VDCNetCDF vdc(opt.nthreads);

    size_t chunksize = 1024 * 1024 * 4;
    int    rc = vdc.Initialize(master, vector<string>(), VDC::A, chunksize);
    if (rc < 0) exit(1);

    DCWRF dcwrf;
    rc = dcwrf.Initialize(wrffiles, vector<string>());
    if (rc < 0) { exit(1); }

    vector<string> varnames = dcwrf.GetCoordVarNames();
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dcwrf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        assert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;
            int rc = vdc.CopyVar(dcwrf, ts, varnames[i], -1, -1);
            if (rc < 0) exit(1);
        }
    }

    if (opt.vars.size()) {
        varnames = opt.vars;
    } else {
        varnames = dcwrf.GetDataVarNames();
    }
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dcwrf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        assert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;

            int rc = vdc.CopyVar(dcwrf, ts, varnames[i], -1, -1);
            if (rc < 0) exit(1);
        }
    }

    return (0);
}
