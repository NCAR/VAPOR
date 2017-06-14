#include <iostream>
#include <cassert>
#include <netcdf.h>
#include <vapor/NetCDFSimple.h>

using namespace VAPoR;
using namespace Wasp;
using namespace std;

NetCDFSimple::NetCDFSimple() {
	_ncid = -1;
	_ovr_table.clear();
	_path = "";	// so _path.c_str() returns an empty string
	_chsz = 4*1024*1024;
	_dimnames.clear();
	_dims.clear();
	_unlimited_dimnames.clear();
	_flt_atts.clear();
	_int_atts.clear();
	_str_atts.clear();
	_variables.clear();
}

NetCDFSimple::~NetCDFSimple() {

	if (_ncid != -1)  {
		int rc = nc_close(_ncid);
		if (rc != 0) {
			SetErrMsg("nc_close(%d) : %s", _ncid, nc_strerror(rc));
			return;
		}
	}
}

int NetCDFSimple::Initialize(string path)
{
	_dimnames.clear();
	_dims.clear();
	_unlimited_dimnames.clear();
	_flt_atts.clear();
	_int_atts.clear();
	_str_atts.clear();
	_variables.clear();
	_path = path;
	
	size_t chsz = _chsz;
	int ncid;
	int rc = nc__open(path.c_str(), NC_NOWRITE, &chsz, &ncid);
	if (rc != 0) {
		SetErrMsg("nc__open(%s,) : %s", path.c_str(), nc_strerror(rc));
		return(-1);
	}

	int ndims;
	rc = nc_inq_ndims(ncid, &ndims);
	if (rc != 0) {
		SetErrMsg("nc_inq_ndims(%d) : %s", ncid, nc_strerror(rc));
		return(-1);
	}

	//
	// Get all the dimensions
	//
	for (int i=0; i<ndims; i++) {
		char namebuf[NC_MAX_NAME+1];
		size_t len;
		rc = nc_inq_dim(ncid, i, namebuf, &len);
		if (rc!=0) {
			SetErrMsg("nc_inq_dim(%d, %d) : %s", ncid, i, nc_strerror(rc));
			return(-1);
		}
		_dimnames.push_back(namebuf);
		_dims.push_back(len);
	}

	//
	// Get unlimited dim. N.B. in netCDF-4/HDF5 there can be multiple
	// unlimited dimensions. Here we only check for one!
	//
	int dimid;
	rc = nc_inq_unlimdim(ncid, &dimid);	
	if (rc!=0) {
		SetErrMsg("nc_inq_unlimdim(%d) : %s", ncid, nc_strerror(rc));
		return(-1);
	}
	if (dimid >= 0) _unlimited_dimnames.push_back(_dimnames[dimid]);
	

	//
	// Get all the global attributes
	//
	rc = _GetAtts(ncid, NC_GLOBAL, _flt_atts, _int_atts, _str_atts);
	if (rc<0) return(-1);


	//
	// Finally, get all of the variable metadata
	//
	int nvars;
	rc = nc_inq_nvars(ncid, &nvars);
	if (rc!=0) {
		SetErrMsg("nc_inq_nvars(%d) : %s", ncid, nc_strerror(rc));
		return(-1);
	}

	for (int varid=0; varid<nvars; varid++) {
		char namebuf[NC_MAX_NAME+1];
		nc_type xtype;
		int ndims;
		int dimids[NC_MAX_VAR_DIMS];
		int natts;

		rc = nc_inq_var(ncid, varid, namebuf, &xtype, &ndims, dimids, &natts);
		if (rc!=0) {
			SetErrMsg(
				"nc_inq_var(%d, %d, %d) : %s", 
				ncid, varid, namebuf, nc_strerror(rc)
			);
			return(-1);
		}
		vector <string> dimnames;
		vector <size_t> dims;
		for (int i=0; i<ndims; i++) {
			dimnames.push_back(DimName(dimids[i]));
			dims.push_back(_dims[dimids[i]]);
		}

		Variable var(namebuf, dimnames, varid, xtype);

		vector <pair <string, vector <double> > > flt_atts;
		vector <pair <string, vector <long> > > int_atts;
		vector <pair <string, string> > str_atts;
		rc = _GetAtts(ncid, varid, flt_atts, int_atts, str_atts);
		if (rc<0) return(-1);
		
		for (int i=0; i<flt_atts.size(); i++) {
			var.SetAtt(flt_atts[i].first, flt_atts[i].second);
		}
		for (int i=0; i<int_atts.size(); i++) {
			var.SetAtt(int_atts[i].first, int_atts[i].second);
		}
		for (int i=0; i<str_atts.size(); i++) {
			var.SetAtt(str_atts[i].first, str_atts[i].second);
		}

		_variables.push_back(var);

	}


	nc_close(ncid);
	return(0);
}

int NetCDFSimple::OpenRead(
	const NetCDFSimple::Variable &variable
) {
	//
	// If _ncid is not valid open the NetCDF file
	//
	if (_ncid == -1) {
		size_t chsz = _chsz;
		int ncid;
		int rc = nc__open(_path.c_str(), NC_NOWRITE, &chsz, &ncid);
		if (rc != 0) {
			SetErrMsg("nc__open(%s,) : %s", _path.c_str(), nc_strerror(rc));
			return(-1);
		}
		_ncid = ncid;
	}

	//
	// Find a file descriptor. Use lowest available, starting with zero
	//
	int fd;
	for (fd=0; fd<_ovr_table.size(); fd++) {
		if (_ovr_table.find(fd) == _ovr_table.end()) {
			break;
		}
	}
	_ovr_table[fd] = variable.GetVarID();

	return(fd);
}

int NetCDFSimple::Read(
	const size_t start[], const size_t count[], float *data, int fd
) const  {

	std::map <int, int>::const_iterator itr;
	if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
		SetErrMsg("Invalid file descriptor : %d", fd);
		return(-1);
	}
	int varid = itr->second;

	int rc = nc_get_vara_float(
		_ncid, varid, start, count, data
	);
	if (rc != 0) {
		SetErrMsg(
			"nc_get_vara_float(%d, %d) : %s", _ncid, varid,
			nc_strerror(rc)
		);
		return(-1);
	}
	return(0);
}

int NetCDFSimple::Read(
	const size_t start[], const size_t count[], int *data, int fd
) const  {

	std::map <int, int>::const_iterator itr;
	if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
		SetErrMsg("Invalid file descriptor : %d", fd);
		return(-1);
	}
	int varid = itr->second;

	int rc = nc_get_vara_int(
		_ncid, varid, start, count, data
	);
	if (rc != 0) {
		SetErrMsg(
			"nc_get_vara_int(%d, %d) : %s", _ncid, varid,
			nc_strerror(rc)
		);
		return(-1);
	}
	return(0);
}

int NetCDFSimple::Read(
	const size_t start[], const size_t count[], char *data, int fd
) const  {

	std::map <int, int>::const_iterator itr;
	if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
		SetErrMsg("Invalid file descriptor : %d", fd);
		return(-1);
	}
	int varid = itr->second;

	int rc = nc_get_vara_text(
		_ncid, varid, start, count, data
	);
	if (rc != 0) {
		SetErrMsg(
			"nc_get_vara_text(%d, %d) : %s", _ncid, varid,
			nc_strerror(rc)
		);
		return(-1);
	}
	return(0);
}

int NetCDFSimple::Close(int fd) {
	std::map <int, int>::iterator itr;
	if ((itr = _ovr_table.find(fd)) == _ovr_table.end()) {
		SetErrMsg("Invalid file descriptor : %d", fd);
		return(-1);
	}

	_ovr_table.erase(itr);

	return(0);
}



void NetCDFSimple::GetDimensions(
	vector <string> &names, vector <size_t> &dims
) const {
	names = _dimnames;
	dims = _dims;
}

string NetCDFSimple::DimName(int id) const {
	if (id>=0 && id < _dimnames.size()) return (_dimnames[id]);

	return("");
}

size_t NetCDFSimple::DimLen(string name) const {
	vector <string> names;
	vector <size_t> dims;
	assert(dims.size() == names.size());

	NetCDFSimple::GetDimensions(names, dims);
	for (int i=0; i<names.size(); i++) {
		if (names[i].compare(name) == 0) return(dims[i]);
	}
	return(0);	// unknown name
}

int NetCDFSimple::DimId(string name) const {
	for (int i=0; i<_dimnames.size(); i++) {
		if (_dimnames[i].compare(name) == 0) return(i);
	}
	return(-1);
}

std::vector <string> NetCDFSimple::GetAttNames() const {
	vector <string> names;

	for (int i=0; i<_flt_atts.size(); i++) {
		names.push_back( _flt_atts[i].first);
	}
	for (int i=0; i<_int_atts.size(); i++) {
		names.push_back( _int_atts[i].first);
	}
	for (int i=0; i<_str_atts.size(); i++) {
		names.push_back( _str_atts[i].first);
	}
	return(names);
}

int NetCDFSimple::GetAttType(string name) const {
	for (int i=0; i<_flt_atts.size(); i++) {
		if (name.compare(_flt_atts[i].first) == 0) return(NC_DOUBLE);
	}
	for (int i=0; i<_int_atts.size(); i++) {
		if (name.compare(_int_atts[i].first) == 0) return(NC_INT64);
	}
	for (int i=0; i<_str_atts.size(); i++) {
		if (name.compare(_str_atts[i].first) == 0) return(NC_CHAR);
	}
	return(-1);
}

void NetCDFSimple::GetAtt(string name, vector <double> &values) const {

	values.clear();
	for (int i=0; i<_flt_atts.size(); i++) {
		if (_flt_atts[i].first.compare(name) == 0) {
			values = _flt_atts[i].second;
			return;
		}
	}
	//
	// Look for atts of type int and then cast to float if found
	//
	for (int i=0; i<_int_atts.size(); i++) {
		if (_int_atts[i].first.compare(name) == 0) {
			for (int j=0; j<_int_atts[i].second.size(); j++) {
				values.push_back(_int_atts[i].second[j]);
			}
			return;
		}
	}
	return;
}
void NetCDFSimple::GetAtt(string name, vector <long> &values) const {

	values.clear();
	for (int i=0; i<_int_atts.size(); i++) {
		if (_int_atts[i].first.compare(name) == 0) {
			values = _int_atts[i].second;
			return;
		}
	}

	//
	// Look for atts of type float and then cast to int if found
	//
	for (int i=0; i<_flt_atts.size(); i++) {
		if (_flt_atts[i].first.compare(name) == 0) {
			for (int j=0; j<_flt_atts[i].second.size(); j++) {
				values.push_back((long) _flt_atts[i].second[j]);
			}
			return;
		}
	}
	return;
}

void NetCDFSimple::GetAtt(string name, string &values) const {

	values.clear();
	for (int i=0; i<_str_atts.size(); i++) {
		if (_str_atts[i].first.compare(name) == 0) {
			values = _str_atts[i].second;
			return;
		}
	}
	return;
}


bool NetCDFSimple::IsNCTypeInt(int type) {
	if (
		type == NC_BYTE || type == NC_SHORT || type == NC_INT ||
		type == NC_LONG || type == NC_UBYTE || type == NC_USHORT ||
		type == NC_UINT || type == NC_INT64 || type == NC_UINT64
	) {
		return(true);
	}

	return(false);
}

bool NetCDFSimple::IsNCTypeFloat(int type) {

	if (type == NC_FLOAT || type == NC_DOUBLE) {
		return(true);
	}

	return(false);
}

bool NetCDFSimple::IsNCTypeText(int type) {

	if (type == NC_CHAR) {
		return(true);
	}

	return(false);
}


int NetCDFSimple::_GetAtts(
	int ncid, int varid,
	vector <pair <string, vector <double> > > &flt_atts,
	vector <pair <string, vector <long> > > &int_atts,
	vector <pair <string, string> > &str_atts
) {
	flt_atts.clear();
	int_atts.clear();
	str_atts.clear();

	int rc;
	int natts;
	if (varid == NC_GLOBAL) {
		rc = nc_inq_natts(ncid, &natts);
	}
	else {
		rc = nc_inq_varnatts(ncid, varid, &natts);
	}
	if (rc!=0) {
		SetErrMsg("nc_inq_varnatts(%d, %d) : %s", ncid, varid, nc_strerror(rc));
		return(-1);
	}

	double *dblbuf = NULL;
	size_t dblbufsz = 0;
	
	long *longbuf = NULL;
	size_t longbufsz = 0;

	char *textbuf = NULL;
	size_t textbufsz = 0;

	for (int i=0; i<natts; i++) {
		char namebuf[NC_MAX_NAME+1];
		rc = nc_inq_attname(ncid, varid, i, namebuf);
		if (rc!=0) {
			SetErrMsg(
				"nc_inq_attname(%d, %d, %d) : %s", 
				ncid, varid, i, nc_strerror(rc)
			);
			return(-1);
		}

		nc_type xtype;
		size_t len;
		rc = nc_inq_att(ncid, varid, namebuf, &xtype, &len);
		if (rc!=0) {
			SetErrMsg(
				"nc_inq_att(%d, %d, %s) : %s", 
				ncid, varid, namebuf, nc_strerror(rc)
			);
			return(-1);
		}

		else if (IsNCTypeInt(xtype)) {

			if (longbufsz < len) {
				if (longbuf) delete [] longbuf;
				longbuf = new long[len];
				longbufsz = len;
			}
			rc = nc_get_att_long(ncid, varid, namebuf, longbuf);
			if (rc!=0) {
				SetErrMsg(
					"nc_get_att_long(%d, %d, %s) : %s", 
					ncid, varid, namebuf, nc_strerror(rc)
				);
				return(-1);
			}

			vector <long> vals;
			for (int i=0; i<len; i++) {
				vals.push_back(longbuf[i]);
			}
			int_atts.push_back(make_pair(namebuf, vals));
		}
		else if (IsNCTypeFloat(xtype)) {

			if (dblbufsz < len) {
				if (dblbuf) delete [] dblbuf;
				dblbuf = new double[len];
				dblbufsz = len;
			}
			rc = nc_get_att_double(ncid, varid, namebuf, dblbuf);
			if (rc!=0) {
				SetErrMsg(
					"nc_get_att_double(%d, %d, %s) : %s", 
					ncid, varid, namebuf, nc_strerror(rc)
				);
				return(-1);
			}

			vector <double> vals;
			for (int i=0; i<len; i++) {
				vals.push_back(dblbuf[i]);
			}
			flt_atts.push_back(make_pair(namebuf, vals));

		}
		else if (IsNCTypeText(xtype)) {

			if (textbufsz < len+1) {
				if (textbuf) delete [] textbuf;
				textbuf = new char[len+1];
				textbufsz = len+1;
			}

			rc = nc_get_att_text(ncid, varid, namebuf, textbuf);
			if (rc!=0) {
				SetErrMsg(
					"nc_get_att_text(%d, %d, %s) : %s", 
					ncid, varid, namebuf, nc_strerror(rc)
				);
				return(-1);
			}
			textbuf[len] = '\0';	// need to null terminate - sigh :-(

			string val = textbuf;
			str_atts.push_back(make_pair(namebuf, val));
		
		}
		else {
			SetErrMsg("Unhandled attribute type : %d", xtype);
			return(-1);

		}

	}
	if (longbuf) delete [] longbuf;
	if (dblbuf) delete [] dblbuf;
	if (textbuf) delete [] textbuf;

	return(0);
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const NetCDFSimple &nc) {
	o << "NetCDFSimple" << endl;
	o << " File : " << nc._path << endl;
	o << " Dimensions : " << endl;
	for (int i=0; i<nc._dimnames.size(); i++) {
		o << "  " << nc._dimnames[i] << " " << nc._dims[i] << endl;
	}
	o << " Unlimited Dimensions : " << endl;
	for (int i=0; i<nc._unlimited_dimnames.size(); i++) {
		o << "  " << nc._unlimited_dimnames[i] << endl;
	}

	o << " Attributes : " << endl;
	for (int i=0; i<nc._flt_atts.size(); i++) {
		o << "  " << nc._flt_atts[i].first << ": ";
		for (int j=0; j<nc._flt_atts[i].second.size(); j++) {
			o << nc._flt_atts[i].second[j] << " ";
		}
		o << endl;
	}
	for (int i=0; i<nc._int_atts.size(); i++) {
		o << "  " << nc._int_atts[i].first << ": ";
		for (int j=0; j<nc._int_atts[i].second.size(); j++) {
			o << nc._int_atts[i].second[j] << " ";
		}
		o << endl;
	}
	for (int i=0; i<nc._str_atts.size(); i++) {
		o << "  " << nc._str_atts[i].first << ": ";
		o << nc._str_atts[i].second << endl;
	}

	for (int i=0; i<nc._variables.size(); i++) {
		o << nc._variables[i];
	}
	
	return(o);
}
}

NetCDFSimple::Variable::Variable() {
	_name.clear();
	_dimnames.clear();
	_flt_atts.clear();
	_int_atts.clear();
	_str_atts.clear();
	_type = -1;
	_varid = -1;
}

NetCDFSimple::Variable::Variable(
	string name, vector <string> dimnames, 
	int varid, int type
) {
	_name = name;
	_dimnames = dimnames;
	_varid = varid;
	_type = type;
	_flt_atts.clear();
	_int_atts.clear();
	_str_atts.clear();
}

vector <string> NetCDFSimple::Variable::GetAttNames() const {
	vector <string> names;

	for (int i=0; i<_flt_atts.size(); i++) {
		names.push_back( _flt_atts[i].first);
	}
	for (int i=0; i<_int_atts.size(); i++) {
		names.push_back( _int_atts[i].first);
	}
	for (int i=0; i<_str_atts.size(); i++) {
		names.push_back( _str_atts[i].first);
	}
	return(names);
}

int NetCDFSimple::Variable::GetAttType(string name) const {
	for (int i=0; i<_flt_atts.size(); i++) {
		if (name.compare(_flt_atts[i].first) == 0) return(NC_DOUBLE);
	}
	for (int i=0; i<_int_atts.size(); i++) {
		if (name.compare(_int_atts[i].first) == 0) return(NC_INT64);
	}
	for (int i=0; i<_str_atts.size(); i++) {
		if (name.compare(_str_atts[i].first) == 0) return(NC_CHAR);
	}
	return(-1);
}

void NetCDFSimple::Variable::GetAtt(string name, vector <double> &values) const {

	values.clear();
	for (int i=0; i<_flt_atts.size(); i++) {
		if (_flt_atts[i].first.compare(name) == 0) {
			values = _flt_atts[i].second;
			return;
		}
	}
	//
	// Look for atts of type int and then cast to float if found
	//
	for (int i=0; i<_int_atts.size(); i++) {
		if (_int_atts[i].first.compare(name) == 0) {
			for (int j=0; j<_int_atts[i].second.size(); j++) {
				values.push_back(_int_atts[i].second[j]);
			}
			return;
		}
	}
	return;
}
void NetCDFSimple::Variable::GetAtt(string name, vector <long> &values) const {

	values.clear();
	for (int i=0; i<_int_atts.size(); i++) {
		if (_int_atts[i].first.compare(name) == 0) {
			values = _int_atts[i].second;
			return;
		}
	}

	//
	// Look for atts of type float and then cast to int if found
	//
	for (int i=0; i<_flt_atts.size(); i++) {
		if (_flt_atts[i].first.compare(name) == 0) {
			for (int j=0; j<_flt_atts[i].second.size(); j++) {
				values.push_back((long) _flt_atts[i].second[j]);
			}
			return;
		}
	}
	return;
}

void NetCDFSimple::Variable::GetAtt(string name, string &values) const {

	values.clear();
	for (int i=0; i<_str_atts.size(); i++) {
		if (_str_atts[i].first.compare(name) == 0) {
			values = _str_atts[i].second;
			return;
		}
	}
	return;
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const NetCDFSimple::Variable &var) {

	o << "Variable" << endl;
	o << " Name : " << var._name << endl;
	o << " NetCDFSimple var id : " << var._varid << endl;
	o << " NetCDFSimple type : " << var._type << endl;
	o << " Dimensions : " << endl;
	for (int i=0; i<var._dimnames.size(); i++) {
		o << "  " << var._dimnames[i] << endl;
	}
	o << " Attributes : " << endl;
	for (int i=0; i<var._flt_atts.size(); i++) {
		o << "  " << var._flt_atts[i].first << ": ";
		for (int j=0; j<var._flt_atts[i].second.size(); j++) {
			o << var._flt_atts[i].second[j] << " ";
		}
		o << endl;
	}
	for (int i=0; i<var._int_atts.size(); i++) {
		o << "  " << var._int_atts[i].first << ": ";
		for (int j=0; j<var._int_atts[i].second.size(); j++) {
			o << var._int_atts[i].second[j] << " ";
		}
		o << endl;
	}
	for (int i=0; i<var._str_atts.size(); i++) {
		o << "  " << var._str_atts[i].first << ": ";
		o << var._str_atts[i].second << endl;
	}
		
	return(o);
}
};
