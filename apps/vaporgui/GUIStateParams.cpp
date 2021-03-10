//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		GUIStateParams.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2016
//
#ifdef WIN32
    // Annoying unreferenced formal parameter warning
    #pragma warning(disable : 4100)
#endif

#include <iostream>
#include <QDir>

#include "MouseModeParams.h"
#include "GUIStateParams.h"

using namespace VAPoR;

const string GUIStateParams::m_activeVisualizer = "ActiveVisualizer";
const string GUIStateParams::m_pathParamsTag = "PathParamsTag";
const string GUIStateParams::m_sessionFileTag = "SessionFileTag";
const string GUIStateParams::m_imagePathTag = "ImagePathTag";
const string GUIStateParams::m_imageSavePathTag = "ImageSavePathTag";
const string GUIStateParams::m_pythonPathTag = "PythonPathTag";
const string GUIStateParams::m_flowPathTag = "FlowPathTag";
const string GUIStateParams::m_tfPathTag = "TFPathTag";
const string GUIStateParams::m_statsDatasetNameTag = "StatsDatasetNameTag";
const string GUIStateParams::m_plotDatasetNameTag = "PlotDatasetNameTag";
const string GUIStateParams::m_proj4StringTag = "Proj4StringTag";
const string GUIStateParams::m_openDataSetsTag = "OpenDataSetsTag";
const string GUIStateParams::_flowDimensionalityTag = "_flowDimensionalityTag";
const string GUIStateParams::DataSetParam::m_dataSetPathsTag = "DataSetPathsTag";
const string GUIStateParams::DataSetParam::m_dataSetFormatTag = "DataSetFormatTag";

//
// Register class with object factory!!!
//
static ParamsRegistrar<GUIStateParams>               registrar(GUIStateParams::GetClassType());
static ParamsRegistrar<GUIStateParams::DataSetParam> registrar_dsp(GUIStateParams::DataSetParam::GetClassType());

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
GUIStateParams::GUIStateParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, GetClassType())
{
    _init();

    m_activeRenderer = new ActiveRenderer(ssave);
    m_activeRenderer->GetNode()->SetParent(GetNode());

    m_mouseModeParams = new MouseModeParams(ssave);
    m_mouseModeParams->GetNode()->SetParent(GetNode());

    // Create a Params container for multiple opacity maps
    //
    m_openDataSets = new ParamsContainer(ssave, m_openDataSetsTag);
    m_openDataSets->SetParent(this);
}

GUIStateParams::GUIStateParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    // If node isn't tagged correctly we correct the tag and reinitialize
    // from scratch;
    //
    if (node->GetTag() != GUIStateParams::GetClassType()) {
        node->SetTag(GUIStateParams::GetClassType());
        _init();
    }

    if (node->HasChild(MouseModeParams::GetClassType())) {
        m_mouseModeParams = new MouseModeParams(ssave, node->GetChild(MouseModeParams::GetClassType()));
    } else {
        m_mouseModeParams = new MouseModeParams(ssave);
        m_mouseModeParams->GetNode()->SetParent(GetNode());
    }

    if (node->HasChild(ActiveRenderer::GetClassType())) {
        m_activeRenderer = new ActiveRenderer(ssave, node->GetChild(ActiveRenderer::GetClassType()));
    } else {
        m_activeRenderer = new ActiveRenderer(ssave);
        m_activeRenderer->GetNode()->SetParent(GetNode());
    }

    if (node->HasChild(m_openDataSetsTag)) {
        m_openDataSets = new ParamsContainer(ssave, node->GetChild(m_openDataSetsTag));
    } else {
        // Node doesn't contain a opacity map
        //
        m_openDataSets = new ParamsContainer(ssave, m_openDataSetsTag);
        m_openDataSets->SetParent(this);
    }
}

GUIStateParams::GUIStateParams(const GUIStateParams &rhs) : ParamsBase(rhs)
{
    m_mouseModeParams = new MouseModeParams(*(rhs.m_mouseModeParams));
    m_activeRenderer = new ActiveRenderer(*(rhs.m_activeRenderer));
}

GUIStateParams &GUIStateParams::operator=(const GUIStateParams &rhs)
{
    m_mouseModeParams = new MouseModeParams(*(rhs.m_mouseModeParams));
    m_activeRenderer = new ActiveRenderer(*(rhs.m_activeRenderer));

    return (*this);
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
GUIStateParams::~GUIStateParams() {}

//----------------------------------------------------------------------------
// Getters and Setters
//----------------------------------------------------------------------------

string GUIStateParams::GetActiveVizName() const
{
    string defaultv;
    return (GetValueString(m_activeVisualizer, defaultv));
}

void GUIStateParams::SetActiveVizName(string vizWin) { SetValueString(m_activeVisualizer, "Set active visualizer window", vizWin); }

//! Get active renderer class and instance name for a visualizer
//
void GUIStateParams::GetActiveRenderer(string vizWin, string &renderType, string &renderInst) const { m_activeRenderer->GetActiveRenderer(vizWin, renderType, renderInst); }

//! Set active renderer class and instance name for a visualizer
//
void GUIStateParams::SetActiveRenderer(string vizWin, string renderType, string renderInst) { m_activeRenderer->SetActiveRenderer(vizWin, renderType, renderInst); }

string GUIStateParams::GetCurrentSessionFile() const { return (GetValueString(m_sessionFileTag, "")); }

void GUIStateParams::SetCurrentSessionFile(string path) { SetValueString(m_sessionFileTag, "Set current session file path", path); }

//! method identifies the current session file
//! \retval session file path
string GUIStateParams::GetCurrentImagePath() const { return (GetValueString(m_imagePathTag, QDir::homePath().toStdString())); }

//! method sets the current session path
//! \param[in] path string
void GUIStateParams::SetCurrentImagePath(string path) { SetValueString(m_imagePathTag, "Set current image path", path); }

//! method identifies the current session file
//! \retval session file path
string GUIStateParams::GetCurrentImageSavePath() const { return (GetValueString(m_imageSavePathTag, QDir::homePath().toStdString())); }

//! method sets the current session path
//! \param[in] path string
void GUIStateParams::SetCurrentImageSavePath(string path) { SetValueString(m_imageSavePathTag, "Set current image path", path); }

//! method identifies the current session file
//! \retval session file path
string GUIStateParams::GetCurrentTFPath() { return (GetValueString(m_tfPathTag, QDir::homePath().toStdString())); }

//! method sets the current session path
//! \param[in] path string
void GUIStateParams::SetCurrentTFPath(string path) { SetValueString(m_tfPathTag, "Set current tf path", path); }

//! method identifies the current session file
//! \retval session file path
string GUIStateParams::GetCurrentPythonPath() const { return (GetValueString(m_pythonPathTag, QDir::homePath().toStdString())); }

//! method sets the current session path
//! \param[in] path string
void GUIStateParams::SetCurrentPythonPath(string path) { SetValueString(m_pythonPathTag, "Set current python path", path); }

//! method identifies the current session file
//! \retval session file path
string GUIStateParams::GetCurrentFlowPath() const { return (GetValueString(m_flowPathTag, QDir::homePath().toStdString())); }

//! method sets the current session path
//! \param[in] path string
void GUIStateParams::SetCurrentFlowPath(string path) { SetValueString(m_flowPathTag, "Set current flow path", path); }

MouseModeParams *GUIStateParams::GetMouseModeParams() const { return (m_mouseModeParams); }

//----------------------------------------------------------------------------
// Funcs
//----------------------------------------------------------------------------

vector<string> GUIStateParams::GetOpenDataSetPaths(string dataSetName) const
{
    DataSetParam *dsParams = (DataSetParam *)m_openDataSets->GetParams(dataSetName);

    if (!dsParams) { return (vector<string>()); }

    vector<string> paths = dsParams->GetPaths();
    if (paths.empty()) paths.push_back("");

    return (paths);
}

string GUIStateParams::GetOpenDataSetFormat(string dataSetName) const
{
    DataSetParam *dsParams = (DataSetParam *)m_openDataSets->GetParams(dataSetName);

    if (!dsParams) { return (""); }

    return (dsParams->GetFormat());
}

void GUIStateParams::InsertOpenDateSet(string dataSetName, string format, const vector<string> &paths)
{
    m_openDataSets->Remove(dataSetName);

    DataSetParam dsParam(_ssave);
    dsParam.SetPaths(paths);
    dsParam.SetFormat(format);

    m_openDataSets->Insert(&dsParam, dataSetName);
}

void GUIStateParams::_init() { SetActiveVizName(""); }

void GUIStateParams::ActiveRenderer::SetActiveRenderer(string vizWin, string renderType, string renderInst)
{
    std::vector<string> v;
    v.push_back(renderType);
    v.push_back(renderInst);
    SetValueStringVec(vizWin, "Set active render type and instance", v);
}

void GUIStateParams::ActiveRenderer::GetActiveRenderer(string vizWin, string &renderType, string &renderInst) const
{
    std::vector<string> defaultv(2, "");
    std::vector<string> v = GetValueStringVec(vizWin, defaultv);
    renderType = v[0];
    renderInst = v[1];
}

std::string GUIStateParams::GetStatsDatasetName() const { return GetValueString(m_statsDatasetNameTag, ""); }

void GUIStateParams::SetStatsDatasetName(std::string &name) { SetValueString(m_statsDatasetNameTag, "Name of the active data set in Statistics", name); }

std::string GUIStateParams::GetPlotDatasetName() const { return GetValueString(m_plotDatasetNameTag, ""); }

void GUIStateParams::SetPlotDatasetName(std::string &name) { SetValueString(m_plotDatasetNameTag, "Name of the active data set in Plot", name); }

string GUIStateParams::ActiveTab() const { return GetValueString("ActiveTabTag", ""); }
void   GUIStateParams::SetActiveTab(const string &t) { SetValueString("ActiveTabTag", "", t); }

int GUIStateParams::GetFlowDimensionality() const { return GetValueLong(_flowDimensionalityTag, -1); }

void GUIStateParams::SetFlowDimensionality(int nDims)
{
    if (nDims > 3)
        nDims = 3;
    else if (nDims < 2)
        nDims = 2;

    SetValueLong(_flowDimensionalityTag, _flowDimensionalityTag, nDims);
}
