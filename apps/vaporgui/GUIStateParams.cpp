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

#include "GUIStateParams.h"

using namespace VAPoR;

const string GUIStateParams::m_activeVisualizer = "ActiveVisualizer";
const string GUIStateParams::m_pathParamsTag = "PathParamsTag";
const string GUIStateParams::m_sessionFileTag = "SessionFileTag";
const string GUIStateParams::m_openDataTag = "OpenDataTag";
const string GUIStateParams::m_imagePathTag = "ImagePathTag";
const string GUIStateParams::m_pythonPathTag = "PythonPathTag";
const string GUIStateParams::m_flowPathTag = "FlowPathTag";
const string GUIStateParams::m_tfPathTag = "TFPathTag";

//
// Register class with object factory!!!
//
static ParamsRegistrar<GUIStateParams> registrar(GUIStateParams::GetClassType());

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

void GUIStateParams::SetOpenDataSets(const std::vector<string> &paths, const std::vector<string> &names)
{
    assert(paths.size() == names.size());

    vector<string> v;
    for (int i = 0; i < paths.size(); i++) {
        v.push_back(names[i]);
        v.push_back(paths[i]);
    }
    SetValueStringVec(m_openDataTag, "open data sets", v);
}

void GUIStateParams::GetOpenDataSets(std::vector<string> &paths, std::vector<string> &names) const
{
    paths.clear();
    names.clear();

    std::vector<string> v = GetValueStringVec(m_openDataTag);
    assert((v.size() % 2) == 0);

    for (int i = 0; i < v.size(); i += 2) {
        names.push_back(v[i]);
        paths.push_back(v[i + 1]);
    }
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
