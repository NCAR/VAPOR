#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/WASP.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    vector<string> dimnames;
    vector<int> dimlens;
    string wname;
    string ofile;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"dimnames", 1, "", "Colon delimited list of dimension names"},
    {"dimlens", 1, "", "Colon delimited list of dimension lengths"},
    {"wname", 1, "bior4.4", "Wavelet family used for compression "
                            "Valid values are bior1.1, bior1.3, "
                            "bior1.5, bior2.2, bior2.4 ,bior2.6, bior2.8, bior3.1, bior3.3, "
                            "bior3.5, bior3.7, bior3.9, bior4.4 intbior2.2"},
    {"ofile", 1, "test.nc", "Output file"},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"dimnames", Wasp::CvtToStrVec, &opt.dimnames, sizeof(opt.dimnames)},
    {"dimlens", Wasp::CvtToIntVec, &opt.dimlens, sizeof(opt.dimlens)},
    {"wname", Wasp::CvtToCPPStr, &opt.wname, sizeof(opt.wname)},
    {"ofile", Wasp::CvtToCPPStr, &opt.ofile, sizeof(opt.ofile)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

string ProgName;

vector<string> split(string s, string delim) {

    size_t pos = 0;
    vector<string> tokens;
    while ((pos = s.find(delim)) != std::string::npos) {
        tokens.push_back(s.substr(0, pos));
        s.erase(0, pos + delim.length());
    }
    if (!s.empty())
        tokens.push_back(s);

    return (tokens);
}

nc_type parsextype(string xtypestr) {
    if (xtypestr.compare("NC_BYTE") == 0)
        return (NC_BYTE);
    if (xtypestr.compare("NC_UBYTE") == 0)
        return (NC_UBYTE);
    if (xtypestr.compare("NC_CHAR") == 0)
        return (NC_CHAR);
    if (xtypestr.compare("NC_SHORT") == 0)
        return (NC_SHORT);
    if (xtypestr.compare("NC_USHORT") == 0)
        return (NC_USHORT);
    if (xtypestr.compare("NC_INT") == 0)
        return (NC_INT);
    if (xtypestr.compare("NC_UINT") == 0)
        return (NC_UINT);
    if (xtypestr.compare("NC_FLOAT") == 0)
        return (NC_FLOAT);
    if (xtypestr.compare("NC_INT64") == 0)
        return (NC_INT64);
    if (xtypestr.compare("NC_UINT64") == 0)
        return (NC_UINT64);
    if (xtypestr.compare("NC_DOUBLE") == 0)
        return (NC_DOUBLE);
    return (-1);
}

int main(int argc, char **argv) {

    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);
    //
    // Parse command line arguments
    //
    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] [name:type:count[:dimname]+]+" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(0);
    }

    if (opt.dimnames.size() != opt.dimlens.size()) {
        MyBase::SetErrMsg("dimension name and dimension length vector size mismatch");
        exit(1);
    }

    WASP wasp;
    vector<size_t> bs;
    bs.push_back(64);
    bs.push_back(64);
    bs.push_back(64);
    size_t chunksize = 1024 * 1024 * 4;

    vector<size_t> cratios3D;
    cratios3D.push_back(1);
    cratios3D.push_back(10);
    cratios3D.push_back(100);
    cratios3D.push_back(500);

    vector<size_t> cratios2D;
    cratios2D.push_back(1);
    cratios2D.push_back(5);
    cratios2D.push_back(25);
    cratios2D.push_back(100);

    vector<size_t> cratios1D;
    cratios1D.push_back(1);
    cratios1D.push_back(2);
    cratios1D.push_back(4);
    cratios1D.push_back(8);

    int rc = wasp.Create(
        opt.ofile, NC_64BIT_OFFSET, 0, chunksize, cratios3D.size());
    if (rc < 0)
        exit(1);

    int dummy;
    rc = wasp.SetFill(NC_NOFILL, dummy);
    if (rc < 0)
        exit(1);

    for (int i = 0; i < opt.dimnames.size(); i++) {
        int rc = wasp.DefDim(opt.dimnames[i], opt.dimlens[i]);
        if (rc < 0)
            exit(1);
    }

    argc--;
    argv++;
    for (int i = 0; i < argc; i++) {
        string s = argv[i];
        vector<string> vardef = split(s, ":");
        if (vardef.size() < 4) {
            MyBase::SetErrMsg("Invalid variable definition : %s", s.c_str());
            exit(1);
        }

        string name = vardef[0];
        vardef.erase(vardef.begin());

        string xtypestr = vardef[0];
        vardef.erase(vardef.begin());
        int xtype = parsextype(xtypestr);

        size_t ncdims;
        std::istringstream(vardef[0]) >> ncdims;
        vardef.erase(vardef.begin());

        // Everything else should be dimension name
        //
        vector<string> dimnames = vardef;

        if (ncdims == 0) {
            vector<size_t> cratios;

            int rc = wasp.DefVar(name, xtype, dimnames, "", bs, cratios);
            if (rc < 0)
                exit(1);

        } else {
            vector<size_t> cratios;
            if (ncdims == 3)
                cratios = cratios3D;
            if (ncdims == 2)
                cratios = cratios2D;
            if (ncdims == 1)
                cratios = cratios1D;
            int rc = wasp.DefVar(
                name, xtype, dimnames, opt.wname, bs, cratios);
            if (rc < 0)
                exit(1);
        }
    }

    wasp.EndDef();
    wasp.Close();
}
