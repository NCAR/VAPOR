//
//      $Id$
//
//***********************************************************************
//                                                                       *
//                            Copyright (C)  2005                        *
//            University Corporation for Atmospheric Research            *
//                            All Rights Reserved                        *
//                                                                       *
//***********************************************************************/
//
//      File:		raw2vdf.cpp
//
//      Author:         John Clyne
//                      National Center for Atmospheric Research
//                      PO 3000, Boulder, Colorado
//
//      Date:           Tue Jun 14 15:01:13 MDT 2005
//
//      Description:	Read a file containing a raw data volume. Translate
//			and append the volume to an existing
//			Vapor Data Collection
//
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sstream>
#include <cerrno>
#include <cstdio>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/Version.h>
#ifdef WIN32
    #include "windows.h"
#endif

using namespace Wasp;

//
//	Command line argument stuff
//
struct opt_t {
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T _long;
    OptionParser::Boolean_T numeric;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"help", 0, "", "Print this message and exit"}, {"long", 0, "", "Print long form (version and date)"}, {"numeric", 0, "", "Print numeric form"}, {NULL}};

OptionParser::Option_T get_options[] = {
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)}, {"long", Wasp::CvtToBoolean, &opt._long, sizeof(opt._long)}, {"numeric", Wasp::CvtToBoolean, &opt.numeric, sizeof(opt.numeric)}, {NULL}};

const char *ProgName;

int main(int argc, char **argv)
{
    OptionParser op;

    //
    // Parse command line arguments
    //
    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] metafile datafile" << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (argc != 1) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    if (opt._long) {
        cout << "Vapor version " << Version::GetVersionString() << " (" << Version::GetDateString() << ")" << endl;
    } else if (opt.numeric) {
        cout << Version::GetMajor() << "." << Version::GetMinor() << "." << Version::GetMinorMinor() << endl;
    } else {
        cout << "vapor-" << Version::GetVersionString() << endl;
    }
}
