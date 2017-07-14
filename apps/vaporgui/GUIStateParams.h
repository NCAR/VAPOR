//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		GUIStateParams
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2016
//
//	Description:	Maintains various GUI state settings
//
#ifndef GUISTATEPARAMS_H
#define GUISTATEPARAMS_H

#include <vapor/ParamsBase.h>
#include "MouseModeParams.h"

class GUIStateParams : public VAPoR::ParamsBase {
public:
    GUIStateParams(VAPoR::ParamsBase::StateSave *ssave);

    GUIStateParams(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);

    GUIStateParams(const GUIStateParams &rhs);

    GUIStateParams &operator=(const GUIStateParams &rhs);

    virtual ~GUIStateParams();

    string GetActiveVizName() const;
    void   SetActiveVizName(string vizWin);

    //! Get active renderer class and instance name for a visualizer
    //
    void GetActiveRenderer(string vizWin, string &renderType, string &renderInst) const;

    //! Get active renderer class and instance name for a visualizer
    //
    void SetActiveRenderer(string vizWin, string renderType, string renderInst);

    //! method identifies the current session file
    //! \retval session file path
    string GetCurrentSessionPath() const;

    //! method sets the current session path
    //! \param[in] path string
    void SetCurrentSessionPath(string path);

    //! Static method identifies the current session file
    //! \retval session file path
    void GetOpenDataSets(std::vector<string> &paths, std::vector<string> &names) const;

    //! method sets the current session path
    //! \param[in] path string
    void SetOpenDataSets(const std::vector<string> &paths, const std::vector<string> &names);

    //! method identifies the current session file
    //! \retval session file path
    string GetCurrentImagePath() const;

    //! method sets the current session path
    //! \param[in] path string
    void SetCurrentImagePath(string path);

    //! method identifies the current session file
    //! \retval session file path
    string GetCurrentTFPath();

    //! method sets the current session path
    //! \param[in] path string
    void SetCurrentTFPath(string path);

    //! method identifies the current session file
    //! \retval session file path
    string GetCurrentPythonPath() const;

    //! method sets the current session path
    //! \param[in] path string
    void SetCurrentPythonPath(string path);

    //! method identifies the current session file
    //! \retval session file path
    string GetCurrentFlowPath() const;

    //! method sets the current session path
    //! \param[in] path string
    void SetCurrentFlowPath(string path);

    MouseModeParams *GetMouseModeParams() const;

    // Get static string identifier for this params class
    //
    static string GetClassType() { return ("GUIStateParams"); }

private:
    class ActiveRenderer : public ParamsBase {
    public:
        ActiveRenderer(VAPoR::ParamsBase::StateSave *ssave) : ParamsBase(ssave, ActiveRenderer::GetClassType()) {}

        ActiveRenderer(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node) : ParamsBase(ssave, node) {}

        virtual ~ActiveRenderer(){};

        void SetActiveRenderer(string vizWin, string renderType, string renderInst);

        void GetActiveRenderer(string vizWin, string &renderType, string &renderInst) const;

        static string GetClassType() { return ("ActiveRenderer"); }
    };

    ActiveRenderer *m_activeRenderer;

    static const string m_activeVisualizer;
    static const string m_pathParamsTag;
    static const string m_sessionFileTag;
    static const string m_openDataTag;
    static const string m_imagePathTag;
    static const string m_pythonPathTag;
    static const string m_flowPathTag;
    static const string m_tfPathTag;

    MouseModeParams *m_mouseModeParams;

    void _init();
};

#endif    // GUISTATEPARAMS_H
