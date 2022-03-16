#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/FileUtils.h>


using namespace Wasp;
using namespace VAPoR;

struct {
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"year", 1, "2020", "year"},
                                         {"month", 1, "1", "month"},
                                         {"day", 1, "1", "day"},
                                         {"hour", 1, "1", "hour"},
                                         {"minute", 1, "1", "minute"},
                                         {"second", 1, "1", "second"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"year", Wasp::CvtToInt, &opt.year, sizeof(opt.year)},
                                        {"month", Wasp::CvtToInt, &opt.month, sizeof(opt.month)},
                                        {"day", Wasp::CvtToInt, &opt.day, sizeof(opt.day)},
                                        {"hour", Wasp::CvtToInt, &opt.hour, sizeof(opt.hour)},
                                        {"minute", Wasp::CvtToInt, &opt.minute, sizeof(opt.minute)},
                                        {"second", Wasp::CvtToInt, &opt.second, sizeof(opt.second)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {NULL}};


const char *ProgName;


int main(int argc, char **argv)
{
    OptionParser op;
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

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }


    UDUnits udunits;
    int rc = udunits.Initialize();
    if (rc < 0) return(1);

    double seconds_since_epoch = 0.0;
    seconds_since_epoch = udunits.EncodeTime(opt.year, opt.month, opt.day, opt.hour, opt.minute, opt.second);


    int year, month, day, hour, minute, second;
    udunits.DecodeTime(seconds_since_epoch, &year, &month, &day, &hour, &minute, &second);

    if (year != opt.year || month != opt.month || day != opt.day || hour != opt.hour || minute != opt.minute || second != opt.second) {
        cerr << "Time/dates disagree" << endl;
        cerr << "Input " << opt.year << "-";
        cerr << opt.month << "-";
        cerr << opt.day << "_";
        cerr << opt.hour << ":";
        cerr << opt.minute << ":";
        cerr << opt.second << endl;

        cerr << "Output " << year << "-";
        cerr << month << "-";
        cerr << day << "_";
        cerr << hour << ":";
        cerr << minute << ":";
        cerr << second << endl;

        return(1);
    }
    return(0);
}
  

