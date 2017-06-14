#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/ImpExp.h>

using namespace Wasp;
using namespace VAPoR;

int cvtToExtents(const char *from, void *to);

struct {
    int ts;
    char *varname;
    char *vdfpath;
    OptionParser::IntRange_T xregion;
    OptionParser::IntRange_T yregion;
    OptionParser::IntRange_T zregion;
    OptionParser::IntRange_T timeseg;
    OptionParser::Boolean_T *help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"ts", 1, "0", "Time step"},
    {"varname", 1, "var1", "Variable name"},
    {"vdfpath", 1, "test.vdf", "Path to vdf file"},
    {"xregion", 1, "0:511", "X dimension subregion bounds (min:max)"},
    {"yregion", 1, "0:511", "Y dimension subregion bounds (min:max)"},
    {"zregion", 1, "0:511", "Z dimension subregion bounds (min:max)"},
    {"timeseg", 1, "0:99", "Time segment range bounds (min:max)"},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"ts", Wasp::CvtToInt, &opt.ts, sizeof(opt.ts)},
    {"varname", Wasp::CvtToString, &opt.varname, sizeof(opt.varname)},
    {"vdfpath", Wasp::CvtToString, &opt.vdfpath, sizeof(opt.vdfpath)},
    {"xregion", Wasp::CvtToIntRange, &opt.xregion, sizeof(opt.xregion)},
    {"yregion", Wasp::CvtToIntRange, &opt.yregion, sizeof(opt.yregion)},
    {"zregion", Wasp::CvtToIntRange, &opt.zregion, sizeof(opt.zregion)},
    {"timeseg", Wasp::CvtToIntRange, &opt.timeseg, sizeof(opt.timeseg)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

const char *ProgName;

int main(int argc, char **argv) {

    OptionParser op;

    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << argv[0] << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << argv[0] << " : " << OptionParser::GetErrMsg();
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << argv[0] << " [options] filename" << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (argc != 1) {
        cerr << "Usage: " << argv[0] << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    ImpExp *impexp = new ImpExp();

    // First export everything
    //
    string path = opt.vdfpath;
    size_t ts = opt.ts;
    string varname = opt.varname;
    size_t min[3] = {opt.xregion.min, opt.yregion.min, opt.zregion.min};
    size_t max[3] = {opt.xregion.max, opt.yregion.max, opt.zregion.max};
    size_t timeseg[2] = {opt.timeseg.min, opt.timeseg.max};

    impexp->Export(path, ts, varname, min, max, timeseg);
    if (impexp->GetErrCode() != 0) {
        cerr << ProgName << " : " << impexp->GetErrMsg() << endl;
        exit(1);
    }

    delete impexp;

    // Now see if we can import it
    //
    string pathImp;
    size_t tsImp;
    string varnameImp;
    size_t minImp[3];
    size_t maxImp[3];
    size_t timesegImp[2];

    impexp = new ImpExp();

    impexp->Import(&pathImp, &tsImp, &varnameImp, minImp, maxImp, timesegImp);
    if (impexp->GetErrCode() != 0) {
        cerr << ProgName << " : " << impexp->GetErrMsg() << endl;
        exit(1);
    }

    cout << "Path = " << pathImp << endl;
    cout << "Time Step = " << tsImp << endl;
    cout << "Variable Name = " << varnameImp << endl;
    cout << "Min region extents = " << minImp[0] << " " << minImp[1] << " " << minImp[2] << endl;
    cout << "Max region extents = " << maxImp[0] << " " << maxImp[1] << " " << maxImp[2] << endl;
    cout << "Time segment region extents = " << timesegImp[0] << " " << timesegImp[1] << endl;

    exit(0);
}
