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
#include "vapor/VAssert.h"
#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>

using namespace VAPoR;

const string RenderParams::_EnabledTag = "Enabled";
const string RenderParams::_histoScaleTag = "HistoScale";
const string RenderParams::_editBoundsTag = "EditBounds";
const string RenderParams::_histoBoundsTag = "HistoBounds";
const string RenderParams::_cursorCoordsTag = "CursorCoords";
const string RenderParams::_heightVariableNameTag = "HeightVariable";
const string RenderParams::_colorMapVariableNameTag = "ColorMapVariable";
const string RenderParams::_xFieldVariableNameTag = "FieldVariableName_X";
const string RenderParams::_yFieldVariableNameTag = "FieldVariableName_Y";
const string RenderParams::_zFieldVariableNameTag = "FieldVariableName_Z";
const string RenderParams::_auxVariableNamesTag = "AuxVariableNames";
const string RenderParams::_distribVariableNamesTag = "DistributionVariableNames";
const string RenderParams::_variableNameTag = "VariableName";
const string RenderParams::_useSingleColorTag = "UseSingleColor";
const string RenderParams::_constantColorTag = "ConstantColor";
const string RenderParams::_constantOpacityTag = "ConstantOpacity";
const string RenderParams::_CompressionLevelTag = "CompressionLevel";
const string RenderParams::_RefinementLevelTag = "RefinementLevel";
const string RenderParams::_transferFunctionsTag = "MapperFunctions";
const string RenderParams::_stretchFactorsTag = "StretchFactors";
const string RenderParams::_currentTimestepTag = "CurrentTimestep";

#define REQUIRED_SAMPLE_SIZE 1000000

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

void RenderParams::SetDefaultVariables(int dim = 3, bool secondaryColormapVariable = false)
{
    // Find the first variable in the data collection of
    // the requested dimesion that exists and make it the default.
    //
    string varname;
    size_t ts;
    bool   ok = false;
    for (int i = dim; i > 1; i--) {
        if (ok = DataMgrUtils::GetFirstExistingVariable(_dataMgr, 0, 0, i, varname, ts)) break;
    }
    if (!ok) varname = "";
    SetVariableName(varname);
    SetColorMapVariableName(varname);

    // Now set the rest of the variable name fields. It's not important
    // that these exist or not
    //
    vector<string> varnames;
    varnames = _dataMgr->GetDataVarNames(dim);

    vector<string> fieldVarNames(3, "");
    fieldVarNames[0] = _findVarStartingWithLetter(varnames, 'u');
    fieldVarNames[1] = _findVarStartingWithLetter(varnames, 'v');
    if (dim == 3) fieldVarNames[2] = _findVarStartingWithLetter(varnames, 'w');

    SetFieldVariableNames(fieldVarNames);

    string colorVar = varname;
    if (secondaryColormapVariable) colorVar = _findVarStartingWithLetter(varnames, 't');

    if (!colorVar.empty()) SetColorMapVariableName(colorVar);
}

void RenderParams::_init()
{
    SetEnabled(true);

    SetDefaultVariables(_maxDim);

    SetRefinementLevel(0);
    SetCompressionLevel(0);
    SetHistoStretch(1.0);

    float rgb[] = {1.0, 1.0, 1.0};
    SetConstantColor(rgb);
    SetConstantOpacity(1.0);
}

int RenderParams::Initialize()
{
    if (_classInitialized) return (0);

    vector<double> minExt, maxExt;

    //
    // Initialize box with bounds of a single variable. First check
    // variable returned by GetVariableName(). If not available,
    // look for others
    //
    string varname = GetVariableName();
    size_t ts = 0;
    if (!_dataMgr->VariableExists(ts, varname, 0, 0)) {
        for (int ndim = 3; ndim > 0; ndim--) {
            bool ok = DataMgrUtils::GetFirstExistingVariable(_dataMgr, 0, 0, ndim, varname, ts);
            if (ok)
                break;
            else
                varname = "";
        }
    }

    if (varname.empty()) return (0);

    int rc = _dataMgr->GetVariableExtents(ts, varname, 0, 0, minExt, maxExt);
    if (rc < 0) return (-1);

    VAssert(minExt.size() == maxExt.size() && (minExt.size() == 2 || minExt.size() == 3));

    bool planar = minExt.size() == 2;
    if (planar) {
        _Box->SetOrientation(VAPoR::Box::XY);
    } else {
        _Box->SetOrientation(VAPoR::Box::XYZ);
    }

    _Box->SetExtents(minExt, maxExt);
    _Box->SetPlanar(planar);

    vector<double> origin(minExt.size());
    for (int i = 0; i < minExt.size(); i++) { origin[i] = minExt[i] + (maxExt[i] - minExt[i]) * 0.5; }
    _transform->SetOrigin(origin);

    _classInitialized = true;
    return (0);
}

RenderParams::RenderParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, const string &classname, int maxdim) : ParamsBase(ssave, classname)
{
    _classInitialized = false;
    _dataMgr = dataMgr;
    _maxDim = maxdim;
    _stride = 1;

    // Initialize DataMgr dependent parameters
    //
    _init();

    _TFs = new ParamsContainer(ssave, _transferFunctionsTag);
    _TFs->SetParent(this);

    _Box = new Box(ssave);
    _Box->SetParent(this);

    _Colorbar = new ColorbarPbase(ssave);
    _Colorbar->SetParent(this);

    _transform = new Transform(ssave);
    _transform->SetParent(this);
}

RenderParams::RenderParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node, int maxdim) : ParamsBase(ssave, node)
{
    _classInitialized = true;
    _dataMgr = dataMgr;
    _maxDim = maxdim;
    _stride = 1;

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

    if (node->HasChild(Transform::GetClassType())) {
        _transform = new Transform(ssave, node->GetChild(Transform::GetClassType()));
    } else {
        // Node doesn't contain a Transform
        //
        _transform = new Transform(ssave);
        _transform->SetParent(this);
    }
}

RenderParams::RenderParams(const RenderParams &rhs) : ParamsBase(rhs)
{
    _dataMgr = rhs._dataMgr;

    _TFs = new ParamsContainer(*(rhs._TFs));
    _Box = new Box(*(rhs._Box));
    _Colorbar = new ColorbarPbase(*(rhs._Colorbar));
    _transform = new Transform(*(rhs._transform));
}

RenderParams &RenderParams::operator=(const RenderParams &rhs)
{
    if (_TFs) delete _TFs;
    if (_Box) delete _Box;
    if (_Colorbar) delete _Colorbar;
    if (_transform) delete _transform;

    ParamsBase::operator=(rhs);

    _dataMgr = rhs._dataMgr;

    _TFs = new ParamsContainer(*(rhs._TFs));
    _Box = new Box(*(rhs._Box));
    _Colorbar = new ColorbarPbase(*(rhs._Colorbar));
    _transform = new Transform(*(rhs._transform));

    return (*this);
}

RenderParams::~RenderParams()
{
    if (_TFs) delete _TFs;
    if (_Box) delete _Box;
    if (_Colorbar) delete _Colorbar;
    if (_transform) delete _transform;
}

void RenderParams::SetEnabled(bool val) { SetValueLong(_EnabledTag, "enable/disable renderer", val); }

void RenderParams::SetVariableName(string varname)
{
    varname = string_replace(varname, "<no-variable>", "NULL");
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

void RenderParams::_calculateStride(string varName)
{
    if (varName.empty() || varName == "NULL") {
        _stride = 1;
        return;
    }

    std::vector<size_t> dimsAtLevel;
    int                 ref = GetRefinementLevel();

    // Yikes.  Error reporting is turned off, so ignore the return code
    _dataMgr->GetDimLensAtLevel(varName, ref, dimsAtLevel, GetCurrentTimestep());

    int size = 1;
    for (int i = 0; i < dimsAtLevel.size(); i++) size *= dimsAtLevel[i];

    _stride = 1;
    if (size > REQUIRED_SAMPLE_SIZE) _stride = 1 + size / REQUIRED_SAMPLE_SIZE;
}

MapperFunction *RenderParams::GetMapperFunc(string varname)
{
    // This way we always return a valid MapperFunction
    //
    if (varname.empty()) { varname = "NULL"; }
    MapperFunction *tfptr = dynamic_cast<MapperFunction *>(_TFs->GetParams(varname));

    if (tfptr) { return (tfptr); }

    // Disable state saving for Get function
    //
    bool enabled = _ssave->GetEnabled();
    _ssave->SetEnabled(false);

    MapperFunction tf(_ssave);

    _calculateStride(varname);

    size_t ts = 0;
    int    level = 0;
    int    lod = 0;
    if (_dataMgr->VariableExists(ts, varname, level, lod)) {
        vector<double> minExt, maxExt;
        _Box->GetExtents(minExt, maxExt);

        vector<double> range;
        bool           prev = EnableErrMsg(false);    // no error handling
        int            rc = _dataMgr->GetDataRange(ts, varname, level, lod, minExt, maxExt, range);
        if (rc < 0) { range = {0.0, 1.0}; }
        EnableErrMsg(prev);
        tf.setMinMaxMapValue(range[0], range[1]);
    }

    _TFs->Insert(&tf, varname);
    tfptr = (MapperFunction *)_TFs->GetParams(varname);
    VAssert(tfptr != NULL);

    _ssave->SetEnabled(enabled);

    return (tfptr);
}

void RenderParams::RemoveMapperFunc(string varname) { _TFs->Remove(varname); }

void RenderParams::SetMapperFunc(string varname, MapperFunction *mf)
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
    //	vector <string> defaultv(3, "");
    vector<string> varnames(3);

    //    varnames = GetValueStringVec(_fieldVariableNamesTag, defaultv);
    varnames[0] = GetValueString(_xFieldVariableNameTag, "");
    varnames[1] = GetValueString(_yFieldVariableNameTag, "");
    varnames[2] = GetValueString(_zFieldVariableNameTag, "");
    varnames = string_replace(varnames, "NULL", "");
    for (int i = varnames.size(); i < 3; i++) varnames.push_back("");

    return (varnames);
}

void RenderParams::SetFieldVariableNames(vector<string> varnames)
{
    varnames = string_replace(varnames, "<no-variable>", "NULL");
    varnames = string_replace(varnames, "", "NULL");
    for (int i = varnames.size(); i < 3; i++) varnames.push_back("NULL");

    SetValueString(_xFieldVariableNameTag, "", varnames[0]);
    SetValueString(_yFieldVariableNameTag, "", varnames[1]);
    SetValueString(_zFieldVariableNameTag, "", varnames[2]);
}

std::string RenderParams::GetXFieldVariableName() const
{
    std::vector<std::string> fieldVars = GetFieldVariableNames();
    VAssert(fieldVars.size() == 3);
    return fieldVars[0];
}

void RenderParams::SetXFieldVariableName(std::string varName)
{
    std::vector<std::string> fieldVars = GetFieldVariableNames();
    VAssert(fieldVars.size() == 3);
    fieldVars[0] = varName;
    SetFieldVariableNames(fieldVars);
}

std::string RenderParams::GetYFieldVariableName() const
{
    std::vector<std::string> fieldVars = GetFieldVariableNames();
    VAssert(fieldVars.size() == 3);
    return fieldVars[1];
}

void RenderParams::SetYFieldVariableName(std::string varName)
{
    std::vector<std::string> fieldVars = GetFieldVariableNames();
    VAssert(fieldVars.size() == 3);
    fieldVars[1] = varName;
    SetFieldVariableNames(fieldVars);
}

std::string RenderParams::GetZFieldVariableName() const
{
    std::vector<std::string> fieldVars = GetFieldVariableNames();
    VAssert(fieldVars.size() == 3);
    return fieldVars[2];
}

void RenderParams::SetZFieldVariableName(std::string varName)
{
    std::vector<std::string> fieldVars = GetFieldVariableNames();
    VAssert(fieldVars.size() == 3);
    fieldVars[2] = varName;
    SetFieldVariableNames(fieldVars);
}

vector<string> RenderParams::GetAuxVariableNames() const
{
    std::vector<std::string> varnames = GetValueStringVec(_auxVariableNamesTag);
    varnames = string_replace(varnames, "NULL", "");
    return (varnames);
}

void RenderParams::SetAuxVariableNames(std::vector<std::string> varnames)
{
    varnames = string_replace(varnames, "<no-variable>", "NULL");
    varnames = string_replace(varnames, "", "NULL");
    SetValueStringVec(_auxVariableNamesTag, "Specify auxiliary varnames", varnames);
}

string RenderParams::GetFirstVariableName() const
{
    string str = GetVariableName();
    if (str.length()) return str;    // scalar
    vector<string> strvec = GetFieldVariableNames();
    for (int i = 0; i < strvec.size(); i++) {
        if (strvec[i] != "") return strvec[i];    // vector
    }
    str = GetColorMapVariableName();
    if (!str.empty()) return str;
    str = GetHeightVariableName();
    if (str.length()) return str;    // height
    return "";                       // none
}

string RenderParams::GetHeightVariableName() const
{
    string varname = GetValueString(_heightVariableNameTag, "");

    varname = string_replace(varname, "NULL", "");

    return (varname);
}

void RenderParams::SetHeightVariableName(string varname)
{
    varname = string_replace(varname, "<no-variable>", "NULL");
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

bool RenderParams::UseSingleColor() const { return GetValueLong(_useSingleColorTag, GetUseSingleColorDefault()); }

void RenderParams::SetUseSingleColor(bool val) { SetValueLong(_useSingleColorTag, "enable/disable use single color", (long)val); }

void RenderParams::SetColorMapVariableName(string varname)
{
    varname = string_replace(varname, "<no-variable>", "NULL");
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
    vector<double> rgbv = GetValueDoubleVec(_constantColorTag, defaultv);

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

string RenderParams::_findVarStartingWithLetter(vector<string> searchVars, char letter)
{
    for (auto &element : searchVars) {
        if (element[0] == letter || element[0] == toupper(letter)) { return element; }
    }
    return "";
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
    VAssert(dataMgr != NULL);
    VAssert(ssave != NULL);

    _dataMgr = dataMgr;
    _ssave = ssave;
    _separator = NULL;
    _elements.clear();

    _separator = new ParamsSeparator(ssave, name);
}

RenParamsContainer::RenParamsContainer(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node)
{
    VAssert(dataMgr != NULL);
    VAssert(ssave != NULL);
    VAssert(node != NULL);

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

        RenderParams *rParams = RenParamsFactory::Instance()->CreateInstance(classname, dataMgr, ssave, eleNode);
        if (rParams == NULL) {
            SetDiagMsg("RenParamsContainer::RenParamsContainer() unrecognized class: %s", classname.c_str());

            continue;
        }
        _elements[eleName] = rParams;
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
    VAssert(_separator);

    vector<string> mynames = GetNames();
    for (int i = 0; i < mynames.size(); i++) { Remove(mynames[i]); }
    _elements.clear();

    _dataMgr = rhs._dataMgr;
    _ssave = rhs._ssave;
    _separator = rhs._separator;

    vector<string> names = rhs.GetNames();
    for (int i = 0; i < names.size(); i++) {
        XmlNode *eleNameNode = _separator->GetNode()->GetChild(names[i]);
        VAssert(eleNameNode);

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
    VAssert(pb != NULL);

    if (name.empty()) { name = "NULL"; }

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
    VAssert(mypb != NULL);
    mypb->SetParent(&mySep);

    _elements[name] = mypb;

    return (mypb);
}

RenderParams *RenParamsContainer::Create(string className, string name)
{
    VAssert(!className.empty());
    VAssert(!name.empty());

    map<string, RenderParams *>::iterator itr = _elements.find(name);
    if (itr != _elements.end()) { delete itr->second; }

    // Create a separator node
    //
    ParamsSeparator mySep(_ssave, name);
    mySep.SetParent(_separator);

    // Create the desired class
    //
    RenderParams *mypb = RenParamsFactory::Instance()->CreateInstance(className, _dataMgr, _ssave, NULL);
    VAssert(mypb != NULL);

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
    if (name.empty()) { name = "NULL"; }

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
