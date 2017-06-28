#ifndef _VDC_C_H_
#define _VDC_C_H_

#define VDC_AccessMode_R 1
#define VDC_AccessMode_W 2
#define VDC_AccessMode_A 3

#define VDC_XType         int
#define VDC_XType_INVALID -1
#define VDC_XType_FLOAT   1
#define VDC_XType_DOUBLE  2
#define VDC_XType_INT32   3
#define VDC_XType_INT64   4
#define VDC_XType_TEXT    5

#define VDCDimension_Axis_X 0
#define VDCDimension_Axis_Y 1
#define VDCDimension_Axis_Z 2
#define VDCDimension_Axis_T 3

#ifdef __cplusplus

    #include "vapor/VDC.h"
typedef VAPoR::VDC            VDC;
typedef VAPoR::VDC::Dimension VDCDimension;
typedef VAPoR::VDC::BaseVar   VDCBaseVar;
typedef VAPoR::VDC::AuxVar    VDCAuxVar;
typedef VAPoR::VDC::DataVar   VDCDataVar;
typedef VAPoR::VDC::CoordVar  VDCCoordVar;
extern "C" {

#else

typedef struct VDC          VDC;
typedef struct VDCDimension VDCDimension;
typedef struct VDCBaseVar   VDCBaseVar;
typedef struct VDCAuxVar    VDCAuxVar;
typedef struct VDCDataVar   VDCDataVar;
typedef struct VDCCoordVar  VDCCoordVar;

#endif

VDCDimension *VDCDimension_new();
void          VDCDimension_delete(VDCDimension *p);
void          VDCDimension_GetName(const VDCDimension *p, char **name);
size_t        VDCDimension_GetLength(const VDCDimension *p);
int           VDCDimension_IsTimeVarying(const VDCDimension *p);

VDCBaseVar *VDCBaseVar_new();
void        VDCBaseVar_delete(VDCBaseVar *p);
void        VDCBaseVar_GetName(const VDCBaseVar *p, char **name);
void        VDCBaseVar_GetUnits(const VDCBaseVar *p, char **units);
int         VDCBaseVar_GetXType(const VDCBaseVar *p);
void        VDCBaseVar_GetWName(const VDCBaseVar *p, char **name);
void        VDCBaseVar_GetCRatios(const VDCBaseVar *p, size_t **ratios, int *count);
void        VDCBaseVar_GetBS(const VDCBaseVar *p, size_t **bs, int *count);
void        VDCBaseVar_GetPeriodic(const VDCBaseVar *p, long **periodic, int *count);
void        VDCBaseVar_GetAttributeNames(const VDCBaseVar *p, char ***names, int *count);    // Return names instead of objects
int         VDCBaseVar_IsCompressed(const VDCBaseVar *p);

VDCAuxVar *VDCAuxVar_new();
void       VDCAuxVar_delete(VDCAuxVar *p);
void       VDCAuxVar_GetDimNames(const VDCAuxVar *p, char ***names, int *count);

VDCDataVar *VDCDataVar_new();
void        VDCDataVar_delete(VDCDataVar *p);
void        VDCDataVar_GetMeshName(const VDCDataVar *p, char **name);
void        VDCDataVar_GetTimeCoordVar(const VDCDataVar *p, char **name);
// GetSamplingLocation
void   VDCDataVar_GetMaskvar(const VDCDataVar *p, char **name);
int    VDCDataVar_GetHasMissing(const VDCDataVar *p);
double VDCDataVar_GetMissingValue(const VDCDataVar *p);

VDCCoordVar *VDCCoordVar_new();
void         VDCCoordVar_delete(VDCCoordVar *p);
void         VDCCoordVar_GetDimNames(const VDCCoordVar *p, char ***names, int *count);
void         VDCCoordVar_GetTimeDimName(const VDCCoordVar *p, char **name);
int          VDCCoordVar_GetAxis(const VDCCoordVar *p);
int          VDCCoordVar_GetUniform(const VDCCoordVar *p);

VDC *VDC_new();
void VDC_delete(VDC *p);
int  VDC_Initialize(VDC *p, const char *path, int mode);

int  VDC_GetDimension(const VDC *p, const char *dimname, VDCDimension *dimension);
void VDC_GetDimensionNames(const VDC *p, char ***names, int *count);
int  VDC_GetCoordVarInfo(const VDC *p, const char *varname, VDCCoordVar *var);
int  VDC_GetDataVarInfo(const VDC *p, const char *varname, VDCDataVar *var);
int  VDC_GetBaseVarInfo(const VDC *p, const char *varname, VDCBaseVar *var);
void VDC_GetDataVarNames(const VDC *p, char ***names, int *count);
void VDC_GetCoordVarNames(const VDC *p, char ***names, int *count);
int  VDC_GetNumRefLevels(const VDC *p, const char *varname);
int  VDC_GetAtt_long(const VDC *p, const char *varname, const char *attname, long **values, int *count);
int  VDC_GetAtt_double(const VDC *p, const char *varname, const char *attname, double **values, int *count);
int  VDC_GetAtt_text(const VDC *p, const char *varname, const char *attname, char **text);
int  VDC_GetAtt_Count(const VDC *p, const char *varname, const char *attname, int *count);
void VDC_GetAttNames(const VDC *p, const char *varname, char ***names, int *count);
int  VDC_GetAttType(const VDC *p, const char *varname, const char *attname);
int  VDC_VariableExists(const VDC *p, size_t ts, const char *varname, int reflevel, int lod);
int  VDC_IsTimeVarying(const VDC *p, const char *varname);

int VDC_GetCRatios(const VDC *p, const char *varname, size_t **ratios, int *count);
int VDC_GetCRatiosCount(const VDC *p, const char *varname);

int VDC_GetVarDimNames(const VDC *p, const char *varname, int spatial, char ***names, int *count);
int VDC_GetVarCoordVars(const VDC *p, const char *varname, int spatial, char ***names, int *count);

int VDC_OpenVariableRead(VDC *p, size_t ts, const char *varname, int level, int lod);
int VDC_CloseVariable(VDC *p);
int VDC_Read(VDC *p, float *region);
int VDC_ReadSlice(VDC *p, float *slice);
int VDC_ReadRegion(VDC *p, const size_t *min, const size_t *max, const int dims, float *region);
int VDC_GetVar(VDC *p, const char *varname, int level, int lod, float *data);
int VDC_GetVarAtTimeStep(VDC *p, size_t ts, const char *varname, int level, int lod, float *data);

// Write

int VDC_DefineDimension(VDC *p, const char *dimname, size_t length);
int VDC_DefineDataVar(VDC *p, const char *varname, const char **dimnames, size_t dimnamesCount, const char **coordvars, size_t coordvarCount, const char *units, VDC_XType xtype, int compressed);
int VDC_DefineCoordVar(VDC *p, const char *varname, const char **dimnames, size_t dimnamesCount, const char *time_dim_name, const char *units, int axis, VDC_XType xtype, int compressed);
int VDC_PutAtt(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const void *values, size_t count);
int VDC_PutAtt_double(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const double *values, size_t count);
int VDC_PutAtt_long(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const long *values, size_t count);
int VDC_PutAtt_text(VDC *p, const char *varname, const char *attname, VDC_XType xtype, const char *values);
int VDC_EndDefine(VDC *p);
int VDC_PutVar(VDC *p, const char *varname, int lod, const float *data);
int VDC_PutVarAtTimeStep(VDC *p, size_t ts, const char *varname, int lod, const float *data);

// Utility
const char *VDC_GetErrMsg();
void        VDC_FreeStringArray(char ***str, int *count);
void        VDC_FreeString(char **str);
void        VDC_FreeLongArray(long **data);
void        VDC_FreeDoubleArray(double **data);
void        VDC_FreeSize_tArray(size_t **data);

#ifdef __cplusplus
}    // extern "C" }
#endif

#endif
