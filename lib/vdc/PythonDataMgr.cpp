#include <vapor/VAssert.h>
#include <vapor/DCRAM.h>
#include <vapor/PythonDataMgr.h>

using namespace Wasp;
using namespace VAPoR;

PythonDataMgr::PythonDataMgr(string format, size_t mem_size, int nthreads)
: DataMgr(format, mem_size, nthreads)
{
    // TODO: Disable caching.
    // TODO: DataMgr will fail to load anything if the cache is too small rather than
    // TODO: just loading it and not caching the result.
    // Disable caching
    // Setting to 0 will just cause it to be rejected and fallback to default cache size.
    // _mem_size = 1;
}

PythonDataMgr::~PythonDataMgr() {}

void PythonDataMgr::AddRegularData(string name, const float *buf, vector<int> dimLens)
{
    auto dcr = GetDC();
    
    size_t totalSize = 1;
//    printf("%s(dims=%li)\n", __func__, dimLens.size());
    for (int i = 0; i < dimLens.size(); i++) {
        totalSize *= dimLens[i];
//        printf("\t dim[%i] = %i\n", i, dimLens[i]);
    }
//    printf("\t data = ");
    for (int i = 0; i < (totalSize < 5 ? totalSize : 5); i++) {
//        printf("%f, ", buf[i]);
    }
//    printf("\n");
    
    
    vector<DC::Dimension> dims;
    
    for (auto len : dimLens) {
        string genName = "__regDim_" + std::to_string(len);
        DC::Dimension dim;
        if (!GetDimension(genName, dim, -1)) {
            dim = DC::Dimension(genName, len);
            dcr->AddDimension(dim);
//            printf("Created new dimension \"%s\"\n", dim.GetName().c_str());
        }
        dims.push_back(dim);
    }
    
    vector<DC::CoordVar> coords;
    int currDim = 0; // (x,y,z,t) = (0,1,2,3)
    auto dimToStr = [](int d){ return "xyzt"[d]; };
    
    for (auto dim : dims) {
        string genName = "__regCoord_" + std::to_string(dim.GetLength()) + "_" + dimToStr(currDim);
        DC::CoordVar coord;
        if (!GetCoordVarInfo(genName, coord)) {
            coord = DC::CoordVar(genName, "m", DC::FLOAT, {false}, /*axis=x*/currDim, /*uniformHint=*/true, {dim.GetName()}, /*timeDim*/"");
            vector<float> cbuf;
            for (size_t i = 0; i < dim.GetLength(); i++)
                cbuf.push_back(i);
            dcr->AddCoordVar(coord, cbuf.data());
//            printf("Created new coordinate \"%s\"\n", coord.GetName().c_str());
        }
        coords.push_back(coord);
        currDim++;
    }
    
    
    vector<string> dimNames, coordNames;
    for (auto dim : dims)   dimNames.push_back(dim.GetName());
    for (auto crd : coords) coordNames.push_back(crd.GetName());
    
    string meshGenName = "__" + DC::Mesh::MakeMeshName(dimNames);
    DC::Mesh mesh;
    if (!GetMesh(meshGenName, mesh)) {
        mesh = DC::Mesh(meshGenName, dimNames, coordNames);
        dcr->AddMesh(mesh);
//        printf("Created new mesh \"%s\"\n", mesh.GetName().c_str());
    }
    
    
    
    vector<bool> periodic(dims.size(), false);
    auto v = DC::DataVar(name, "", DC::FLOAT, periodic, mesh.GetName(), /*timeCoordVar*/"", DC::Mesh::NODE);
    
    dcr->AddDataVar(v, buf);
    ClearCache(name);
}

DCRAM *PythonDataMgr::GetDC() const
{
    auto dcr = dynamic_cast<DCRAM*>(_dc);
    VAssert(dcr);
    return dcr;
}

void PythonDataMgr::ClearCache(string varname)
{
//    printf("%s(%s)\n", __func__, varname.c_str());
    _free_var(varname);
    _dataVarNamesCache.clear();
}
