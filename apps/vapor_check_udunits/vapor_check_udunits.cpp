#include <iostream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/FileUtils.h>
#include <vapor/UDUnitsClass.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"quiet", 0, "", "Don't print anything, just exit with status"}, {"help", 0, "", "Print this message and exit"}, {NULL}};

OptionParser::Option_T get_options[] = {{"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)}, {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)}, {NULL}};

string ProgName;

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { return 1; }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { return 1; }

    if (argc != 1 || opt.help) {
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return 1;
    }

    string path = UDUnits::GetDatabasePath();
    if (!opt.quiet && !path.empty()) printf("VAPOR UDUnits database path: \"%s\"\n", path.c_str());

    UDUnits ud;
    int     rc = ud.Initialize();

    if (rc < 0) return 1;

    if (!opt.quiet) { printf("UDUnits passed\n"); }

    return 0;
}
