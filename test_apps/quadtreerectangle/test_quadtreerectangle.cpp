#include <iostream>
#include "vapor/VAssert.h"

#include <vapor/FileUtils.h>
#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/QuadTreeRectangle.hpp>

using namespace std;

using namespace Wasp;
using namespace VAPoR;

struct {
    int                     n;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"n", 1, "1000", "Quad mesh X & Y dimensions"}, {"help", 0, "", "Print this message and exit"}, {NULL}};

OptionParser::Option_T get_options[] = {{"n", Wasp::CvtToInt, &opt.n, sizeof(opt.n)}, {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)}, {NULL}};

const char *ProgName;

void print_histo(const QuadTreeRectangle<float, size_t> &qtr)
{
    vector<size_t> payload_histo;
    vector<size_t> level_histo;
    qtr.GetStats(payload_histo, level_histo);
    cout << "Payload histo" << endl;
    for (int i = 0; i < payload_histo.size(); i++) { cout << "   " << i << " " << payload_histo[i] << endl; }

    cout << "Level histo" << endl;
    for (int i = 0; i < level_histo.size(); i++) { cout << "   " << i << " " << level_histo[i] << endl; }
    cout << endl;
}

void test_mesh()
{
    size_t n = opt.n;
    VAssert(n >= 2);

    QuadTreeRectangle<float, size_t> qtr(0.0, 0.0, 1.0, 1.0, n * n);

    float delta = 1.0 / (float)(n - 1);

    cout << "	Build" << endl;
    size_t index = 0;
    for (size_t j = 0; j < n - 1; j++) {
        for (size_t i = 0; i < n - 1; i++) {
            float left = (float)i * delta;
            float right = (float)i * delta + delta;
            float top = (float)j * delta;
            float bottom = (float)j * delta + delta;

            qtr.Insert(left, top, right, bottom, index);

            index++;
        }
    }
    cout << "	Search" << endl;

    size_t num_missed = 0;
    size_t num_wrong = 0;
    index = 0;
    for (size_t j = 0; j < n - 1; j++) {
        for (size_t i = 0; i < n - 1; i++) {
            float x = (float)i * delta + (delta * 0.5);
            ;
            float y = (float)j * delta + (delta * 0.5);
            ;

            vector<size_t> payloads;

            qtr.GetPayloadContained(x, y, payloads);
            if (payloads.size() < 1) {
                num_missed++;
                continue;
            }
            bool found = false;
            for (int ii = 0; ii < payloads.size(); ii++) {
                if (payloads[ii] == index) {
                    found = true;
                    break;
                }
            }
            if (!found) { num_wrong++; }

            index++;
        }
    }
    cout << "	Num missed : " << num_missed << endl;
    cout << "	Num wrong : " << num_wrong << endl;
    // cout << qtr;

    print_histo(qtr);
}

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

    QuadTreeRectangle<float, int> qtr;
    cout << qtr;

    qtr.Insert(0.0, 0.0, 1.0, 1.0, 100);
    qtr.Insert(0.0, 0.0, 0.2, 0.2, 101);
    qtr.Insert(0.9, 0.9, 1.0, 1.0, 102);
    cout << qtr;

    vector<int> payloads;

    qtr.GetPayloadContained(0.1, 0.1, payloads);

    cout << "quads intersecting 0.1, 0.1" << endl;
    for (int i = 0; i < payloads.size(); i++) { cout << payloads[i] << " "; }
    cout << endl;

    qtr.GetPayloadContained(0.9, 0.9, payloads);

    cout << "quads intersecting 0.9, 0.9" << endl;
    for (int i = 0; i < payloads.size(); i++) { cout << payloads[i] << " "; }
    cout << endl;

    test_mesh();

    return 0;
}
