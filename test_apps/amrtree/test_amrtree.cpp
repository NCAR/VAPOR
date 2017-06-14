#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/AMRTree.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

const char *ProgName;

int main(int argc, char **argv) {

    OptionParser op;
    AMRTree *tree;

    MyBase::SetErrMsgFilePtr(stderr);
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
        cerr << "Usage: " << ProgName << " " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    size_t basedim[3] = {2, 2, 2};
    double min[3] = {4.0, 4.0, 4.0};
    double max[3] = {8.0, 8.0, 8.0};

    tree = new AMRTree(basedim, min, max);
    if (AMRTree::GetErrCode() != 0) {
        fprintf(stderr, "AMRTree() : %s\n", AMRTree::GetErrMsg());
        exit(1);
    }

    AMRTreeBranch::cid_t bidx, nidx;
    AMRTree::cid_t cellid = tree->RefineCell(0);
    tree->DecodeCellID(cellid, &bidx, &nidx);

    AMRTree::cid_t c0 = tree->RefineCell(cellid + 0);
    AMRTree::cid_t c1 = tree->RefineCell(cellid + 1);
    cerr << tree->RefineCell(c1) << endl;
    ;
    cerr << tree->RefineCell(c0) << endl;
    ;
    for (int i = 0; i < 30; i++) {
        cellid = tree->RefineCell(cellid + 3);
        tree->DecodeCellID(cellid, &bidx, &nidx);
        cerr << "child " << i << " " << nidx << endl;
    }

#ifdef DEAD
    AMRTree::cid_t c0, c0save;

    c0 = tree->RefineCell(cellid + 0);
    c0save = c0;
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    c0 = tree->RefineCell(cellid + 1);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    c0 = tree->RefineCell(cellid + 2);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    c0 = tree->RefineCell(cellid + 3);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    c0 = tree->RefineCell(cellid + 4);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    c0 = tree->RefineCell(cellid + 5);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    c0 = tree->RefineCell(cellid + 6);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "second child " << nidx << endl;
    //	c0 = tree->RefineCell(cellid+7);
    //	tree->DecodeCellID(c0, &bidx, &nidx);
    //	cerr << "second child " << nidx << endl;

    c0 = tree->RefineCell(c0save);
    tree->DecodeCellID(c0, &bidx, &nidx);
    cerr << "third child " << nidx << endl;

    cellid = tree->RefineCell(cellid);
    tree->DecodeCellID(cellid, &bidx, &nidx);
    cerr << "third child " << nidx << endl;
#endif

    tree->Write("tree.xml");

    AMRTree tree1("tree.xml");
    tree1.Write("tree1.xml");

    exit(0);
}
