#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cerrno>
#include <csignal>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/VDCNetCDF.h>

using namespace Wasp;
using namespace VAPoR;
typedef VDCNetCDF::XType XType;
typedef DC::Dimension Dimension;
typedef unsigned int Verbosity;

struct Attribute;
struct Variable;
struct DataVariable;
struct CoordVariable;
class OutFormat;

//---------------------------------------
// Command Line Options
//---------------------------------------

struct opt_t {
	OptionParser::Boolean_T verbose;
	OptionParser::Boolean_T ncOrder;
	OptionParser::Boolean_T	help;
} opt;

OptionParser::OptDescRec_T	set_opts[] = {
	{"verbose",		0, 	"0", "Print additional information"},
	{"nc-order",	0, 	"0", "Reverse dimension order to match NetCDF"},
	{"help",		0,	"",	 "Print this message and exit"},
	{NULL}
};

OptionParser::Option_T	get_options[] = {
	{"verbose",  Wasp::CvtToBoolean, &opt.verbose, sizeof(opt.verbose)},
	{"nc-order", Wasp::CvtToBoolean, &opt.ncOrder, sizeof(opt.ncOrder)},
	{"help",     Wasp::CvtToBoolean, &opt.help,    sizeof(opt.help)},
	{NULL}
};

//---------------------------------------
// Utility Functions and Print Format
//---------------------------------------

#define Assert(tst) { if (!(tst)) { cerr << "[" << __LINE__ << ":" << __FILE__ << "] Assert Failed: " << #tst << endl; std::raise(SIGINT); } }

class OutFormat {
	protected:
		int _depth;
	public:
		stringstream _ss;

		OutFormat() : _depth(0) {}
		void push() { _depth++; }
		void pop() { _depth--; Assert(_depth >= 0); }
		void pre() { for (int i = 0; i < _depth; i++) cout << "\t"; }
		void post() { cout << _ss.str() << " ;" << endl; _ss.str(""); _ss.clear(); }
		void header(string name) {
			cout << "----------------------------------" << endl;
			cout << name << endl;
			cout << "----------------------------------" << endl;
		}
		void stringProperty(string owner, string name, string value) { pre(); _ss << owner << (owner == "" ? "" : ":") << name << " = \"" << value << "\""; post(); }
		void boolProperty(string owner, string name, bool value) { pre(); _ss << owner << (owner == "" ? "" : ":") << name << " = " << (value ? "True" : "False"); post(); }
		void intProperty(string owner, string name, long value) { pre(); _ss << owner << (owner == "" ? "" : ":") << name << " = " << value; post(); }
		void floatProperty(string owner, string name, double value) { pre(); _ss << owner << (owner == "" ? "" : ":") << name << " = " << value; post(); }
		void typeProperty(string owner, XType type) { pre(); _ss << owner << (owner == "" ? "" : ":") << "Type = " << GetTypeString(type); post(); }
		template<typename T> void vectorProperty(string owner, string name, vector<T> array, bool reversed) {
			pre();
			_ss << owner << (owner == "" ? "" : ":") << name << " = ";
			if (reversed)
				for (int i = array.size() - 1; i >= 0; i--)
					_ss << (T)array[i] << (i == 0 ? "" : ", ");
			else
				for (int i = 0; i < array.size(); i++)
					_ss << (T)array[i] << (i == array.size()-1 ? "" : ", ");
			post();
		}
		void category(string name) { pre(); _ss << name; post(); }
		static string GetAxisString(int axis) {
			switch (axis) {
				case 0: return "X";
				case 1: return "Y";
				case 2: return "Z";
				case 3: return "Time";
				default: return std::to_string((long long int)axis);
			}
		}
		static string GetTypeString(XType type) {
			switch (type) {
				case XType::UINT8:   return "uint8";
				case XType::INT8:   return "int8";
				case XType::INT32:   return "int32";
				case XType::INT64:   return "int64";
				case XType::FLOAT:   return "float";
				case XType::DOUBLE:  return "double";
				case XType::TEXT:    return "text";
				case XType::INVALID: return "INVALID";
			};
		}
};

//---------------------------------------
// Classes that load and print vdc data
//---------------------------------------

struct Attribute {
	string _name;
	XType _type;
	vector<long> _ivalues;
	vector<double> _fvalues;
	string _svalues;

	Attribute(string name, XType type)
		: _name(name), _type(type) {}
	bool isInt()   { return _type == XType::INT32 || _type == XType::INT64; }
	bool isFloat() { return _type == XType::FLOAT || _type == XType::DOUBLE; }
	bool isText()  { return _type == XType::TEXT; }
	void print(OutFormat *out, string parentName) {
		if (isText())
			out->stringProperty(parentName, _name, _svalues);
		else if (isInt())
			out->vectorProperty<long>(parentName, _name, _ivalues, false);
		else
			out->vectorProperty<double>(parentName, _name, _fvalues, false);
	}
};

struct Variable {
	string _name;
	DC::BaseVar *_data;
	vector<Attribute> _attributes;
	vector <string> _dimnames;

	Variable(string name, VDCNetCDF &vdc) : _name(name), _data(0) {
		//
		// Get Attributes
		//
		vector<string> attNames = vdc.GetAttNames(name);
		for (int i = 0; i < attNames.size(); i++) {
			_attributes.push_back(Attribute(attNames[i], vdc.GetAttType(name, attNames[i])));
			if      (_attributes[i].isInt())
				vdc.GetAtt(name, attNames[i], _attributes[i]._ivalues);
			else if (_attributes[i].isFloat())
				vdc.GetAtt(name, attNames[i], _attributes[i]._fvalues);
			else if (_attributes[i].isText())
				vdc.GetAtt(name, attNames[i], _attributes[i]._svalues);
		}
		vdc.GetVarDimNames(_name, false, _dimnames);
	}
	virtual ~Variable() {}

	void printName(OutFormat *out, Verbosity V, bool reversedCoordOrder) {
		out->pre();
		out->_ss << OutFormat::GetTypeString(_data->GetXType()) << " " << _name << "(";
		if (reversedCoordOrder)
			for (int i = _dimnames.size() - 1; i >= 0; i--)
				out->_ss << _dimnames[i] << (i != 0 ? ", " : "");
		else
			for (int i = 0; i < _dimnames.size(); i++)
				out->_ss << _dimnames[i] << (i != _dimnames.size() - 1 ? ", " : "");
		out->_ss << ")";
		out->post();
	}

	virtual void printChild(OutFormat *out, Verbosity V, bool reversedCoordOrder) = 0;
	virtual void print(OutFormat *out, Verbosity V, bool reversedCoordOrder) {

		printName(out, V, reversedCoordOrder);
		out->push();
		// Print VDC specific data if verbose
		if (V >= 1) {
			out->stringProperty(_name, "Units", _data->GetUnits());
			out->stringProperty(_name, "WName", _data->GetWName());
			out->vectorProperty<size_t>(_name, "CRatios", _data->GetCRatios(), false);
			out->vectorProperty<size_t>(_name, "BS", _data->GetBS(), false);
			out->vectorProperty<bool>(_name, "Periodic", _data->GetPeriodic(), false);
		}

		// Print information specific to Data or Coordinate variable
		printChild(out, V, reversedCoordOrder);

		// Print attributes
		if (_attributes.size()) {
			out->category("Attributes");
			out->push();
			for (int i = 0; i < _attributes.size(); i++)
				_attributes[i].print(out, _name);
			out->pop();
		}
		out->pop();
	}
};

struct DataVariable : public Variable {
	vector <string> _coordvars;

	DataVariable(string name, VDCNetCDF &vdc) : Variable(name, vdc) {
		_data = new DC::DataVar();
		vdc.GetDataVarInfo(name, *(DC::DataVar *)_data);
		vdc.GetVarCoordVars(name, false, _coordvars);
	}
	~DataVariable()
	{ if (_data) delete _data; }

	void printChild(OutFormat *out, Verbosity V, bool reversedCoordOrder)
	{
		out->vectorProperty(_name, "Coord Vars", _coordvars, reversedCoordOrder);

		// Print VDC specific data if verbose
		if (V >= 1) {
			out->stringProperty(_name, "Mask Var", ((DC::DataVar *)_data)->GetMaskvar());
			out->boolProperty(_name, "Has Missing", ((DC::DataVar *)_data)->GetHasMissing());
			out->boolProperty(_name, "Missing Value", ((DC::DataVar *)_data)->GetMissingValue());
		}
	}
};

struct CoordVariable : public Variable {
	CoordVariable(string name, VDCNetCDF &vdc) : Variable(name, vdc) {
		_data = new DC::CoordVar();
		vdc.GetCoordVarInfo(name, *(DC::CoordVar *)_data);
	}
	~CoordVariable()
	{ if (_data) delete _data; }

	void printChild(OutFormat *out, Verbosity V, bool reversedCoordOrder)
	{
		out->stringProperty(_name, "Axis", OutFormat::GetAxisString(((DC::CoordVar *)_data)->GetAxis()));

		// Print VDC specific data if verbose
		if (V >= 1) {
			out->boolProperty(_name, "Uniform", ((DC::CoordVar *)_data)->GetUniform());
		}
	}
};

struct GlobalData : public Variable {
	vector<DC::Dimension> _dimensions;

	GlobalData(VDCNetCDF &vdc) : Variable("", vdc) {
		_name = "__GLOBAL__";
		vector<string> dimNames = vdc.GetDimensionNames();
		_dimensions.resize(dimNames.size());
		for (int i = 0; i < dimNames.size(); i++)
			vdc.GetDimension(dimNames[i], _dimensions[i]);
	}
	void printChild(OutFormat *out, Verbosity V, bool reversedCoordOrder) {}
	void print(OutFormat *out, Verbosity V, bool reversedCoordOrder) {
		out->category("Dimensions");
		out->push();
		for (int i = 0; i < _dimensions.size(); i++) {
			out->category(_dimensions[i].GetName());
			out->push();
			out->intProperty("", "Length", _dimensions[i].GetLength());
			out->pop();
		}
		out->pop();

		out->category("Attributes");
		out->push();
		for (int i = 0; i < _attributes.size(); i++)
			_attributes[i].print(out, "");
		out->pop();
	}
};

//---------------------------------------
// Utility Functions and Print Format
//---------------------------------------

int main(int argc, char **argv)
{
	OptionParser op;
	const char *progName = Basename(argv[0]);
	MyBase::SetErrMsgFilePtr(stderr);


	if (op.AppendOptions(set_opts) < 0) {
		cerr << progName << ": " << op.GetErrMsg();
		exit(1);
	}
	if (op.ParseOptions(&argc, argv, get_options) < 0) {
		cerr << progName << " : " << op.GetErrMsg();
		exit(1);
	}
	if (opt.help) {
		cerr << "Usage: " << progName << " [options] vdc.nc" << endl;
		op.PrintOptionHelp(stderr);
		exit(0);
	}
	if (argc != 2) {
		cerr << "Usage: " << progName << " vdcmaster.nc" << endl;
		exit(1);
	}


	VDCNetCDF vdc(1);
	int rc = vdc.Initialize(
		string(argv[1]), vector <string> (), VDC::R, 4*1024*1024
	);
	if (rc < 0) exit(1);

	//
	// Allocate VDC data
	//
	GlobalData *globalData;
	vector<DataVariable*> variables;
	vector<CoordVariable*> coordinates;

	//
	// Load VDC data
	//
	globalData = new GlobalData(vdc);
	vector<string> varNames = vdc.GetDataVarNames();
	for (int i = 0; i < varNames.size(); i++)
		variables.push_back(new DataVariable(varNames[i], vdc));
	vector<string> coordNames = vdc.GetCoordVarNames();
	for (int i = 0; i < coordNames.size(); i++)
		coordinates.push_back(new CoordVariable(coordNames[i], vdc));

	//
	// Print VDC data
	//
	OutFormat *out = new OutFormat;
	out->header("Global Data");

	globalData->print(out, opt.verbose, opt.ncOrder);

	out->header("Data Variables");
	for (int i = 0; i < variables.size(); i++) variables[i]->print(out, opt.verbose, opt.ncOrder);

	out->header("Coordinate Variables");
	for (int i = 0; i < coordinates.size(); i++) coordinates[i]->print(out, opt.verbose, opt.ncOrder);


	return 0;
}
