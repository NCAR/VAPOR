#include <string>
#include <vector>
#include <map>
#include "vapor/VDC.h"
#include "vapor/VDCNetCDF.h"
#include "vapor/VDC_c.h"
#include "vapor/MyBase.h"

// #define VDC_DEBUG
#ifdef VDC_DEBUG
#define VDC_DEBUG_printf(...) fprintf(stderr, __VA_ARGS__)
#define VDC_DEBUG_printff(...)                                      \
    {                                                               \
        fprintf(stderr, "[%s:%i]%s", __FILE__, __LINE__, __func__); \
        fprintf(stderr, __VA_ARGS__);                               \
    }
#define VDC_DEBUG_called() VDC_DEBUG_printff("()\n")
#include <signal.h>
#define VDC_DEBUG_break() raise(SIGINT)
#else
#define VDC_DEBUG_printf(...)
#define VDC_DEBUG_printff(...)
#define VDC_DEBUG_called()
#define VDC_DEBUG_break()
#endif

using std::string;
using std::vector;

void _stringToCString(const string s, char **str);
void _stringVectorToCStringArray(const vector<string> v, char ***str, int *count);
void _longVectorToCArray(const vector<long> v, long **data, int *count);
void _boolVectorToCArray(const vector<bool> v, long **data, int *count);
void _doubleVectorToCArray(const vector<double> v, double **data, int *count);
void _size_tVectorToCArray(const vector<size_t> v, size_t **data, int *count);
int _XTypeToInt(const VDC::XType type);
VDC::XType _IntToXType(int type);
const char *_XTypeToString(const VDC::XType type);
const char *_boolToStr(const int b);
const char *_AxisToStr(const int a);

vector<string> _strArrayToStringVector(const char **a, size_t count);
string _stringVectorToString(const vector<string> v);
vector<size_t> _size_tArrayToSize_tVector(const size_t *a, size_t count);
string _size_tVectorToString(const vector<size_t> v);

// ########################
// #    VDC::Dimension    #
// ########################

VDCDimension *VDCDimension_new() { return new VDCDimension(); }
void VDCDimension_delete(VDCDimension *p) { delete p; }
void VDCDimension_GetName(const VDCDimension *p, char **name) { _stringToCString(p->GetName(), name); }
size_t VDCDimension_GetLength(const VDCDimension *p) { return p->GetLength(); }
int VDCDimension_IsTimeVarying(const VDCDimension *p) { return p->IsTimeVarying(); }

// ########################
// #     VDC::BaseVar     #
// ########################

VDCBaseVar *VDCBaseVar_new() { return new VDCBaseVar(); }
void VDCBaseVar_delete(VDCBaseVar *p) { delete p; }
void VDCBaseVar_GetName(const VDCBaseVar *p, char **name) { _stringToCString(p->GetName(), name); }
void VDCBaseVar_GetUnits(const VDCBaseVar *p, char **units) { _stringToCString(p->GetUnits(), units); }
int VDCBaseVar_GetXType(const VDCBaseVar *p) { return _XTypeToInt(p->GetXType()); }
void VDCBaseVar_GetWName(const VDCBaseVar *p, char **name) { _stringToCString(p->GetWName(), name); }
void VDCBaseVar_GetCRatios(const VDCBaseVar *p, size_t **ratios, int *count) { _size_tVectorToCArray(p->GetCRatios(), ratios, count); }
void VDCBaseVar_GetBS(const VDCBaseVar *p, size_t **bs, int *count) { _size_tVectorToCArray(p->GetBS(), bs, count); }
void VDCBaseVar_GetPeriodic(const VDCBaseVar *p, long **periodic, int *count) { _boolVectorToCArray(p->GetPeriodic(), periodic, count); }
void VDCBaseVar_GetAttributeNames(const VDCBaseVar *p, char ***names, int *count) {
    const std::map<string, VDC::Attribute> attributes = p->GetAttributes();
    vector<string> names_v;
    std::map<string, VDC::Attribute>::const_iterator it;
    for (it = attributes.begin(); it != attributes.end(); it++)
        names_v.push_back(it->first);
    _stringVectorToCStringArray(names_v, names, count);
}
int VDCBaseVar_IsCompressed(const VDCBaseVar *p) { return p->IsCompressed(); }

// ########################
// #     VDC::AuxVar     #
// ########################

VDCAuxVar *VDCAuxVar_new() { return new VDCAuxVar(); }
void VDCAuxVar_delete(VDCAuxVar *p) { delete p; }
void VDCAuxVar_GetDimNames(const VDCAuxVar *p, char ***names, int *count) { _stringVectorToCStringArray(p->GetDimNames(), names, count); }

// ########################
// #     VDC::DataVar     #
// ########################

VDCDataVar *VDCDataVar_new() { return new VDCDataVar(); }
void VDCDataVar_delete(VDCDataVar *p) { delete p; }
void VDCDataVar_GetMeshName(const VDCDataVar *p, char **name) { _stringToCString(p->GetMeshName(), name); }
void VDCDataVar_GetTimeCoordVar(const VDCDataVar *p, char **name) { _stringToCString(p->GetTimeCoordVar(), name); }
void VDCDataVar_GetMaskvar(const VDCDataVar *p, char **name) { _stringToCString(p->GetMaskvar(), name); }
int VDCDataVar_GetHasMissing(const VDCDataVar *p) { return p->GetHasMissing(); }
double VDCDataVar_GetMissingValue(const VDCDataVar *p) { return p->GetMissingValue(); }

// ########################
// #     VDC::CoordVar    #
// ########################

VDCCoordVar *VDCCoordVar_new() { return new VDCCoordVar(); }
void VDCCoordVar_delete(VDCCoordVar *p) { delete p; }
void VDCCoordVar_GetDimNames(const VDCCoordVar *p, char ***names, int *count) { _stringVectorToCStringArray(p->GetDimNames(), names, count); }
void VDCCoordVar_GetTimeDimName(const VDCCoordVar *p, char **name) { _stringToCString(p->GetTimeDimName(), name); }
int VDCCoordVar_GetAxis(const VDCCoordVar *p) { return p->GetAxis(); }
int VDCCoordVar_GetUniform(const VDCCoordVar *p) { return p->GetUniform(); }

// ########################
// #         VDC          #
// ########################

VDC *VDC_new() {
    VDC_DEBUG_called();
    return new VAPoR::VDCNetCDF();
}

void VDC_delete(VDC *p) {
    VDC_DEBUG_called();
    delete p;
}

int VDC_Initialize(VDC *p, const char *path, int mode) {
    VDC_DEBUG_called();
    VDC::AccessMode am = VDC::R;
    if (mode == VDC_AccessMode_R)
        am = VDC::R;
    else if (mode == VDC_AccessMode_W)
        am = VDC::W;
    else if (mode == VDC_AccessMode_A)
        am = VDC::A;

    VAPoR::VDCNetCDF *pnc = (VAPoR::VDCNetCDF *)p;
    int ret = pnc->Initialize(string(path), am, 0);
    // pnc->SetFill(0x100); // Required to disable set_fill
    return ret;
}

int VDC_GetDimension(const VDC *p, const char *dimname, VDCDimension *dimension) {
    VDC_DEBUG_called();
    return p->GetDimension(string(dimname), *dimension);
}

void VDC_GetDimensionNames(const VDC *p, char ***names, int *count) {
    VDC_DEBUG_called();
    _stringVectorToCStringArray(p->GetDimensionNames(), names, count);
}

int VDC_GetCoordVarInfo(const VDC *p, const char *varname, VDCCoordVar *var) {
    VDC_DEBUG_called();
    return p->GetCoordVarInfo(string(varname), *var);
}

int VDC_GetDataVarInfo(const VDC *p, const char *varname, VDCDataVar *var) {
    VDC_DEBUG_called();
    return p->GetDataVarInfo(string(varname), *var);
}

int VDC_GetBaseVarInfo(const VDC *p, const char *varname, VDCBaseVar *var) {
    VDC_DEBUG_called();
    return p->GetBaseVarInfo(string(varname), *var);
}

void VDC_GetDataVarNames(const VDC *p, char ***names, int *count) {
    VDC_DEBUG_called();
    _stringVectorToCStringArray(p->GetDataVarNames(), names, count);
}

void VDC_GetCoordVarNames(const VDC *p, char ***names, int *count) {
    VDC_DEBUG_called();
    _stringVectorToCStringArray(p->GetCoordVarNames(), names, count);
}

int VDC_GetNumRefLevels(const VDC *p, const char *varname) {
    VDC_DEBUG_called();
    return p->GetNumRefLevels(string(varname));
}

int VDC_GetAtt_long(const VDC *p, const char *varname, const char *attname, long **values, int *count) {
    VDC_DEBUG_called();
    vector<long> values_v;
    bool ret = p->GetAtt(string(varname), string(attname), values_v);
    if (ret)
        _longVectorToCArray(values_v, values, count);

    return ret;
}

int VDC_GetAtt_double(const VDC *p, const char *varname, const char *attname, double **values, int *count) {
    VDC_DEBUG_called();
    vector<double> values_v;
    bool ret = p->GetAtt(string(varname), string(attname), values_v);
    if (ret)
        _doubleVectorToCArray(values_v, values, count);

    return ret;
}

int VDC_GetAtt_text(const VDC *p, const char *varname, const char *attname, char **text) {
    VDC_DEBUG_called();
    string text_s;
    bool ret = p->GetAtt(string(varname), string(attname), text_s);
    if (ret)
        _stringToCString(text_s, text);

    return ret;
}

int VDC_GetAtt_Count(const VDC *p, const char *varname, const char *attname, int *count) {
    VDC_DEBUG_called();
    VDC::XType type = p->GetAttType(string(varname), string(attname));
    if (type < 0 || type == VDC::XType::INVALID)
        return 0;

    if (type == VDC::XType::TEXT) {
        *count = 1;
    } else if (type == VDC::XType::FLOAT || type == VDC::XType::DOUBLE) {
        vector<double> v;
        p->GetAtt(string(varname), string(attname), v);
        *count = v.size();
    } else if (type == VDC::XType::INT32 || type == VDC::XType::INT64) {
        vector<long> v;
        p->GetAtt(string(varname), string(attname), v);
        *count = v.size();
    }
    return 1;
}

void VDC_GetAttNames(const VDC *p, const char *varname, char ***names, int *count) {
    VDC_DEBUG_called();
    _stringVectorToCStringArray(p->GetAttNames(string(varname)), names, count);
}

int VDC_GetAttType(const VDC *p, const char *varname, const char *attname) {
    VDC_DEBUG_called();
    return _XTypeToInt(p->GetAttType(string(varname), string(attname)));
}

int VDC_VariableExists(const VDC *p, size_t ts, const char *varname, int reflevel, int lod) {
    VDC_DEBUG_called();
    return p->VariableExists(ts, string(varname), reflevel, lod);
}

int VDC_IsTimeVarying(const VDC *p, const char *varname) {
    VDC_DEBUG_called();
    return p->IsTimeVarying(string(varname));
}

int VDC_GetCRatios(const VDC *p, const char *varname, size_t **ratios, int *count) {
    VDC_DEBUG_called();
    VDCBaseVar v;
    bool ret = p->GetBaseVarInfo(string(varname), v);
    if (ret)
        _size_tVectorToCArray(v.GetCRatios(), ratios, count);
    return ret;
}

int VDC_GetCRatiosCount(const VDC *p, const char *varname) {
    VDC_DEBUG_called();
    VDCBaseVar v;
    p->GetBaseVarInfo(string(varname), v);
    return v.GetCRatios().size();
}

int VDC_GetVarDimNames(const VDC *p, const char *varname, int spatial, char ***names, int *count) {
    VDC_DEBUG_called();
    vector<string> names_v;
    bool ret = p->GetVarDimNames(string(varname), spatial, names_v);
    if (ret)
        _stringVectorToCStringArray(names_v, names, count);

    return ret;
}

int VDC_GetVarCoordVars(const VDC *p, const char *varname, int spatial, char ***names, int *count) {
    VDC_DEBUG_called();
    vector<string> names_v;
    bool ret = p->GetVarCoordVars(string(varname), spatial, names_v);
    if (ret)
        _stringVectorToCStringArray(names_v, names, count);

    return ret;
}

int VDC_OpenVariableRead(VDC *p, size_t ts, const char *varname, int level, int lod) {
    VDC_DEBUG_called();
    return p->OpenVariableRead(ts, string(varname), level, lod);
}

int VDC_CloseVariable(VDC *p) {
    VDC_DEBUG_called();
    return p->CloseVariable();
}

int VDC_Read(VDC *p, float *region) {
    VDC_DEBUG_called();
    return p->Read(region);
}

int VDC_ReadSlice(VDC *p, float *slice) {
    VDC_DEBUG_called();
    return p->ReadSlice(slice);
}

int VDC_ReadRegion(VDC *p, const size_t *min, const size_t *max, const int dims, float *region) {
    VDC_DEBUG_called();
    vector<size_t> min_v;
    vector<size_t> max_v;
    for (int i = 0; i < dims; i++) {
        min_v.push_back(min[i]);
        max_v.push_back(max[i]);
    }
    return p->ReadRegion(min_v, max_v, region);
}

int VDC_GetVar(VDC *p, const char *varname, int level, int lod, float *data) {
    VDC_DEBUG_called();
    return p->GetVar(string(varname), level, lod, data);
}

int VDC_GetVarAtTimeStep(VDC *p, size_t ts, const char *varname, int level, int lod, float *data) {
    VDC_DEBUG_called();
    return p->GetVar(ts, string(varname), level, lod, data);
}

// ########################
// #        Write         #
// ########################

int VDC_SetCompressionBlock(VDC *p, const size_t *bs, int bsCount, const char *wname, const size_t *cratios, int cratiosCount) {
    vector<size_t> bs_v = _size_tArrayToSize_tVector(bs, bsCount);
    vector<size_t> cratios_v = _size_tArrayToSize_tVector(cratios, cratiosCount);
    VDC_DEBUG_printff("(%s, \"%s\", %s)", _size_tVectorToString(bs_v).c_str(), wname, _size_tVectorToString(cratios_v).c_str());
    int ret = p->SetCompressionBlock(bs_v, string(wname), cratios_v);
    return ret;
}

int VDC_DefineDimension(VDC *p, const char *dimname, size_t length) {
    VDC_DEBUG_called();
    return p->DefineDimension(string(dimname), length);
}

int VDC_DefineDataVar(VDC *p, const char *varname, const char **dimnames, size_t dimnamesCount, const char **coordvars, size_t coordvarsCount, const char *units, VDC_XType xtype, int compressed) {
    VDC_DEBUG_printff("(%s, \"%s\", <dims>, %li, <coords>, %li, \"%s\", %s, %s)\n", p ? "*p" : "NULL", varname, dimnamesCount, coordvarsCount, units, _XTypeToString(_IntToXType(xtype)), _boolToStr(compressed));
    VDC_DEBUG_printff(": Current dimensions = %s\n", _stringVectorToString(p->GetDimensionNames()).c_str());
    VDC_DEBUG_printff(": Current coordvars = %s\n", _stringVectorToString(p->GetCoordVarNames()).c_str());
    VDC_DEBUG_printff(": Current datavars = %s\n", _stringVectorToString(p->GetDataVarNames()).c_str());
    vector<string> dimnames_v;
    vector<string> coordvars_v;
    for (int i = 0; i < dimnamesCount; i++)
        dimnames_v.push_back(string(dimnames[i]));
    for (int i = 0; i < coordvarsCount; i++)
        coordvars_v.push_back(string(coordvars[i]));
    VDC_DEBUG_printff(": calling VDC::DefineDataVar(\"%s\", %s, %s, \"%s\", %s, %s)\n", varname, _stringVectorToString(dimnames_v).c_str(), _stringVectorToString(coordvars_v).c_str(), units, _XTypeToString(_IntToXType(xtype)), _boolToStr(compressed));
    int ret = p->DefineDataVar(string(varname), dimnames_v, coordvars_v, string(units), _IntToXType(xtype), compressed);
    VDC_DEBUG_printff(": return (%i)\n", ret);
    if (ret < 0)
        VDC_DEBUG_printff(": Error message = \"%s\"\n", Wasp::MyBase::GetErrMsg());
    return ret;
}

int VDC_DefineCoordVar(VDC *p, const char *varname, const char **dimnames, size_t dimnamesCount, const char *time_dim_name, const char *units, int axis, VDC_XType xtype, int compressed) {
    VDC_DEBUG_printff("(%s, \"%s\", %s, %li, \"%s\", \"%s\", %s, %s, %s)\n", p ? "*p" : "NULL", varname, _stringVectorToString(_strArrayToStringVector(dimnames, dimnamesCount)).c_str(), dimnamesCount, time_dim_name, units, _AxisToStr(axis), _XTypeToString(_IntToXType(xtype)), _boolToStr(compressed));
    vector<string> dimnames_v;
    for (int i = 0; i < dimnamesCount; i++)
        dimnames_v.push_back(string(dimnames[i]));
    VDC_DEBUG_printff(": calling VDC::DefineCoordVar(\"%s\", %s, \"%s\", \"%s\", %s, %s, %s)\n", varname, _stringVectorToString(dimnames_v).c_str(), time_dim_name, units, _AxisToStr(axis), _XTypeToString(_IntToXType(xtype)), _boolToStr(compressed));
    return p->DefineCoordVar(string(varname), dimnames_v, string(time_dim_name), string(units), axis, _IntToXType(xtype), compressed);
}

int VDC_DefineCoordVarUniform(VDC *p, const char *varname, const char **dimnames, size_t dimnamesCount, const char *time_dim_name, const char *units, int axis, VDC_XType xtype, int compressed) {
    VDC_DEBUG_printff("(%s, \"%s\", %s, %li, \"%s\", \"%s\", %s, %s, %s)\n", p ? "*p" : "NULL", varname, _stringVectorToString(_strArrayToStringVector(dimnames, dimnamesCount)).c_str(), dimnamesCount, time_dim_name, units, _AxisToStr(axis), _XTypeToString(_IntToXType(xtype)), _boolToStr(compressed));
    vector<string> dimnames_v;
    for (int i = 0; i < dimnamesCount; i++)
        dimnames_v.push_back(string(dimnames[i]));
    VDC_DEBUG_printff(": calling VDC::DefineCoordVarUniform(\"%s\", %s, \"%s\", \"%s\", %s, %s, %s)\n", varname, _stringVectorToString(dimnames_v).c_str(), time_dim_name, units, _AxisToStr(axis), _XTypeToString(_IntToXType(xtype)), _boolToStr(compressed));
    return p->DefineCoordVarUniform(string(varname), dimnames_v, string(time_dim_name), string(units), axis, _IntToXType(xtype), compressed);
}

int VDC_PutAtt(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const void *values, size_t count) {
    VDC_DEBUG_printff("(%s, \"%s\", \"%s\", %s, <data>, %li)\n", p ? "*p" : "NULL", varname, attname, _XTypeToString(_IntToXType(xtype)), count);
    switch (xtype) {
    case VDC_XType_FLOAT:
    case VDC_XType_DOUBLE:
        return VDC_PutAtt_double(p, varname, attname, xtype, (const double *)values, count);
    case VDC_XType_INT32:
    case VDC_XType_INT64:
        return VDC_PutAtt_long(p, varname, attname, xtype, (const long *)values, count);
    case VDC_XType_TEXT:
        return VDC_PutAtt_text(p, varname, attname, xtype, (const char *)values);
    case VDC_XType_INVALID:
    default:
        return -2;
    }
}

static string valueCArrayToString(const void *a, int l, VDC_XType type) {
    string s("[");
    char buffer[128];
    for (int i = 0; i < l; i++) {
        switch (type) {
        case VDC_XType_FLOAT:
            sprintf(buffer, "%g", ((float *)a)[i]);
            break;
        case VDC_XType_DOUBLE:
            sprintf(buffer, "%g", ((double *)a)[i]);
            break;
        case VDC_XType_INT32:
            sprintf(buffer, "%i", ((int *)a)[i]);
            break;
        case VDC_XType_INT64:
            sprintf(buffer, "%li", ((long *)a)[i]);
            break;
        }
        s += string(buffer);
        if (i != l - 1)
            s += ", ";
    }
    return s + string("]");
}

int VDC_PutAtt_double(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const double *values, size_t count) {
    VDC_DEBUG_printff("(%s, \"%s\", \"%s\", %s, %s, %li)\n", p ? "*p" : "NULL", varname, attname, _XTypeToString(_IntToXType(xtype)), valueCArrayToString(values, count, xtype).c_str(), count);
    vector<double> values_v;
    if (xtype == VDC_XType_FLOAT)
        for (int i = 0; i < count; i++)
            values_v.push_back(((float *)values)[i]);
    else
        for (int i = 0; i < count; i++)
            values_v.push_back(values[i]);

    return p->PutAtt(string(varname), string(attname), _IntToXType(xtype), values_v);
}

int VDC_PutAtt_long(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const long *values, size_t count) {
    VDC_DEBUG_printff("(%s, \"%s\", \"%s\", %s, %s, %li)\n", p ? "*p" : "NULL", varname, attname, _XTypeToString(_IntToXType(xtype)), valueCArrayToString(values, count, xtype).c_str(), count);
    vector<long> values_v;
    if (xtype == VDC_XType_INT32)
        for (int i = 0; i < count; i++)
            values_v.push_back(((int *)values)[i]);
    else
        for (int i = 0; i < count; i++)
            values_v.push_back(values[i]);

    return p->PutAtt(string(varname), string(attname), _IntToXType(xtype), values_v);
}

int VDC_PutAtt_text(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const char *values) {
    VDC_DEBUG_printff("(%s, \"%s\", \"%s\", %s, \"%s\")\n", p ? "*p" : "NULL", varname, attname, _XTypeToString(_IntToXType(xtype)), values);
    return p->PutAtt(string(varname), string(attname), _IntToXType(xtype), string(values));
}

int VDC_EndDefine(VDC *p) {
    VDC_DEBUG_printff("(%s)\n", p ? "*p" : "NULL");
    return p->EndDefine();
}

int VDC_PutVar(VDC *p, const char *varname, int lod, const float *data) {
    VDC_DEBUG_called();
    return p->PutVar(string(varname), lod, data);
}

int VDC_PutVarAtTimeStep(VDC *p, size_t ts, const char *varname, int lod, const float *data) {
    VDC_DEBUG_printff("(%s, %li, \"%s\", %i, <data>)\n", p ? "*p" : "NULL", ts, varname, lod);
    return p->PutVar(ts, string(varname), lod, data);
}

// ########################
// #       Utility        #
// ########################

const char *VDC_GetErrMsg() {
    VDC_DEBUG_called();
    return Wasp::MyBase::GetErrMsg();
}

void VDC_FreeStringArray(char ***str, int *count) {
    for (int i = 0; i < *count; i++)
        delete (*str)[i];
    delete *str;
    *str = 0;
    *count = 0;
}

void VDC_FreeString(char **str) {
    delete *str;
    *str = 0;
}

void VDC_FreeLongArray(long **data) {
    delete *data;
    *data = 0;
}

void VDC_FreeDoubleArray(double **data) {
    delete *data;
    *data = 0;
}

void VDC_FreeSize_tArray(size_t **data) {
    delete *data;
    *data = 0;
}

// ########################
// #        Static        #
// ########################

void _stringToCString(const string s, char **str) {
    int size = s.size();
    *str = new char[size + 1];
    s.copy(*str, size);
    (*str)[size] = 0;
}

void _stringVectorToCStringArray(const vector<string> v, char ***str, int *count) {
    *count = v.size();
    *str = new char *[*count];
    for (int i = 0; i < *count; i++) {
        (*str)[i] = new char[v[i].size() + 1];
        v[i].copy((*str)[i], v[i].size());
        (*str)[i][v[i].size()] = 0;
    }
}

void _longVectorToCArray(const vector<long> v, long **data, int *count) {
    *count = v.size();
    *data = new long[*count];
    for (int i = 0; i < *count; i++)
        (*data)[i] = v[i];
}

void _boolVectorToCArray(const vector<bool> v, long **data, int *count) {
    *count = v.size();
    *data = new long[*count];
    for (int i = 0; i < *count; i++)
        (*data)[i] = v[i];
}

void _doubleVectorToCArray(const vector<double> v, double **data, int *count) {
    *count = v.size();
    *data = new double[*count];
    for (int i = 0; i < *count; i++)
        (*data)[i] = v[i];
}

void _size_tVectorToCArray(const vector<size_t> v, size_t **data, int *count) {
    *count = v.size();
    *data = new size_t[*count];
    for (int i = 0; i < *count; i++)
        (*data)[i] = v[i];
}

int _XTypeToInt(const VDC::XType type) {
    switch (type) {
    case VDC::XType::INVALID:
        return VDC_XType_INVALID;
    case VDC::XType::FLOAT:
        return VDC_XType_FLOAT;
    case VDC::XType::DOUBLE:
        return VDC_XType_DOUBLE;
    case VDC::XType::INT32:
        return VDC_XType_INT32;
    case VDC::XType::INT64:
        return VDC_XType_INT64;
    case VDC::XType::TEXT:
        return VDC_XType_TEXT;
    default:
        return -1;
    }
}

VDC::XType _IntToXType(int type) {
    switch (type) {
    case VDC_XType_INVALID:
        return VDC::XType::INVALID;
    case VDC_XType_FLOAT:
        return VDC::XType::FLOAT;
    case VDC_XType_DOUBLE:
        return VDC::XType::DOUBLE;
    case VDC_XType_INT32:
        return VDC::XType::INT32;
    case VDC_XType_INT64:
        return VDC::XType::INT64;
    case VDC_XType_TEXT:
        return VDC::XType::TEXT;
    default:
        return VDC::XType::INVALID;
    }
}

const char *_XTypeToString(const VDC::XType type) {
    switch (type) {
    case VDC::XType::INVALID:
        return "INVALID";
    case VDC::XType::FLOAT:
        return "float";
    case VDC::XType::DOUBLE:
        return "double";
    case VDC::XType::INT32:
        return "int32";
    case VDC::XType::INT64:
        return "int64";
    case VDC::XType::TEXT:
        return "text";
    default:
        return "<ERROR_TYPE>";
    }
}

const char *_boolToStr(const int b) {
    if (b)
        return "true";
    else
        return "false";
}

const char *_AxisToStr(const int a) {
    switch (a) {
    case 0:
        return "X";
    case 1:
        return "Y";
    case 2:
        return "Z";
    case 3:
        return "T";
    default:
        return "INVALID AXIS";
    }
}

vector<string> _strArrayToStringVector(const char **a, size_t count) {
    vector<string> v;
    for (int i = 0; i < count; i++)
        v.push_back(string(a[i]));
    return v;
}

string _stringVectorToString(const vector<string> v) {
    string s = "[";
    for (int i = 0; i < v.size(); i++) {
        s += "\"" + v[i] + "\"" + (i == v.size() - 1 ? "" : ", ");
    }
    s += "]";
    return s;
}

vector<size_t> _size_tArrayToSize_tVector(const size_t *a, size_t count) {
    vector<size_t> v;
    for (int i = 0; i < count; i++)
        v.push_back(a[i]);
    return v;
}

string _size_tVectorToString(const vector<size_t> v) {
    string s = "[";
    for (int i = 0; i < v.size(); i++) {
        s += "\"" + std::to_string(v[i]) + "\"" + (i == v.size() - 1 ? "" : ", ");
    }
    s += "]";
    return s;
}
