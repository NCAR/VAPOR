#include <cassert>
#include <sstream>
#include <sstream>
#include <iterator>
#include "vapor/NetCDFCpp.h"
#include "vapor/MatWaveBase.h"

using namespace VAPoR;

#define MY_NC_ERR(rc, path, func)                                                                                                                        \
    if (rc != NC_NOERR) {                                                                                                                                \
        string msg = func;                                                                                                                               \
        SetErrMsg("Error accessing netCDF file \"%s\", %s : %s -- file (%s), line(%d)", path.c_str(), msg.c_str(), nc_strerror(rc), __FILE__, __LINE__); \
        return (rc);                                                                                                                                     \
    }

NetCDFCpp::NetCDFCpp()
{
    _ncid = -1;
    _path.clear();
}

NetCDFCpp::~NetCDFCpp() {}

int NetCDFCpp::Create(string path, int cmode, size_t initialsz, size_t &bufrsizehintp)
{
    NetCDFCpp::Close();

    _ncid = -1;
    _path.clear();

    int ncid;
    int rc = nc__create(path.c_str(), cmode, initialsz, &bufrsizehintp, &ncid);
    MY_NC_ERR(rc, path, "nc__create()");

    _path = path;
    _ncid = ncid;

    return (NC_NOERR);
}

int NetCDFCpp::Open(string path, int mode)
{
    NetCDFCpp::Close();

    _ncid = -1;
    _path.clear();

    int ncid;
    int rc = nc_open(path.c_str(), mode, &ncid);
    MY_NC_ERR(rc, path, "nc_open()");

    _path = path;
    _ncid = ncid;

    return (NC_NOERR);
}

int NetCDFCpp::SetFill(int fillmode, int &old_modep)
{
    int rc = nc_set_fill(_ncid, fillmode, &old_modep);
    MY_NC_ERR(rc, _path, "nc_set_fill()");
    return (NC_NOERR);
}

int NetCDFCpp::EndDef() const
{
    int rc = nc_enddef(_ncid);
    MY_NC_ERR(rc, _path, "nc_enddif()");
    return (NC_NOERR);
}

int NetCDFCpp::Close()
{
    if (_ncid < 0) return (NC_NOERR);

    int rc = nc_close(_ncid);
    MY_NC_ERR(rc, _path, "nc_close()");

    _ncid = -1;
    _path.clear();

    return (NC_NOERR);
}

int NetCDFCpp::DefDim(string name, size_t len) const
{
    int dimid;
    int rc = nc_def_dim(_ncid, name.c_str(), len, &dimid);
    MY_NC_ERR(rc, _path, "nc_def_dim(" + name + ")");

    return (NC_NOERR);
}

int NetCDFCpp::DefVar(string name, nc_type xtype, vector<string> dimnames)
{
    int dimids[NC_MAX_DIMS];

    for (int i = 0; i < dimnames.size(); i++) {
        int rc = nc_inq_dimid(_ncid, dimnames[i].c_str(), &dimids[i]);
        MY_NC_ERR(rc, _path, "nc_inq_dimid(" + dimnames[i] + ")");
    }

    int varid;
    int rc = nc_def_var(_ncid, name.c_str(), xtype, dimnames.size(), dimids, &varid);
    MY_NC_ERR(rc, _path, "nc_def_var(" + name + ")");
    return (NC_NOERR);
}

int NetCDFCpp::InqVarDims(string name, vector<string> &dimnames, vector<size_t> &dims) const
{
    dimnames.clear();
    dims.clear();

    int varid;
    int rc = NetCDFCpp::InqVarid(name, varid);
    if (rc < 0) return (rc);

    int ndims;
    rc = nc_inq_varndims(_ncid, varid, &ndims);
    MY_NC_ERR(rc, _path, "nc_inq_varndims()");

    int dimids[NC_MAX_VAR_DIMS];
    rc = nc_inq_vardimid(_ncid, varid, dimids);
    MY_NC_ERR(rc, _path, "nc_inq_vardimid()");

    for (int i = 0; i < ndims; i++) {
        char   dimnamebuf[NC_MAX_NAME + 1];
        size_t dimlen;
        rc = nc_inq_dim(_ncid, dimids[i], dimnamebuf, &dimlen);
        MY_NC_ERR(rc, _path, "nc_inq_dim()");
        dimnames.push_back(dimnamebuf);
        dims.push_back(dimlen);
    }
    return (NC_NOERR);
}

int NetCDFCpp::InqDims(vector<string> &dimnames, vector<size_t> &dims) const
{
    dimnames.clear();
    dims.clear();

    int dimids[NC_MAX_DIMS];

    int ndims;
    int rc = nc_inq_dimids(_ncid, &ndims, dimids, 0);
    MY_NC_ERR(rc, _path, "nc_inq_dimids()");

    for (int i = 0; i < ndims; i++) {
        char   dimnamebuf[NC_MAX_NAME + 1];
        size_t dimlen;
        rc = nc_inq_dim(_ncid, dimids[i], dimnamebuf, &dimlen);
        MY_NC_ERR(rc, _path, "nc_inq_dim()");
        dimnames.push_back(dimnamebuf);
        dims.push_back(dimlen);
    }

    return (NC_NOERR);
}

int NetCDFCpp::InqDimlen(string name, size_t &len) const
{
    len = 0;

    int dimid;
    int rc = nc_inq_dimid(_ncid, name.c_str(), &dimid);
    MY_NC_ERR(rc, _path, "nc_inq_dimid(" + name + ")");

    rc = nc_inq_dimlen(_ncid, dimid, &len);
    MY_NC_ERR(rc, _path, "nc_inq_dimlen()");
    return (0);
}

int NetCDFCpp::InqAttnames(string varname, std::vector<string> &attnames) const
{
    attnames.clear();

    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (-1);

    int natts;
    if (!varname.empty()) {
        rc = nc_inq_varnatts(_ncid, varid, &natts);
        MY_NC_ERR(rc, _path, "nc_inq_varnatts()");
    } else {
        rc = nc_inq_natts(_ncid, &natts);
        MY_NC_ERR(rc, _path, "nc_inq_natts()");
    }

    for (int attnum = 0; attnum < natts; attnum++) {
        char namebuf[NC_MAX_NAME + 1];

        rc = nc_inq_attname(_ncid, varid, attnum, namebuf);
        MY_NC_ERR(rc, _path, "nc_inq_attname()");

        attnames.push_back(namebuf);
    }

    return (0);
}

int NetCDFCpp::CopyAtt(string varname_in, string attname, NetCDFCpp &ncdf_out, string varname_out) const
{
    int varid_in;
    int rc = NetCDFCpp::InqVarid(varname_in, varid_in);
    if (rc < 0) return (rc);

    int varid_out;
    rc = ncdf_out.InqVarid(varname_out, varid_out);
    if (rc < 0) return (rc);

    int ncid_out = ncdf_out.GetNCID();
    if (rc < 0) return (rc);

    rc = nc_copy_att(_ncid, varid_in, attname.c_str(), ncid_out, varid_out);
    MY_NC_ERR(rc, _path, "nc_copy_att()");

    return (NC_NOERR);
}

//
// PutAtt - Integer
//
int NetCDFCpp::PutAtt(string varname, string attname, int value) const { return (NetCDFCpp::PutAtt(varname, attname, &value, 1)); }

int NetCDFCpp::PutAtt(string varname, string attname, vector<int> values) const
{
    size_t n = values.size();
    int *  buf = new int[n];
    for (size_t i = 0; i < n; i++) buf[i] = values[i];

    int rc = NetCDFCpp::PutAtt(varname, attname, buf, n);
    delete[] buf;

    return (rc);
}

int NetCDFCpp::PutAtt(string varname, string attname, const int values[], size_t n) const
{
    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    if (attname == "_FillValue") {
        nc_type xtype;
        rc = InqVartype(varname, xtype);
        if (rc < 0) return (rc);

        if (xtype == NC_BYTE) {
            unsigned char *valuesB = new unsigned char[n];
            for (int i = 0; i < n; i++) valuesB[i] = (unsigned char)values[i];
            rc = nc_put_att_ubyte(_ncid, varid, attname.c_str(), NC_BYTE, n, valuesB);
            delete[] valuesB;
        } else if (xtype == NC_SHORT) {
            short *valuesS = new short[n];
            for (int i = 0; i < n; i++) valuesS[i] = (short)values[i];
            rc = nc_put_att_short(_ncid, varid, attname.c_str(), NC_SHORT, n, valuesS);
            delete[] valuesS;
        } else if (xtype == NC_INT64) {
            long *valuesS = new long[n];
            for (int i = 0; i < n; i++) valuesS[i] = (long)values[i];
            rc = nc_put_att_long(_ncid, varid, attname.c_str(), NC_INT64, n, valuesS);
            delete[] valuesS;
        } else {
            rc = nc_put_att_int(_ncid, varid, attname.c_str(), NC_INT, n, values);
        }
    } else {
        rc = nc_put_att_int(_ncid, varid, attname.c_str(), NC_INT, n, values);
    }
    MY_NC_ERR(rc, _path, "nc_put_att_int(" + attname + ")");

    return (NC_NOERR);
}

//
// GetAtt - Integer
//
int NetCDFCpp::GetAtt(string varname, string attname, int &value) const { return (NetCDFCpp::GetAtt(varname, attname, &value, 1)); }

int NetCDFCpp::GetAtt(string varname, string attname, vector<int> &values) const
{
    values.clear();

    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t n;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &n);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    int *buf = new int[n];

    rc = NetCDFCpp::GetAtt(varname, attname, buf, n);

    for (int i = 0; i < n; i++) { values.push_back(buf[i]); }
    delete[] buf;

    return (rc);
}

int NetCDFCpp::GetAtt(string varname, string attname, int values[], size_t n) const
{
    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t len;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &len);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    int *buf = new int[len];

    rc = nc_get_att_int(_ncid, varid, attname.c_str(), buf);
    if (rc != NC_NOERR) delete[] buf;
    MY_NC_ERR(rc, _path, "nc_get_att_int(" + attname + ")");

    for (size_t i = 0; i < len && i < n; i++) { values[i] = buf[i]; }
    delete[] buf;

    return (NC_NOERR);
}

//
// PutAtt - size_t
//
int NetCDFCpp::PutAtt(string varname, string attname, size_t value) const { return (NetCDFCpp::PutAtt(varname, attname, &value, 1)); }

int NetCDFCpp::PutAtt(string varname, string attname, vector<size_t> values) const
{
    vector<int> ivalues;
    for (size_t i = 0; i < values.size(); i++) ivalues.push_back(values[i]);

    return (NetCDFCpp::PutAtt(varname, attname, ivalues));
}

int NetCDFCpp::PutAtt(string varname, string attname, const size_t values[], size_t n) const
{
    vector<int> ivalues;

    for (size_t i = 0; i < n; i++) ivalues.push_back(values[i]);

    return (NetCDFCpp::PutAtt(varname, attname, ivalues));
}

//
// GetAtt - size_t
//
int NetCDFCpp::GetAtt(string varname, string attname, size_t &value) const { return (NetCDFCpp::GetAtt(varname, attname, &value, 1)); }

int NetCDFCpp::GetAtt(string varname, string attname, vector<size_t> &values) const
{
    values.clear();

    vector<int> ivalues;
    int         rc = NetCDFCpp::GetAtt(varname, attname, ivalues);
    for (int i = 0; i < ivalues.size(); i++) values.push_back(ivalues[i]);
    return (rc);
}

int NetCDFCpp::GetAtt(string varname, string attname, size_t values[], size_t n) const
{
    vector<int> ivalues;
    int         rc = NetCDFCpp::GetAtt(varname, attname, ivalues);
    for (int i = 0; i < ivalues.size() && i < n; i++) values[i] = ivalues[i];

    return (rc);
}

//
// PutAtt - Float
//
int NetCDFCpp::PutAtt(string varname, string attname, float value) const { return (NetCDFCpp::PutAtt(varname, attname, &value, 1)); }

int NetCDFCpp::PutAtt(string varname, string attname, vector<float> values) const
{
    size_t n = values.size();
    float *buf = new float[n];
    for (size_t i = 0; i < n; i++) buf[i] = values[i];

    int rc = NetCDFCpp::PutAtt(varname, attname, buf, n);
    delete[] buf;

    return (rc);
}

int NetCDFCpp::PutAtt(string varname, string attname, const float values[], size_t n) const
{
    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    rc = nc_put_att_float(_ncid, varid, attname.c_str(), NC_DOUBLE, n, values);
    MY_NC_ERR(rc, _path, "nc_put_att_float(" + attname + ")");

    return (NC_NOERR);
}

//
// PutAtt - Double
//
int NetCDFCpp::PutAtt(string varname, string attname, double value) const { return (NetCDFCpp::PutAtt(varname, attname, &value, 1)); }

int NetCDFCpp::PutAtt(string varname, string attname, vector<double> values) const
{
    size_t  n = values.size();
    double *buf = new double[n];
    for (size_t i = 0; i < n; i++) buf[i] = values[i];

    int rc = NetCDFCpp::PutAtt(varname, attname, buf, n);
    delete[] buf;

    return (rc);
}

int NetCDFCpp::PutAtt(string varname, string attname, const double values[], size_t n) const
{
    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    if (attname == "_FillValue") {
        nc_type xtype;
        rc = InqVartype(varname, xtype);
        if (rc < 0) return (rc);

        if (xtype == NC_FLOAT) {
            float *valuesF = new float[n];
            for (int i = 0; i < n; i++) valuesF[i] = (float)values[i];
            rc = nc_put_att_float(_ncid, varid, attname.c_str(), NC_FLOAT, n, valuesF);
            delete[] valuesF;
        } else {
            rc = nc_put_att_double(_ncid, varid, attname.c_str(), NC_DOUBLE, n, values);
        }
    } else {
        rc = nc_put_att_double(_ncid, varid, attname.c_str(), NC_DOUBLE, n, values);
    }

    MY_NC_ERR(rc, _path, "nc_put_att_double(" + attname + ")");

    return (NC_NOERR);
}

//
// GetAtt - Float
//
int NetCDFCpp::GetAtt(string varname, string attname, float &value) const { return (NetCDFCpp::GetAtt(varname, attname, &value, 1)); }

int NetCDFCpp::GetAtt(string varname, string attname, vector<float> &values) const
{
    values.clear();

    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t n;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &n);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    float *buf = new float[n];

    rc = NetCDFCpp::GetAtt(varname, attname, buf, n);

    for (int i = 0; i < n; i++) { values.push_back(buf[i]); }
    delete[] buf;

    return (rc);
}

int NetCDFCpp::GetAtt(string varname, string attname, float values[], size_t n) const
{
    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t len;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &len);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    float *buf = new float[len];

    rc = nc_get_att_float(_ncid, varid, attname.c_str(), buf);
    if (rc != NC_NOERR) delete[] buf;
    MY_NC_ERR(rc, _path, "nc_get_att_float(" + attname + ")");

    for (size_t i = 0; i < len && i < n; i++) { values[i] = buf[i]; }
    delete[] buf;

    return (NC_NOERR);
}

//
// GetAtt - Double
//
int NetCDFCpp::GetAtt(string varname, string attname, double &value) const { return (NetCDFCpp::GetAtt(varname, attname, &value, 1)); }

int NetCDFCpp::GetAtt(string varname, string attname, vector<double> &values) const
{
    values.clear();

    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t n;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &n);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    double *buf = new double[n];

    rc = NetCDFCpp::GetAtt(varname, attname, buf, n);

    for (int i = 0; i < n; i++) { values.push_back(buf[i]); }
    delete[] buf;

    return (rc);
}

int NetCDFCpp::GetAtt(string varname, string attname, double values[], size_t n) const
{
    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t len;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &len);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    double *buf = new double[len];

    rc = nc_get_att_double(_ncid, varid, attname.c_str(), buf);
    if (rc != NC_NOERR) delete[] buf;
    MY_NC_ERR(rc, _path, "nc_get_att_double(" + attname + ")");

    for (size_t i = 0; i < len && i < n; i++) { values[i] = buf[i]; }
    delete[] buf;

    return (NC_NOERR);
}

//
// PutAtt - String
//

int NetCDFCpp::PutAtt(string varname, string attname, string value) const
{
    size_t n = value.length();
    char * buf = new char[n + 1];
    strcpy(buf, value.c_str());
    int rc = NetCDFCpp::PutAtt(varname, attname, buf, n);
    delete[] buf;
    return (rc);
}
int NetCDFCpp::PutAtt(string varname, string attname, vector<string> values) const
{
    string s;
    for (int i = 0; i < values.size(); i++) {
        s += values[i];
        s += " ";
    }
    return (NetCDFCpp::PutAtt(varname, attname, s));
}

int NetCDFCpp::PutAtt(string varname, string attname, const char values[], size_t n) const
{
    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    rc = nc_put_att_text(_ncid, varid, attname.c_str(), n, values);
    MY_NC_ERR(rc, _path, "nc_put_att_text(" + attname + ")");

    return (NC_NOERR);
}

//
// GetAtt - String
//
int NetCDFCpp::GetAtt(string varname, string attname, string &value) const
{
    value.clear();

    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t n;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &n);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    char *buf = new char[n + 1];

    rc = NetCDFCpp::GetAtt(varname, attname, buf, n);
    value = buf;

    delete[] buf;

    return (rc);
}

int NetCDFCpp::GetAtt(string varname, string attname, char values[], size_t n) const
{
    int varid;
    int rc = InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t len;
    rc = nc_inq_attlen(_ncid, varid, attname.c_str(), &len);
    MY_NC_ERR(rc, _path, "nc_inq_attlen(" + attname + ")");

    char *buf = new char[len + 1];

    rc = nc_get_att_text(_ncid, varid, attname.c_str(), buf);
    if (rc != NC_NOERR) delete[] buf;
    MY_NC_ERR(rc, _path, "nc_get_att_text(" + attname + ")");

    size_t i;
    for (i = 0; i < len && i < n; i++) { values[i] = buf[i]; }
    values[i] = '\0';
    delete[] buf;

    return (NC_NOERR);
}

int NetCDFCpp::GetAtt(string varname, string attname, vector<string> &values) const
{
    values.clear();

    string s;

    int rc = NetCDFCpp::GetAtt(varname, attname, s);
    if (rc < 0) return (rc);

    string       buf;
    stringstream ss(s);
    while (ss >> buf) values.push_back(buf);

    return (0);
}

int NetCDFCpp::InqVarid(string varname, int &varid) const
{
    if (varname.empty()) {
        varid = NC_GLOBAL;
        return (NC_NOERR);
    }
    int my_varid = -1;
    int rc = nc_inq_varid(_ncid, varname.c_str(), &my_varid);
    MY_NC_ERR(rc, _path, "nc_inq_varid(" + varname + ")");

    varid = my_varid;

    return (NC_NOERR);
}

int NetCDFCpp::InqAtt(string varname, string attname, nc_type &xtype, size_t &len) const
{
    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    rc = nc_inq_att(_ncid, varid, attname.c_str(), &xtype, &len);
    MY_NC_ERR(rc, _path, "nc_inq_att(" + attname + ")");

    return (NC_NOERR);
}

int NetCDFCpp::InqVartype(string varname, nc_type &xtype) const
{
    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    rc = nc_inq_vartype(_ncid, varid, &xtype);
    MY_NC_ERR(rc, _path, "nc_inq_vartype()");

    return (NC_NOERR);
}

size_t NetCDFCpp::SizeOf(nc_type xtype)
{
    switch (xtype) {
    case NC_BYTE:
    case NC_UBYTE:
    case NC_CHAR: return (1);
    case NC_SHORT:
    case NC_USHORT: return (2);
    case NC_INT:    // NC_LONG and NC_INT
    case NC_UINT:
    case NC_FLOAT: return (4);
    case NC_INT64:
    case NC_UINT64:
    case NC_DOUBLE: return (8);
    default: return (0);
    }
}

bool NetCDFCpp::ValidFile(string path)
{
    bool valid = false;

    int ncid;
    int rc = nc_open(path.c_str(), 0, &ncid);
    if (rc == NC_NOERR) {
        valid = true;
        nc_close(ncid);
    }
    return (valid);
}

int NetCDFCpp::_PutVara(string varname, vector<size_t> start, vector<size_t> count, const void *data, string func)
{
    assert(start.size() == count.size());

    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];

    for (int i = 0; i < start.size(); i++) {
        mystart[i] = start[i];
        mycount[i] = count[i];
    }

    if (func == "nc_put_vara") {
        // The nc_put_var() function will write a variable of any type,
        // including
        // user defined type. For this function, the type of the data in memory
        // must match the type of the variable - no data conversion is done.
        //
        rc = nc_put_vara(_ncid, varid, mystart, mycount, (const void *)data);
    } else if (func == "nc_put_vara_float") {
        rc = nc_put_vara_float(_ncid, varid, mystart, mycount, (const float *)data);
    } else if (func == "nc_put_vara_double") {
        rc = nc_put_vara_double(_ncid, varid, mystart, mycount, (const double *)data);
    } else if (func == "nc_put_vara_int") {
        rc = nc_put_vara_int(_ncid, varid, mystart, mycount, (const int *)data);
    } else if (func == "nc_put_vara_long") {
        rc = nc_put_vara_long(_ncid, varid, mystart, mycount, (const long *)data);
    } else if (func == "nc_put_vara_uchar") {
        rc = nc_put_vara_uchar(_ncid, varid, mystart, mycount, (const unsigned char *)data);
    } else {
        assert(func == "");
    }
    MY_NC_ERR(rc, _path, func);

    return (0);
}

int NetCDFCpp::PutVara(string varname, vector<size_t> start, vector<size_t> count, const void *data) { return (_PutVara(varname, start, count, (const void *)data, "nc_put_vara")); }

int NetCDFCpp::PutVara(string varname, vector<size_t> start, vector<size_t> count, const float *data) { return (_PutVara(varname, start, count, (const void *)data, "nc_put_vara_float")); }

int NetCDFCpp::PutVara(string varname, vector<size_t> start, vector<size_t> count, const double *data) { return (_PutVara(varname, start, count, (const void *)data, "nc_put_vara_double")); }

int NetCDFCpp::PutVara(string varname, vector<size_t> start, vector<size_t> count, const int *data) { return (_PutVara(varname, start, count, (const void *)data, "nc_put_vara_int")); }

int NetCDFCpp::PutVara(string varname, vector<size_t> start, vector<size_t> count, const long *data) { return (_PutVara(varname, start, count, (const void *)data, "nc_put_vara_long")); }

int NetCDFCpp::PutVara(string varname, vector<size_t> start, vector<size_t> count, const unsigned char *data) { return (_PutVara(varname, start, count, (const void *)data, "nc_put_vara_uchar")); }

int NetCDFCpp::_PutVar(string varname, const void *data, string func)
{
    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    if (func == "nc_put_var") {
        // The nc_put_var() function will write a variable of any type,
        // including
        // user defined type. For this function, the type of the data in memory
        // must match the type of the variable - no data conversion is done.
        //
        rc = nc_put_var(_ncid, varid, (const void *)data);
    } else if (func == "nc_put_var_float") {
        rc = nc_put_var_float(_ncid, varid, (const float *)data);
    } else if (func == "nc_put_var_double") {
        rc = nc_put_var_double(_ncid, varid, (const double *)data);
    } else if (func == "nc_put_var_int") {
        rc = nc_put_var_int(_ncid, varid, (const int *)data);
    } else if (func == "nc_put_var_long") {
        rc = nc_put_var_long(_ncid, varid, (const long *)data);
    } else if (func == "nc_put_var_uchar") {
        rc = nc_put_var_uchar(_ncid, varid, (const unsigned char *)data);
    } else {
        assert(func == "");
    }
    MY_NC_ERR(rc, _path, func);

    return (0);
}

int NetCDFCpp::PutVar(string varname, const void *data) { return (_PutVar(varname, (const void *)data, "nc_put_var")); }

int NetCDFCpp::PutVar(string varname, const float *data) { return (_PutVar(varname, (const void *)data, "nc_put_var_float")); }

int NetCDFCpp::PutVar(string varname, const double *data) { return (_PutVar(varname, (const void *)data, "nc_put_var_double")); }

int NetCDFCpp::PutVar(string varname, const int *data) { return (_PutVar(varname, (const void *)data, "nc_put_var_int")); }

int NetCDFCpp::PutVar(string varname, const long *data) { return (_PutVar(varname, (const void *)data, "nc_put_var_long")); }

int NetCDFCpp::PutVar(string varname, const unsigned char *data) { return (_PutVar(varname, (const void *)data, "nc_put_var_uchar")); }

int NetCDFCpp::_GetVara(string varname, vector<size_t> start, vector<size_t> count, void *data, string func) const
{
    assert(start.size() == count.size());

    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    size_t mystart[NC_MAX_VAR_DIMS];
    size_t mycount[NC_MAX_VAR_DIMS];

    for (int i = 0; i < start.size(); i++) {
        mystart[i] = start[i];
        mycount[i] = count[i];
    }

    if (func == "nc_get_vara") {
        // The nc_get_var() function will write a variable of any type,
        // including
        // user defined type. For this function, the type of the data in memory
        // must match the type of the variable - no data conversion is done.
        //
        rc = nc_get_vara(_ncid, varid, mystart, mycount, (void *)data);
    } else if (func == "nc_get_vara_float") {
        rc = nc_get_vara_float(_ncid, varid, mystart, mycount, (float *)data);
    } else if (func == "nc_get_vara_double") {
        rc = nc_get_vara_double(_ncid, varid, mystart, mycount, (double *)data);
    } else if (func == "nc_get_vara_int") {
        rc = nc_get_vara_int(_ncid, varid, mystart, mycount, (int *)data);
    } else if (func == "nc_get_vara_long") {
        rc = nc_get_vara_long(_ncid, varid, mystart, mycount, (long *)data);
    } else if (func == "nc_get_vara_uchar") {
        rc = nc_get_vara_uchar(_ncid, varid, mystart, mycount, (unsigned char *)data);
    } else {
        assert(func == "");
    }
    MY_NC_ERR(rc, _path, func);

    return (rc);
}

int NetCDFCpp::GetVara(string varname, vector<size_t> start, vector<size_t> count, void *data) const { return (_GetVara(varname, start, count, (void *)data, "nc_get_vara")); }

int NetCDFCpp::GetVara(string varname, vector<size_t> start, vector<size_t> count, float *data) const { return (_GetVara(varname, start, count, (void *)data, "nc_get_vara_float")); }

int NetCDFCpp::GetVara(string varname, vector<size_t> start, vector<size_t> count, double *data) const { return (_GetVara(varname, start, count, (void *)data, "nc_get_vara_double")); }

int NetCDFCpp::GetVara(string varname, vector<size_t> start, vector<size_t> count, int *data) const { return (_GetVara(varname, start, count, (void *)data, "nc_get_vara_int")); }

int NetCDFCpp::GetVara(string varname, vector<size_t> start, vector<size_t> count, long *data) const { return (_GetVara(varname, start, count, (void *)data, "nc_get_vara_long")); }

int NetCDFCpp::GetVara(string varname, vector<size_t> start, vector<size_t> count, unsigned char *data) const { return (_GetVara(varname, start, count, (void *)data, "nc_get_vara_uchar")); }

int NetCDFCpp::_GetVar(string varname, void *data, string func) const
{
    int varid;
    int rc = NetCDFCpp::InqVarid(varname, varid);
    if (rc < 0) return (rc);

    if (func == "nc_get_var") {
        // The nc_get_var() function will write a variable of any type,
        // including
        // user defined type. For this function, the type of the data in memory
        // must match the type of the variable - no data conversion is done.
        //
        rc = nc_get_var(_ncid, varid, (void *)data);
    } else if (func == "nc_get_var_float") {
        rc = nc_get_var_float(_ncid, varid, (float *)data);
    } else if (func == "nc_get_var_double") {
        rc = nc_get_var_double(_ncid, varid, (double *)data);
    } else if (func == "nc_get_var_int") {
        rc = nc_get_var_int(_ncid, varid, (int *)data);
    } else if (func == "nc_get_var_long") {
        rc = nc_get_var_long(_ncid, varid, (long *)data);
    } else if (func == "nc_get_var_uchar") {
        rc = nc_get_var_uchar(_ncid, varid, (unsigned char *)data);
    } else {
        assert(func == "");
    }
    MY_NC_ERR(rc, _path, func);

    return (0);
}

int NetCDFCpp::GetVar(string varname, void *data) const { return (_GetVar(varname, (void *)data, "nc_get_var")); }

int NetCDFCpp::GetVar(string varname, float *data) const { return (_GetVar(varname, (void *)data, "nc_get_var_float")); }

int NetCDFCpp::GetVar(string varname, double *data) const { return (_GetVar(varname, (void *)data, "nc_get_var_double")); }

int NetCDFCpp::GetVar(string varname, int *data) const { return (_GetVar(varname, (void *)data, "nc_get_var_int")); }

int NetCDFCpp::GetVar(string varname, long *data) const { return (_GetVar(varname, (void *)data, "nc_get_var_long")); }

int NetCDFCpp::GetVar(string varname, unsigned char *data) const { return (_GetVar(varname, (void *)data, "nc_get_var_uchar")); }

int NetCDFCpp::CopyVar(string varname, NetCDFCpp &ncdf_out) const
{
    nc_type xtype;
    int     rc = NetCDFCpp::InqVartype(varname, xtype);
    if (rc < 0) return (-1);

    size_t elem_size = NetCDFCpp::SizeOf(xtype);
    assert(elem_size != 0);

    vector<string> dimnames;
    vector<size_t> dims;
    rc = NetCDFCpp::InqVarDims(varname, dimnames, dims);
    if (rc < 0) return (-1);

    size_t nelements = 1;
    for (int i = 0; i < dims.size(); i++) nelements *= dims[i];

    unsigned char *data = new unsigned char[nelements * elem_size];

    rc = NetCDFCpp::GetVar(varname, (void *)data);
    if (rc < 0) {
        delete[] data;
        return (-1);
    }

    rc = ncdf_out.PutVar(varname, (void *)data);
    if (rc < 0) {
        delete[] data;
        return (-1);
    }

    delete[] data;
    return (NC_NOERR);
}

bool NetCDFCpp::InqDimDefined(string dimname)
{
    int dummy;
    int rc = nc_inq_dimid(_ncid, dimname.c_str(), &dummy);

    if (rc == NC_NOERR) return (true);

    return (false);
}

bool NetCDFCpp::InqAttDefined(string varname, string attname)
{
    int varid = -1;
    int rc = nc_inq_varid(_ncid, varname.c_str(), &varid);
    if (rc != NC_NOERR) return (false);

    int dummy;
    rc = nc_inq_attid(_ncid, varid, attname.c_str(), &dummy);

    if (rc == NC_NOERR) return (true);

    return (false);
}

int NetCDFCpp::InqVarnames(vector<string> &varnames) const
{
    varnames.clear();

    int ndims, nvars, natts, unlimitedid;

    int rc = nc_inq(_ncid, &ndims, &nvars, &natts, &unlimitedid);
    MY_NC_ERR(rc, _path, "nc_inq()");

    for (int varid = 0; varid < nvars; varid++) {
        char    namebuf[NC_MAX_NAME + 1];
        nc_type xtype;
        int     dimids[NC_MAX_VAR_DIMS];

        rc = nc_inq_var(_ncid, varid, namebuf, &xtype, &ndims, dimids, &natts);
        MY_NC_ERR(rc, _path, "nc_inq_var()");

        varnames.push_back(namebuf);
    }
    return (0);
}
