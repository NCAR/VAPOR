//************************************************************************
//									*
//		     Copyright (C)  2014				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		RenderParams.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2014
//
//	Description:	Implements the RenderParams class.
//		This is an abstract class for all the tabbed panel rendering params classes.
//		Supports functionality common to all the tabbed panel render params.
//
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <string>
#include <cassert>
#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

using namespace VAPoR;

const string RenderParams::_EnabledTag = "Enabled";
const string RenderParams::_histoScaleTag = "HistoScale";
const string RenderParams::_editBoundsTag = "EditBounds";
const string RenderParams::_histoBoundsTag = "HistoBounds";
const string RenderParams::_cursorCoordsTag = "CursorCoords";
const string RenderParams::_heightVariableNameTag = "HeightVariable";
const string RenderParams::_colorMapVariableNameTag = "ColorMapVariable";
const string RenderParams::_fieldVariableNamesTag = "FieldVariableNames";
const string RenderParams::_auxVariableNamesTag = "AuxVariableNames";
const string RenderParams::_distribVariableNamesTag = "DistributionVariableNames";
const string RenderParams::_variableNameTag = "VariableName";
const string RenderParams::_useSingleColorTag = "UseSingleColor";
const string RenderParams::_constantColorTag = "ConstantColor";
const string RenderParams::_constantOpacityTag = "ConstantOpacity";
const string RenderParams::_CompressionLevelTag = "CompressionLevel";
const string RenderParams::_RefinementLevelTag = "RefinementLevel";
const string RenderParams::_transferFunctionsTag = "TransferFunctions";
const string RenderParams::_stretchFactorsTag = "StretchFactors";
const string RenderParams::_currentTimestepTag = "CurrentTimestep";

namespace {
vector<string> string_replace(vector<string> v, string olds, string news)
{
    for (int i = 0; i < v.size(); i++) {
        if (v[i] == olds) v[i] = news;
    }
    return (v);
}

string string_replace(string s, string olds, string news)
{
    if (s == olds) s = news;
    return (s);
}

};    // namespace

void RenderParams::_init()
{
    SetEnabled(true);

    vector<string> varnames;
    for (int ndim = _maxDim; ndim > 0; ndim--) {
        varnames = _dataMgr->GetDataVarNames(ndim, true);
        if (!varnames.empty()) break;
    }

    string         varname = "";
    vector<string> fieldVarNames(3, "");
    if (varnames.size()) {
        varname = varnames[0];
        for (int i = 0; i < varnames.size() && i < fieldVarNames.size(); i++) { fieldVarNames[i] = varnames[i]; }
    }

    SetVariableName(varname);
    SetFieldVariableNames(fieldVarNames);
    SetHeightVariableName("");
    SetColorMapVariableName("");

    SetRefinementLevel(0);
    SetCompressionLevel(0);
    SetHistoStretch(1.0);

    float rgb[] = {1.0, 1.0, 1.0};
    SetConstantColor(rgb);
    SetConstantOpacity(1.0);
    SetUseSingleColor(false);

    SetStretchFactors(vector<double>(3, 1.0));
}

void RenderParams::_initBox()
{
    string varname = GetVariableName();
    if (varname.empty()) return;

    vector<double> minExt, maxExt;
    int            rc = _dataMgr->GetVariableExtents(0, varname, -1, minExt, maxExt);

    // Crap. No error handling from constructor. Need Initialization()
    // method.
    //
    assert(rc >= 0);

    assert(minExt.size() == maxExt.size() && (minExt.size() == 2 || minExt.size() == 3));

    bool planar = minExt.size() == 2;
    if (planar) {
        minExt.push_back(0.0);
        maxExt.push_back(0.0);
    }

    _Box->SetExtents(minExt, maxExt);
    _Box->SetPlanar(planar);
}

RenderParams::RenderParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, const string &classname, int maxdim) : ParamsBase(ssave, classname)
{
    _dataMgr = dataMgr;
    _maxDim = maxdim;

    // Initialize DataMgr dependent parameters
    //
    _init();

    _TFs = new ParamsContainer(ssave, _transferFunctionsTag);
    _TFs->SetParent(this);

    _Box = new Box(ssave);
    _Box->SetParent(this);
    _initBox();

    _Colorbar = new ColorbarPbase(ssave);
    _Colorbar->SetParent(this);
}

RenderParams::RenderParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node, int maxdim) : ParamsBase(ssave, node)
{
    _dataMgr = dataMgr;
    _maxDim = maxdim;

    // Reconcile DataMgr dependent parameters
    //

    if (node->HasChild(_transferFunctionsTag)) {
        _TFs = new ParamsContainer(ssave, node->GetChild(_transferFunctionsTag));

    } else {
        // Node doesn't contain a transfer function container
        //
        _TFs = new ParamsContainer(ssave, _transferFunctionsTag);
        _TFs->SetParent(this);
    }

    if (node->HasChild(Box::GetClassType())) {
        _Box = new Box(ssave, node->GetChild(Box::GetClassType()));
    } else {
        // Node doesn't contain a Box
        //
        _Box = new Box(ssave);
        _Box->SetParent(this);
    }

    if (node->HasChild(ColorbarPbase::GetClassType())) {
        _Colorbar = new ColorbarPbase(ssave, node->GetChild(ColorbarPbase::GetClassType()));
    } else {
        // Node doesn't contain a ColorbarPbase
        //
        _Colorbar = new ColorbarPbase(ssave);
        _Colorbar->SetParent(this);
    }
}

RenderParams::RenderParams(const RenderParams &rhs) : ParamsBase(rhs)
{
    _dataMgr = rhs._dataMgr;

    _TFs = new ParamsContainer(*(rhs._TFs));
    _Box = new Box(*(rhs._Box));
    _Colorbar = new ColorbarPbase(*(rhs._Colorbar));
}

RenderParams &RenderParams::operator=(const RenderParams &rhs)
{
    if (_TFs) delete _TFs;
    if (_Box) delete _Box;
    if (_Colorbar) delete _Colorbar;

    ParamsBase::operator=(rhs);

    _dataMgr = rhs._dataMgr;

    _TFs = new ParamsContainer(*(rhs._TFs));
    _Box = new Box(*(rhs._Box));
    _Colorbar = new ColorbarPbase(*(rhs._Colorbar));

    return (*this);
}

RenderParams::~RenderParams()
{
    if (_TFs) delete _TFs;
    if (_Box) delete _Box;
    if (_Colorbar) delete _Colorbar;
}

void RenderParams::SetEnabled(bool val) { SetValueLong(_EnabledTag, "enable/disable renderer", val); }

void RenderParams::SetVariableName(string varname)
{
    varname = string_replace(varname, "0", "NULL");
    varname = string_replace(varname, "", "NULL");

    SetValueString(_variableNameTag, "Specify variable name", varname);
}

string RenderParams::GetVariableName() const
{
    string varname = GetValueString(_variableNameTag, "");

    varname = string_replace(varname, "NULL", "");
    return (varname);
}

int RenderParams::GetCompressionLevel() const { return GetValueLong(_CompressionLevelTag, 0); }

void RenderParams::SetCompressionLevel(int level) { SetValueLong(_CompressionLevelTag, "Set compression level", level); }

void RenderParams::SetRefinementLevel(int level)
{
    if (level < 0) level = 0;
    SetValueLong(_RefinementLevelTag, "Set refinement level", level);
}

int RenderParams::GetRefinementLevel() const { return (GetValueLong(_RefinementLevelTag, 0)); }

void RenderParams::SetHistoStretch(float factor)
{
    if (factor < 0.0) factor = 0.0;
    SetValueDouble(_histoScaleTag, "Set histo stretch", (double)factor);
}

float RenderParams::GetHistoStretch() const
{
    float factor = GetValueDouble(_histoScaleTag, (float)1.0);
    if (factor < 0.0) factor = 0.0;
    return (factor);
}

void RenderParams::SetColorbarPbase(ColorbarPbase *pb)
{
    if (_Colorbar) delete _Colorbar;

    _Colorbar = new ColorbarPbase(*pb);
    _Colorbar->SetParent(this);
}

TransferFunction *RenderParams::GetTransferFunc(string varname) const
{
    TransferFunction *tfptr = (TransferFunction *)_TFs->GetParams(varname);

    return (tfptr);
}

TransferFunction *RenderParams::MakeTransferFunc(string varname)
{
    TransferFunction *tfptr = (TransferFunction *)_TFs->GetParams(varname);

    if (tfptr) return (tfptr);

    string s = "Make transfer function for " + varname;
    _ssave->BeginGroup(s);

    TransferFunction tf(_ssave);

    vector<string> varnames = _dataMgr->GetDataVarNames();
    if (find(varnames.begin(), varnames.end(), varname) != varnames.end()) {
        vector<double> range;
        (void)_dataMgr->GetDataRange(0, varname, 0, 0, range);
        tf.setMinMaxMapValue(range[0], range[1]);
    }

    _TFs->Insert(&tf, varname);
    tfptr = (TransferFunction *)_TFs->GetParams(varname);
    assert(tfptr != NULL);

    _ssave->EndGroup();

    return (tfptr);
}

void RenderParams::SetTransferFunc(string varname, TransferFunction *mf)
{
    if (_TFs->GetParams(varname)) { _TFs->Remove(varname); }

    _TFs->Insert(mf, varname);
}

void RenderParams::SetCursorCoords(const float coords[2])
{
    vector<double> coordsv;
    coordsv.push_back(coords[0]);
    coordsv.push_back(coords[1]);
    SetValueDoubleVec(_cursorCoordsTag, "set cursor coords", coordsv);
}

void RenderParams::GetCursorCoords(float coords[2]) const
{
    coords[0] = coords[1] = 2;
    vector<double> defaultv(2, 0.0);

    vector<double> coordsv = GetValueDoubleVec(_cursorCoordsTag, defaultv);
}

vector<string> RenderParams::GetFieldVariableNames() const
{
    vector<string> defaultv(3, "");
    vector<string> varnames;

    varnames = GetValueStringVec(_fieldVariableNamesTag, defaultv);

    varnames = string_replace(varnames, "NULL", "");
    for (int i = varnames.size(); i < 3; i++) varnames.push_back("");

    return (varnames);
}

void RenderParams::SetFieldVariableNames(vector<string> varnames)
{
    varnames = string_replace(varnames, "0", "NULL");
    varnames = string_replace(varnames, "", "NULL");
    for (int i = varnames.size(); i < 3; i++) varnames.push_back("NULL");

    SetValueStringVec(_fieldVariableNamesTag, "Specify vector field variable names", varnames);
    //	setAllBypass(false);
}

vector<string> RenderParams::GetAuxVariableNames() const
{
    vector<string> defaultv(1, "");
    vector<string> varnames;

    varnames = GetValueStringVec(_auxVariableNamesTag, defaultv);

    varnames = string_replace(varnames, "NULL", "");

    return (varnames);
}

void RenderParams::SetAuxVariableNames(vector<string> varnames)
{
    varnames = string_replace(varnames, "0", "NULL");
    varnames = string_replace(varnames, "", "NULL");

    SetValueStringVec(_auxVariableNamesTag, "Specify auxiliary varnames", varnames);
}

string RenderParams::GetFirstVariableName() const
{
    string str = GetVariableName();
    if (str.length()) return str;
    vector<string> strvec = GetFieldVariableNames();
    for (int i = 0; i < strvec.size(); i++) {
        if (strvec[i] != "") return strvec[i];
    }
    return "";
}

string RenderParams::GetHeightVariableName() const
{
    string varname = GetValueString(_heightVariableNameTag, "");

    varname = string_replace(varname, "NULL", "");

    return (varname);
}

void RenderParams::SetHeightVariableName(string varname)
{
    varname = string_replace(varname, "0", "NULL");
    varname = string_replace(varname, "", "NULL");

    SetValueString(_heightVariableNameTag, "Set height variable name", varname);
    //	setAllBypass(false);
}

string RenderParams::GetColorMapVariableName() const
{
    string varname = GetValueString(_colorMapVariableNameTag, "");

    varname = string_replace(varname, "NULL", "");

    return (varname);
}

void RenderParams::SetColorMapVariableName(string varname)
{
    varname = string_replace(varname, "0", "NULL");
    varname = string_replace(varname, "", "NULL");

    SetValueString(_colorMapVariableNameTag, "Set color map variable name", varname);
}

void RenderParams::SetConstantColor(const float rgb[3])
{
    vector<double> rgbv;
    for (int i = 0; i < 3; i++) {
        float v = rgb[i];
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        rgbv.push_back(v);
    }
    SetValueDoubleVec(_constantColorTag, "Specify constant color in RGB", rgbv);
}

void RenderParams::GetConstantColor(float rgb[3]) const
{
    vector<double> defaultv(3, 1.0);
    defaultv[0] = 0.0;
    defaultv[1] = 0.0;
    defaultv[2] = 0.0;
    vector<double> rgbv = GetValueDoubleVec(_constantColorTag, defaultv);
    for (int i = 0; i < 3; i++) rgb[i] = 1.0;

    for (int i = 0; i < rgbv.size() && i < 3; i++) {
        float v = rgbv[i];
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        rgb[i] = rgbv[i];
    }
};

void RenderParams::SetConstantOpacity(float o)
{
    if (o < 0.0) o = 0.0;
    if (o > 1.0) o = 1.0;

    SetValueDouble(_constantOpacityTag, "Specify constant opacity", o);
}

float RenderParams::GetConstantOpacity() const
{
    vector<double> defaultv(1, 1.0);
    vector<double> vec = GetValueDoubleVec(_constantOpacityTag, defaultv);

    float o = 1.0;
    if (vec.size()) o = vec[0];

    if (o < 0.0) o = 0.0;
    if (o > 1.0) o = 1.0;

    return (o);
}

//////////////////////////////////////////////////////////////////////////
//
// RenParamsFactory Class
//
/////////////////////////////////////////////////////////////////////////

RenderParams *RenParamsFactory::CreateInstance(string className, DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node)
{
    RenderParams *instance = NULL;

    // find className in the registry and call factory method.
    //
    auto it = _factoryFunctionRegistry.find(className);
    if (it != _factoryFunctionRegistry.end()) instance = it->second(dataMgr, ssave, node);

    if (instance != NULL)
        return instance;
    else
        return NULL;
}

vector<string> RenParamsFactory::GetFactoryNames() const
{
    vector<string>                                                                                       names;
    map<string, function<RenderParams *(DataMgr *, ParamsBase::StateSave *, XmlNode *)>>::const_iterator itr;

    for (itr = _factoryFunctionRegistry.begin(); itr != _factoryFunctionRegistry.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

//////////////////////////////////////////////////////////////////////////
//
// RenParamsContainer Class
//
/////////////////////////////////////////////////////////////////////////

RenParamsContainer::RenParamsContainer(DataMgr *dataMgr, ParamsBase::StateSave *ssave, const string &name)
{
    assert(dataMgr != NULL);
    assert(ssave != NULL);

    _dataMgr = dataMgr;
    _ssave = ssave;
    _separator = NULL;
    _elements.clear();

    _separator = new ParamsSeparator(ssave, name);
}

RenParamsContainer::RenParamsContainer(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node)
{
    assert(dataMgr != NULL);
    assert(ssave != NULL);
    assert(node != NULL);

    _dataMgr = dataMgr;
    _ssave = ssave;
    _separator = new ParamsSeparator(ssave, node);
    _elements.clear();

    for (int i = 0; i < node->GetNumChildren(); i++) {
        XmlNode *eleNameNode = node->GetChild(i);
        if (!eleNameNode->HasChild(0)) continue;    // bad node

        XmlNode *eleNode = eleNameNode->GetChild(0);

        string eleName = eleNameNode->GetTag();
        string classname = eleNode->GetTag();

        _elements[eleName] = RenParamsFactory::Instance()->CreateInstance(classname, dataMgr, ssave, eleNode);
        assert(_elements[eleName] != NULL);
    }
}

RenParamsContainer::RenParamsContainer(const RenParamsContainer &rhs)
{
    _dataMgr = rhs._dataMgr;
    _ssave = rhs._ssave;
    _separator = NULL;
    _elements.clear();

    _separator = new ParamsSeparator(*(rhs._separator));
    _separator->SetParent(NULL);

    vector<string> names = rhs.GetNames();
    for (int i = 0; i < names.size(); i++) {
        // Make copy of RenderParams
        //
        RenderParams *rhspb = rhs.GetParams(names[i]);
        XmlNode *     node = new XmlNode(*(rhspb->GetNode()));

        string        classname = rhspb->GetName();
        RenderParams *mypb = RenParamsFactory::Instance()->CreateInstance(classname, _dataMgr, _ssave, node);
        mypb->SetParent(_separator);

        _elements[names[i]] = mypb;
    }
}

RenParamsContainer &RenParamsContainer::operator=(const RenParamsContainer &rhs)
{
    assert(_separator);

    vector<string> mynames = GetNames();
    for (int i = 0; i < mynames.size(); i++) { Remove(mynames[i]); }
    _elements.clear();

    _dataMgr = rhs._dataMgr;
    _ssave = rhs._ssave;
    _separator = rhs._separator;

    vector<string> names = rhs.GetNames();
    for (int i = 0; i < names.size(); i++) {
        XmlNode *eleNameNode = _separator->GetNode()->GetChild(names[i]);
        assert(eleNameNode);

        ParamsSeparator mySep(_ssave, eleNameNode);

        XmlNode *eleNode = eleNameNode->GetChild(0);

        string eleName = eleNameNode->GetTag();
        string classname = eleNode->GetTag();

        RenderParams *mypb = RenParamsFactory::Instance()->CreateInstance(classname, _dataMgr, _ssave, eleNode);
        mypb->SetParent(&mySep);

        _elements[names[i]] = mypb;
    }

    return (*this);
}

RenParamsContainer::~RenParamsContainer()
{
    map<string, RenderParams *>::iterator itr;
    for (itr = _elements.begin(); itr != _elements.end(); ++itr) {
        if (itr->second) delete itr->second;
    }

    if (_separator) delete _separator;
}

RenderParams *RenParamsContainer::Insert(const RenderParams *pb, string name)
{
    assert(pb != NULL);

    map<string, RenderParams *>::iterator itr = _elements.find(name);
    if (itr != _elements.end()) { delete itr->second; }

    // Create a separator node
    //
    ParamsSeparator mySep(_ssave, name);
    mySep.SetParent(_separator);

    // Create element name node
    //
    string        classname = pb->GetName();
    XmlNode *     node = new XmlNode(*(pb->GetNode()));
    RenderParams *mypb = RenParamsFactory::Instance()->CreateInstance(classname, _dataMgr, _ssave, node);
    assert(mypb != NULL);
    mypb->SetParent(&mySep);

    _elements[name] = mypb;

    return (mypb);
}

RenderParams *RenParamsContainer::Create(string className, string name)
{
    map<string, RenderParams *>::iterator itr = _elements.find(name);
    if (itr != _elements.end()) { delete itr->second; }

    // Create a separator node
    //
    ParamsSeparator mySep(_ssave, name);
    mySep.SetParent(_separator);

    cout << "createInstance (rp): " << className << " " << name << endl;
    cout << (_dataMgr == NULL) << endl;

    // Create the desired class
    //
    RenderParams *mypb = RenParamsFactory::Instance()->CreateInstance(className, _dataMgr, _ssave, NULL);
    assert(mypb != NULL);

    mypb->SetParent(&mySep);

    _elements[name] = mypb;

    return (mypb);
}

void RenParamsContainer::Remove(string name)
{
    map<string, RenderParams *>::iterator itr = _elements.find(name);
    if (itr == _elements.end()) return;

    RenderParams *mypb = itr->second;

    // Set parent to root so  Xml representation will be deleted
    //
    mypb->SetParent(NULL);
    delete mypb;

    _elements.erase(itr);
}

RenderParams *RenParamsContainer::GetParams(string name) const
{
    map<string, RenderParams *>::const_iterator itr = _elements.find(name);
    if (itr != _elements.end()) return (itr->second);

    return (NULL);
}

vector<string> RenParamsContainer::GetNames() const
{
    map<string, RenderParams *>::const_iterator itr;

    vector<string> names;
    for (itr = _elements.begin(); itr != _elements.end(); ++itr) { names.push_back(itr->first); }

    return (names);
}
