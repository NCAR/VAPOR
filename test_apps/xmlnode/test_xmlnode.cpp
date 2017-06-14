#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/XmlNode.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    string                  ifile;
    OptionParser::Boolean_T help;
    OptionParser::Boolean_T quiet;
    OptionParser::Boolean_T debug;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"ifile", 1, "", "Construct Xml tree from a file"}, {"help", 0, "", "Print this message and exit"}, {"quiet", 0, "", "Operate quitely"}, {"debug", 0, "", "Debug mode"}, {NULL}};

OptionParser::Option_T get_options[] = {{"ifile", Wasp::CvtToCPPStr, &opt.ifile, sizeof(opt.ifile)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {"quiet", Wasp::CvtToBoolean, &opt.quiet, sizeof(opt.quiet)},
                                        {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
                                        {NULL}};

const char *ProgName;

int main(int argc, char **argv)
{
    OptionParser op;
    double       timer = 0.0;
    string       s;

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

    if (opt.help) {
        cerr << "Usage: " << ProgName << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.debug) { MyBase::SetDiagMsgFilePtr(stderr); }

    if (argc != 1) {
        cerr << "Usage: " << ProgName << " [options] " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    XmlNode *parent = new XmlNode("parent");
    parent->SetElementLong("long_data1", 1);
    parent->SetElementLong("long_data2", 2);
    parent->SetElementString("string_data", "my string");
    parent->SetElementDouble("double_data1", 3.0);

    XmlNode *child1 = parent->NewChild("child1");
    child1->SetElementLong("long_data1", 4);
    child1->SetElementLong("long_data2", 5);
    child1->SetElementString("string_data", "my string");
    child1->SetElementDouble("double_data1", 6.0);
    assert(child1->GetParent() == parent);

    XmlNode *child2 = parent->NewChild("child2");
    child2->SetElementLong("long_data1", 7);
    child2->SetElementLong("long_data2", 8);
    child2->SetElementString("string_data", "my string");
    child2->SetElementDouble("double_data1", 9.0);
    assert(child2->GetParent() == parent);

    XmlNode *child3 = child2->NewChild("child3");
    assert(child3 != NULL);
    assert(child3->GetParent() == child2);

    child3->SetElementLong("long_data1", 10);
    child3->SetElementLong("long_data2", 11);
    child3->SetElementString("string_data", "my string");
    child3->SetElementDouble("double_data1", 12.0);

    XmlNode *parent2 = new XmlNode(*parent);

    if (*parent2 == *parent) {
        cout << "parent == parent 2" << endl;
    } else {
        cout << "parent != parent 2" << endl;
    }

    child3->SetElementDouble("double_data1", 99.0);

    if (*parent2 == *parent) {
        cout << "parent == parent 2" << endl;
    } else {
        cout << "parent != parent 2" << endl;
    }

    cout << "Allocated note count before delete : " << XmlNode::GetAllocatedNodes().size() << endl;

    delete parent;
    delete parent2;

    cout << "Allocated note count after delete : " << XmlNode::GetAllocatedNodes().size() << endl;

    // cout << "parent 1 " << endl << parent;
    // cout << "parent 2 " << endl << *parent2;

    if (!opt.ifile.empty()) {
        XmlNode   parent3;
        XmlParser parser;
        int       rc = parser.LoadFromFile(&parent3, opt.ifile);
        if (rc < 0) return 1;
        cout << "parent 3 " << endl << parent3;
    }
}
