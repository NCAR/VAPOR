#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include <vapor/MetadataVDC.h>
#include <vapor/CFuncs.h>

using namespace Wasp;
using namespace VAPoR;

const char *ProgName;

int main(int argc, char **argv)
{
    ProgName = Basename(argv[0]);

    if (argc < 4) {
        cerr << "Usage: src1 src2 [src3...] dst" << argv[0] << endl;
        exit(1);
    }
    argv++;
    argc--;

    MetadataVDC *m = new MetadataVDC(*argv);
    if (MetadataVDC::GetErrCode() != 0) {
        cerr << "MetadataVDC::MetadataVDC() : " << MetadataVDC::GetErrMsg() << endl;
        exit(1);
    }
    argv++;
    argc--;

    while (argv[1]) {
        m->Merge(*argv);
        if (MetadataVDC::GetErrCode() != 0) {
            cerr << "MetadataVDC::Merge() : " << MetadataVDC::GetErrMsg() << endl;
            exit(1);
        }
        argv++;
        argc--;
    }

    m->Write(*argv, 0);
    if (MetadataVDC::GetErrCode() != 0) {
        cerr << "MetadataVDC::Write() : " << MetadataVDC::GetErrMsg() << endl;
        exit(1);
    }

    delete m;
    exit(0);
}
