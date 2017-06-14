#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cassert>
#include <cstdio>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/AMRTree.h>
#include <vapor/AMRData.h>
#include <vapor/Metadata.h>
#include <vapor/WaveletBlock3DBufWriter.h>

#include "flashhdf5.h"

using namespace Wasp;
using namespace VAPoR;

struct {
    char *savefile;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"savefile", 1, "", "Path to output file"},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"save", Wasp::CvtToString, &opt.savefile, sizeof(opt.savefile)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

const char *ProgName;

int main(int argc, char **argv) {

    OptionParser op;
    int refinelevel = 3;

    MyBase::SetDiagMsgFilePtr(stderr);
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

    if (argc != 4) {
        cerr << "Usage : " << ProgName << " srcfile vdfout_file amrout_file " << endl;
        exit(1);
    }

    FlashHDFFile hdffile(argv[1]);

    assert(hdffile.GetNumberOfDimensions() == 3);

    int total_blocks = hdffile.GetNumberOfBlocks();

    int *gids = new int[total_blocks * 15];
    assert(gids != NULL);

    hdffile.GetGlobalIds(gids);

    float *bboxes = new float[total_blocks * 3 * 2];
    assert(bboxes != NULL);

    hdffile.GetBoundingBoxes(bboxes);

    int *refine_levels = new int[total_blocks];
    assert(refine_levels != NULL);

    hdffile.GetRefineLevels(refine_levels);

    AMRTree tree(
        (int(*)[15])gids, (float(*)[3][2])bboxes,
        refine_levels, total_blocks);
    if (AMRTree::GetErrCode()) {
        cerr << ProgName << " : " << AMRTree::GetErrMsg() << endl;
        exit(1);
    }

    int dim[3];
    hdffile.GetCellDimensions(dim);

    float *variable = new float[total_blocks * dim[0] * dim[1] * dim[2]];
    assert(variable != NULL);

    hdffile.GetScalarVariable("vely", 0, total_blocks, variable);

    const size_t celldim[3] = {dim[0], dim[1], dim[2]};
    AMRData amrdata(&tree, celldim, (int(*)[15])gids, variable, total_blocks, -1);
    if (AMRData::GetErrCode()) {
        cerr << ProgName << " : " << AMRData::GetErrMsg() << endl;
        exit(1);
    }

    const size_t *bdim = tree.GetBaseDim();
    const size_t regmin[3] = {0, 0, 0};
    size_t regmax[3];

    size_t size;
    for (int i = 0; i < 3; i++)
        regmax[i] = (bdim[i] << refinelevel) - 1;

    size_t vdim[3]; // dimension in voxels of resampled grid

    for (int i = 0; i < 3; i++)
        vdim[i] = (regmax[i] - regmin[i] + 1) * celldim[i];

    size = vdim[0] * vdim[1] * vdim[2];

    float *grid = new float[size];
    assert(grid != NULL);

    for (int i = 0; i < size; i++) {
        grid[i] = 1.0;
    }

    amrdata.ReGrid(regmin, regmax, refinelevel, grid);
    if (AMRData::GetErrCode()) {
        cerr << ProgName << " : " << AMRData::GetErrMsg() << endl;
        exit(1);
    }

    if (strlen(opt.savefile)) {
        FILE *fp;

        fp = fopen(opt.savefile, "w");
        if (!fp) {
            cerr << "Can't open output file " << opt.savefile << endl;
        }

        fwrite(grid, sizeof(grid[0]), size, fp);
        fclose(fp);
    }

#ifdef DEAD

    for (int i = 0; i < 3; i++)
        vdim[i] = vdim[i] * 2;

    float *buf = new float[vdim[0] * vdim[1] * vdim[2]];

    int xx, yy, zz;
    int srcindex, dstindex;
    float cmin = grid[0];
    float cmax = grid[0];
    for (int z = 0; z < vdim[2]; z++) {
        for (int y = 0; y < vdim[1]; y++) {
            for (int x = 0; x < vdim[0]; x++) {

                xx = (unsigned int)x >> 1;
                yy = (unsigned int)y >> 1;
                zz = (unsigned int)z >> 1;

                srcindex = (zz * vdim[1] * vdim[0] / 4) + (yy * vdim[0] / 2) + xx;
                dstindex = (z * vdim[1] * vdim[0]) + (y * vdim[0]) + x;

                buf[dstindex] = grid[srcindex];

                if (grid[srcindex] < cmin)
                    cmin = grid[srcindex];
                if (grid[srcindex] > cmax)
                    cmax = grid[srcindex];
            }
        }
    }
    cerr << "Grid min max " << cmin << " " << cmax << endl;

    grid = buf;

#endif

    size_t bs[3] = {32, 32, 32};
    Metadata metadata(vdim, 3, bs);
    if (Metadata::GetErrCode()) {
        cerr << ProgName << " : " << Metadata::GetErrMsg() << endl;
        exit(1);
    }

    if (metadata.Write(argv[2]) < 0) {
        cerr << ProgName << " : " << Metadata::GetErrMsg() << endl;
        exit(1);
    }

    WaveletBlock3DBufWriter bufwriter(argv[2]);
    if (bufwriter.GetErrCode()) {
        cerr << ProgName << " : " << bufwriter.GetErrMsg() << endl;
        exit(1);
    }

    bufwriter.OpenVariableWrite(0, "var1");
    if (bufwriter.GetErrCode()) {
        cerr << ProgName << " : " << bufwriter.GetErrMsg() << endl;
        exit(1);
    }

    for (int z = 0; z < vdim[2]; z++) {
        bufwriter.WriteSlice(&grid[z * vdim[1] * vdim[0]]);
    }
    if (bufwriter.GetErrCode()) {
        cerr << ProgName << " : " << bufwriter.GetErrMsg() << endl;
        exit(1);
    }

    bufwriter.CloseVariable();

    amrdata.WriteNCDF(argv[3]);
    if (amrdata.GetErrCode()) {
        cerr << ProgName << " : " << amrdata.GetErrMsg() << endl;
        exit(1);
    }

    exit(0);
}
