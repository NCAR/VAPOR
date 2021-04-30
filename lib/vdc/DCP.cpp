#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include "vapor/VAssert.h"
#include <stdio.h>

#ifdef _WINDOWS
#define _USE_MATH_DEFINES
#pragma warning(disable : 4251 4100)
#endif
#include <cmath>

#include <vapor/GeoUtil.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/DCP.h>
#include <vapor/DCUtils.h>

using namespace VAPoR;
using namespace std;

[[maybe_unused]] static const string ToStr(const vector<size_t> &v) {
    vector<string> vs;
    for (const auto &e : v)
        vs.push_back(to_string(e));
    
    return "{" + STLUtils::Join(vs, ", ") + "}";
}

[[maybe_unused]] static const string ToStr(const vector<string> &v) {
    return "{" + STLUtils::Join(v, ", ") + "}";
}

template<typename T>
static const char *TypeToChar(const T &t);

template<> [[maybe_unused]] const char *TypeToChar<int>   (const int    &t) { return "int"; }
template<> [[maybe_unused]] const char *TypeToChar<float> (const float  &t) { return "float"; }
template<> [[maybe_unused]] const char *TypeToChar<string>(const string &t) { return "string"; }
template<> [[maybe_unused]] const char *TypeToChar<vector<long>>  (const vector<long>   &t) { return "vector<long>"; }
template<> [[maybe_unused]] const char *TypeToChar<vector<double>>(const vector<double> &t) { return "vector<double>"; }

namespace {

#ifdef UNUSED_FUNCTION
// Product of elements in a vector
//
size_t vproduct(vector <size_t> a) {
	size_t ntotal = 1;

	for (int i=0; i<a.size(); i++) ntotal *= a[i];
	return(ntotal);
}
#endif

};

DCP::DCP() {
	_ncdfc = NULL;

	_dimsMap.clear();
	_coordVarsMap.clear();
	_dataVarsMap.clear();
	_meshMap.clear();
	_coordVarKeys.clear();

}

DCP::~DCP() {
	if (_ncdfc) delete _ncdfc;
}

static void ReplaceAll(string *s, char a, char b)
{
    for (auto &c : *s)
        c = c == a ? b : c;
}

[[maybe_unused]] static const string S(const string &v) {return v;}
[[maybe_unused]] static const string S(const bool &v) {return v?"true":"false";}
template<typename T> const string S(const T &v) {return std::to_string(v);}
template<typename T> const string S(vector<T> v) {
    string s = "{";
    for (int i = 0; i < v.size(); i++) {
        s += S(v[i]);
        if (i != v.size()-1)
            s += ", ";
    }
    s += "}";
    return s;
}
template<typename T> void P(const T &v) {printf("%s", S(v).c_str());}
template<typename T> const char *C(const T &v) {
    static string ret[32];
    static int i = -1;
    if (++i >= 32) i = 0;
    ret[i] = S(v);
    return ret[i].c_str();
}

static void DumpNCDFC(NetCDFCollection *n)
{
    auto dimNames = n->GetDimNames();
    auto dimLens = n->GetDims();
    
    printf("Dimensions\n");
    for (int i = 0; i < dimNames.size(); i++)
        printf("\t%s[%li]\n", dimNames[i].c_str(), dimLens[i]);
    
    printf("Variables\n");
    for (int dim = 0; dim < 5; dim++) {
        auto varNames = n->GetVariableNames(dim, true);
        for (auto var : varNames) {
            printf("\t%s[%i]%s\n", var.c_str(), dim, C(n->GetDimNames(var)));
            
            auto attNames = n->GetAttNames(var);
            for (auto att : attNames) {
                printf("\t\t%s = ", att.c_str());
                int type = n->GetAttType(var, att);
                switch (type) {
                    case NC_INT64: {vector<long> v; n->GetAtt(var, att, v); P(v);} break;
                    case NC_DOUBLE: {vector<double> v; n->GetAtt(var, att, v); P(v);} break;
                    case NC_CHAR: {string v; n->GetAtt(var, att, v); P(v);} break;
                    default: printf("ERR"); break;
                }
                printf("\n");
            }
        }
    }
}

int DCP::initialize(
	const vector <string> &paths, const std::vector <string> &options
) {

	NetCDFCollection *ncdfc = new NetCDFCollection();
    _ncdfc = ncdfc;

	// Initialize the NetCDFCFCollection class. 
	//
    int rc = ncdfc->Initialize(paths, {"time", "T"}, {"time", "T"});
	if (rc<0) {
		SetErrMsg("Failed to initialize netCDF data collection for reading");
		return(-1);
	}

	// Use UDUnits for unit conversion
	//
	rc = _udunits.Initialize();
	if (rc<0) {
		SetErrMsg(
			"Failed to initialize udunits2 library : %s",
			_udunits.GetErrMsg().c_str()
		);
		return(-1);
	}
    
    DumpNCDFC(ncdfc);
    
    // Dimensions
    // ==================
    string particlesDim;
    string axisDim;
    auto dimNames = ncdfc->GetDimNames();
    auto dimLens = ncdfc->GetDims();
    
    bool dimNamePhony = false;
    for (auto d : dimNames)
        if (STLUtils::BeginsWith(d, "phony"))
            dimNamePhony = true;
    if (dimNamePhony) {
        particlesDim = "phony_dim_0";
        axisDim = "phony_dim_1";
    } else {
        particlesDim = "P";
        axisDim = "axis";
    }
    
    for (int i = 0; i < dimNames.size(); i++)
        _dimsMap[dimNames[i]] = DC::Dimension(dimNames[i], dimLens[i]);
    
    _dimsMap["densityX"] = DC::Dimension("densityX", 32);
    _dimsMap["densityY"] = DC::Dimension("densityY", 32);
    _dimsMap["densityZ"] = DC::Dimension("densityZ", 4);
    
    
    // Aux Vars
    // ==================
    {
        vector<bool> periodic(3, false);
        for (auto v : {nodeFaceVar, faceNodeVar})
            _auxVarsMap[v] = AuxVar(v, "", DC::INT32, "", vector<size_t>(), periodic, {particlesDim});
    }
    
    // Coord Vars
    // ==================
    for (int dim = 0; dim < 5; dim++) {
        auto vars = ncdfc->GetVariableNames(dim, true);
        for (auto &var : vars) {
            if (isCoordVar(var)) {
                const vector<bool> periodic(false);
                auto spacialDims = ncdfc->GetSpatialDimNames(var);
                auto timeDim = ncdfc->GetTimeDimName(var);
                if (STLUtils::Contains(spacialDims, axisDim)) {
                    spacialDims.erase(find(spacialDims.begin(), spacialDims.end(), axisDim));
                    for (auto axis : {"_x", "_y", "_z"})
                        _coordVarsMap[var+axis] = CoordVar(var+axis, getUnits(var), DC::FLOAT, periodic, getAxis(var+axis), false, spacialDims, timeDim);
                } else {
                    _coordVarsMap[var] = CoordVar(var, getUnits(var), DC::FLOAT, periodic, getAxis(var), false, spacialDims, timeDim);
                }
            }
        }
    }
    
    // Mesh
    // ==================
    string particlePositions = "Position";
    vector<string> coords = {
        particlePositions + "_x",
        particlePositions + "_y",
        particlePositions + "_z",
    };
    DC::Mesh mesh("particles", 1, 1, particlesDim, particlesDim, coords);
    mesh.SetNodeFaceVar(nodeFaceVar);
    mesh.SetFaceNodeVar(faceNodeVar);
    _meshMap[mesh.GetName()] = mesh;

    // Data Vars
    // ==================
    {
        vector<bool> periodic(3, false);
        for (int dim = 0; dim < 5; dim++) {
            auto vars = ncdfc->GetVariableNames(dim, true);
            for (auto v : vars) {
                if (!isCoordVar(v)) {
                    auto v_san = sanitizeVarName(v);
                    auto dims = ncdfc->GetDimNames(v);
                    if (STLUtils::Contains(dims, axisDim)) {
                        for (auto axis : {"_x", "_y", "_z"}) {
                            auto name = v_san+axis;
                            _dataVarsMap[name] = DC::DataVar(name, "", DC::FLOAT, periodic, mesh.GetName(), getTimeCoordVar(v), DC::Mesh::NODE);
                        }
                    } else {
                        _dataVarsMap[v_san] = DC::DataVar(v_san, "", DC::FLOAT, periodic, mesh.GetName(), getTimeCoordVar(v), DC::Mesh::NODE);
                    }
                }
            }
        }
    }
    
    return 0;
    /*
    auto particleAttributes = ncdfc->GetVariableNames(1,true);
    auto particleVecs = ncdfc->GetVariableNames(2,true);
    
    for (auto &n : particleAttributes) n = sanitizeVarName(n);
    for (auto &n : particleVecs)       n = sanitizeVarName(n);
    
    string particlePositions = "Position";
    vector<string> coords = {
        particlePositions + "_x",
        particlePositions + "_y",
        particlePositions + "_z",
    };
    {
        auto positionPos = find(particleVecs.begin(), particleVecs.end(), particlePositions);
        VAssert(positionPos != particleVecs.end());
        particleVecs.erase(positionPos);
    }
    
//    printf("Particle Coordinates\n");
//    printf("\t%s\n", particlePositions.c_str());
//    printf("Particle Vecs\n");
//    for (auto s : particleVecs)
//        printf("\t%s\n", s.c_str());
//    printf("Particle Attributes\n");
//    for (auto s : particleAttributes)
//        printf("\t%s\n", s.c_str());
    
    // Coord Vars
    // ==================
    string timeCoordVar = "";
    {
        vector <bool> periodic(false);
        for (int i = 0; i < 3; i++) {
            auto c = coords[i];
            auto axis = i;
            _coordVarsMap[c] = CoordVar(c, "", DC::FLOAT, periodic, axis, false, {"phony_dim_0"}, timeDim);
        }
    }
    
    // Aux Vars
    // ==================
    {
        vector<bool> periodic(3, false);
        for (auto v : {nodeFaceVar, faceNodeVar})
            _auxVarsMap[v] = AuxVar(v, "", DC::INT32, "", vector<size_t>(), periodic, {particlesDim});
    }
    
    // Mesh
    // ==================
    DC::Mesh mesh("particles", 1, 1, "phony_dim_0", "phony_dim_0", coords);
    mesh.SetNodeFaceVar(nodeFaceVar);
    mesh.SetFaceNodeVar(faceNodeVar);
    _meshMap[mesh.GetName()] = mesh;
    
    
    // Data Vars
    // ==================
    {
        vector<bool> periodic(3, false);
        for (string v : particleVecs) {
            auto dims = ncdfc->GetDimNames(getOriginalVarName(v));
            if (STLUtils::Contains(dims, axesDim)) {
                for (auto axis : {"_x", "_y", "_z"})
                    _dataVarsMap[v+axis] = DC::DataVar(v+axis, "", DC::FLOAT, periodic, mesh.GetName(), timeCoordVar, DC::Mesh::NODE);
            } else {
                _dataVarsMap[v] = DC::DataVar(v, "", DC::FLOAT, periodic, mesh.GetName(), timeCoordVar, DC::Mesh::NODE);
            }
        }
    }
    
	_ncdfc = ncdfc;

	return(0);
     */
}

bool DCP::isCoordVar(const string &var) const
{
    return STLUtils::BeginsWith(var, "Position") || var == "T";
    
    int type = _ncdfc->GetAttType(var, "coordinates");
    if (type < 0)
        return true;
    assert(type == NC_CHAR);
    return false;
}

int DCP::getAxis(const string &var) const
{
    string s;
    _ncdfc->GetAtt(var, "axis", s);
    s = STLUtils::ToLower(s);
    if (s == "x") return 0;
    if (s == "y") return 1;
    if (s == "z") return 2;
    if (s == "t") return 3;
    if (var == "T") return 3;
    if (STLUtils::EndsWith(var, "_x")) return 0;
    if (STLUtils::EndsWith(var, "_y")) return 1;
    if (STLUtils::EndsWith(var, "_z")) return 2;
    VAssert(0);
    return -1;
}

string DCP::getUnits(const string &var) const
{
    string s;
    _ncdfc->GetAtt(var, "units", s);
    return s;
}

string DCP::getTimeCoordVar(const string &var) const
{
    if (_ncdfc->GetTimeDimName(var) == "T")
        return "T";
    return "";
}

bool DCP::getDimension(
	string dimname, DC::Dimension &dimension
) const {
	map <string, DC::Dimension>::const_iterator itr;

	itr = _dimsMap.find(dimname);
	if (itr == _dimsMap.end()) return(false);

	dimension = itr->second;
	return(true); 
}

std::vector <string> DCP::getDimensionNames() const {
	map <string, DC::Dimension>::const_iterator itr;

	vector <string> names;

	for (itr=_dimsMap.begin(); itr != _dimsMap.end(); ++itr) {
		names.push_back(itr->first);
	}

	return(names);
}

vector <string> DCP::getMeshNames() const {
	vector <string> mesh_names;
	std::map <string, Mesh>::const_iterator itr = _meshMap.begin();
	for (;itr!=_meshMap.end(); ++itr) {
		mesh_names.push_back(itr->first);
	}
	return(mesh_names);
}

bool DCP::getMesh(
	string mesh_name, DC::Mesh &mesh
) const {

	map <string, Mesh>::const_iterator itr = _meshMap.find(mesh_name);
	if (itr == _meshMap.end()) return (false);

	mesh = itr->second;
	return(true);
}

bool DCP::getCoordVarInfo(string varname, DC::CoordVar &cvar) const {

	map <string, DC::CoordVar>::const_iterator itr;

	itr = _coordVarsMap.find(varname);
	if (itr == _coordVarsMap.end()) {
		return(false);
	}

	cvar = itr->second;
	return(true);
}

bool DCP::getDataVarInfo( string varname, DC::DataVar &datavar) const {

	map <string, DC::DataVar>::const_iterator itr;

	itr = _dataVarsMap.find(varname);
	if (itr == _dataVarsMap.end()) {
		return(false);
	}

	datavar = itr->second;
	return(true);
}

bool DCP::getBaseVarInfo(string varname, DC::BaseVar &var) const {
	map <string, DC::CoordVar>::const_iterator itr;

	itr = _coordVarsMap.find(varname);
	if (itr != _coordVarsMap.end()) {
		var = itr->second;
		return(true);
	}

	map <string, DC::DataVar>::const_iterator itr1 = _dataVarsMap.find(varname);
	if (itr1 != _dataVarsMap.end()) {
		var = itr1->second;
		return(true);
	}

	return(false);
}


std::vector <string> DCP::getDataVarNames() const {
	map <string, DC::DataVar>::const_iterator itr;

	vector <string> names;
	for (itr = _dataVarsMap.begin(); itr != _dataVarsMap.end(); ++itr) {
		names.push_back(itr->first);
	}
	return(names);
}


std::vector <string> DCP::getCoordVarNames() const {
	map <string, DC::CoordVar>::const_iterator itr;

	vector <string> names;
	for (itr = _coordVarsMap.begin(); itr != _coordVarsMap.end(); ++itr) {
		names.push_back(itr->first);
	}
	return(names);
}

template <class T>
bool DCP::_getAttTemplate(
	string varname, string attname, T &values
) const {
    
    printf("%s(%s, %s, %s&)\n", __func__, varname.c_str(), attname.c_str(), TypeToChar(values));

    if (_ncdfc->GetAttType(varname, attname) < 0)
        return false;
    
    _ncdfc->GetAtt(varname, attname, values);

	return(true);
}

bool DCP::getAtt(
	string varname, string attname, vector <double> &values
) const {
	values.clear();

	return(_getAttTemplate(varname, attname, values));
}

bool DCP::getAtt(
	string varname, string attname, vector <long> &values
) const {
	values.clear();

	return(_getAttTemplate(varname, attname, values));
}

bool DCP::getAtt(
	string varname, string attname, string &values
) const {
	values.clear();

	return(_getAttTemplate(varname, attname, values));
}

std::vector <string> DCP::getAttNames(string varname) const {
    printf("%s(%s) = ", __func__, varname.c_str());
    
    if (varname.empty()) {
        return _ncdfc->GetAttNames("");
    } else {
        DC::BaseVar var;
        bool status = getBaseVarInfo(varname, var);
        if (! status) {
            printf("{} (ERR)\n");
            return(vector <string> ());
        }

        vector <string> names;

        const std::map <string, Attribute> &atts = var.GetAttributes();
        std::map <string, Attribute>::const_iterator itr;
        for (itr = atts.begin(); itr!=atts.end(); ++itr) {
            names.push_back(itr->first);
        }

        printf("%s\n", ToStr(names).c_str());
        return(names);
    }
}

DC::XType DCP::getAttType(string varname, string attname) const {
    printf("%s(%s, %s)\n", __func__, varname.c_str(), attname.c_str());
	DC::BaseVar var;
	bool status = getBaseVarInfo(varname, var);
	if (! status) return(DC::INVALID);

	DC::Attribute att;
	status = var.GetAttribute(attname, att);
	if (! status) return(DC::INVALID);

	return(att.GetXType());
}

int DCP::getDimLensAtLevel(
	string varname, int, std::vector <size_t> &dims_at_level,
	std::vector <size_t> &bs_at_level
) const {
	dims_at_level.clear();
	bs_at_level.clear();

	bool ok = GetVarDimLens(varname, true, dims_at_level);
	if (!ok) {
		SetErrMsg("Undefined variable name : %s", varname.c_str());
		return(-1);
	}

	// Never blocked
	//
	bs_at_level = vector <size_t> (dims_at_level.size(), 1);

	return(0);
}


int DCP::openVariableRead(
	size_t ts, string varname
) {
    printf("DCP::%s(%li, %s) = ", __func__, ts, varname.c_str());
    
    if (STLUtils::Contains(fakeVars, varname)) {
        int ret = fakeVarsFileCounter++;
        printf("(fake) %i\n", ret);
        _fdMap[ret] = varname;
        return ret;
    }
    
    string ncdfName = varname;
    if (STLUtils::EndsWith(ncdfName, "_x") ||
        STLUtils::EndsWith(ncdfName, "_y") ||
        STLUtils::EndsWith(ncdfName, "_z"))
    {
        if (!_ncdfc->VariableExists(varname)) {
            ncdfName = ncdfName.substr(0, ncdfName.length()-2);
            printf("<removing _axis>\n\t");
            printf("DCP::%s(%li, %s) = ", __func__, ts, ncdfName.c_str());
        }
    }
    
    ncdfName = getOriginalVarName(ncdfName);

	int aux = _ncdfc->OpenRead(ts, ncdfName);
    if (!(aux < 0)) {
        FileTable::FileObject *f = new FileTable::FileObject(ts, ncdfName, 0, 0, aux);
        aux = _fileTable.AddEntry(f);
    }
    
    printf("%i\n", aux);
    _fdMap[aux] = varname;
	return aux;
}


int DCP::closeVariable(int fd) {
    printf("DCP::%s(%i)\n", __func__, fd);
    
    auto fdIt = _fdMap.find(fd);
    if (fdIt != _fdMap.end())
        _fdMap.erase(fdIt);
    
    if (fd >= fakeVarsFileCounterStart) {
        fakeVarsFileCounter--;
        return 0;
    }
    
	DC::FileTable::FileObject *w = _fileTable.GetEntry(fd);

	if (! w) {
		SetErrMsg("Invalid file descriptor : %d", fd);
		return(-1);
	}
	int aux = w->GetAux();

	int rc = _ncdfc->Close(aux);

	_fileTable.RemoveEntry(fd);

	return(rc);
}

// min -> max is inclusive

template <class T>
int DCP::_readRegionTemplate(
	int fd,
	const vector <size_t> &min_, const vector <size_t> &max_, T *region
) {
    vector<size_t> min = min_;
    vector<size_t> max = max_;
    string varname = _fdMap[fd];
//    printf("%s(%i(%s), %s, %s, %s*)\n", __func__, fd, varname.c_str(), ToStr(min).c_str(), ToStr(max).c_str(), TypeToChar(*region));
    
//    VAssert(min.size() == 1);
    
    bool fake = fd >= fakeVarsFileCounterStart;
    
    if (fake) {
//        printf("\t (Fake)\n");
        for (size_t i = min[0]; i <= max[0]; i++)
            region[i] = i/(double)(max[0]-1);
        return 0;
    }
    
    if (!_ncdfc->VariableExists(varname)) {
        int axis = -1;
        if (STLUtils::EndsWith(varname, "_x")) axis = 0;
        if (STLUtils::EndsWith(varname, "_y")) axis = 1;
        if (STLUtils::EndsWith(varname, "_z")) axis = 2;
        if (axis != -1) {
            
//        printf("\t Read data from header\n");
//        for (size_t i = min[0]; i <= max[0]; i++)
//            region[i] = Particle_Position[i*3+axis];
//        return 0;
        
        
//        min.push_back(axis);
//        max.push_back(axis);
            min.push_back(min[0]);
            max.push_back(max[0]);
            min[0] = axis;
            max[0] = axis;
        
//            printf("\t %s(%i(%s), %s, %s, %s*)\n", __func__, fd, varname.substr(0,varname.length()-2).c_str(), ToStr(min).c_str(), ToStr(max).c_str(), TypeToChar(*region));
        }
    }
    
	FileTable::FileObject *w = (FileTable::FileObject *) _fileTable.GetEntry(fd);

	if (! w) {
		SetErrMsg("Invalid file descriptor : %d", fd);
		return(-1);
	}
	int aux = w->GetAux();

	vector <size_t> ncdf_start = min;
	reverse(ncdf_start.begin(), ncdf_start.end());

	vector <size_t> ncdf_max = max;
	reverse(ncdf_max.begin(), ncdf_max.end());

	vector <size_t> ncdf_count;
	for (int i=0; i<ncdf_start.size(); i++) {
		ncdf_count.push_back(ncdf_max[i] - ncdf_start[i] + 1);
	}

	return(_ncdfc->Read(ncdf_start, ncdf_count, region, aux));
}

bool DCP::variableExists(
	size_t ts, string varname, int, int 
) const {
    bool found = false;
    
    for (const auto &it : _dataVarsMap) {
        if (found) break;
        if (it.first == varname) found = true;
    }
    for (const auto &it : _coordVarsMap) {
        if (found) break;
        if (it.first == varname) found = true;
    }
    for (const auto &it : _auxVarsMap) {
        if (found) break;
        if (it.first == varname) found = true;
    }
    
    if (found == false)
        printf("WARNING %s(%li, %s) = %s\n", __func__, ts, varname.c_str(), found?"true":"false");
    return found;
}


std::vector <string> DCP::getAuxVarNames() const
{
    vector<string> names(_auxVarsMap.size());
    for (const auto &it : _auxVarsMap)
        names.push_back(it.first);
    return names;
}

bool DCP::getAuxVarInfo(string varname, DC::AuxVar &var) const
{
    const auto &it = _auxVarsMap.find(varname);
    if (it == _auxVarsMap.end())
        return false;
    var = it->second;
    return true;
}

string DCP::sanitizeVarName(const string &name)
{
    assert(_sanitizedToOriginalMap.count(name) == 0);
    
    string sanitizedName = name;
    ReplaceAll(&sanitizedName, ' ', '_');
    _sanitizedToOriginalMap[sanitizedName] = name;
    
    return sanitizedName;
}

string DCP::getOriginalVarName(const string &name) const
{
    auto it = _sanitizedToOriginalMap.find(name);
    if (it == _sanitizedToOriginalMap.end())
        return name;
    return it->second;
}
