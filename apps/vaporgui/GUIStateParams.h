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

class MouseModeParams;

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
    string GetCurrentSessionFile() const;

    //! method sets the current session file path
    //! \param[in] path string
    void SetCurrentSessionFile(string path);

    //! Get names of currently opened data sets
    //!
    std::vector<string> GetOpenDataSetNames() const { return (m_openDataSets->GetNames()); }

    std::vector<string> GetOpenDataSetPaths(string dataSetName) const;

    string GetOpenDataSetFormat(string dataSetName) const;

    void RemoveOpenDateSet(string dataSetName) { m_openDataSets->Remove(dataSetName); }

    void InsertOpenDateSet(string dataSetName, string format, const std::vector<string> &paths);

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
    string GetCurrentImageSavePath() const;

    //! method sets the current session path
    //! \param[in] path string
    void SetCurrentImageSavePath(string path);

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

    //! method sets and gets the active data set name in Statistics
    //!
    std::string GetStatsDatasetName() const;
    void        SetStatsDatasetName(std::string &name);

    //! method sets and gets the active data set name in Plot utility
    //!
    std::string GetPlotDatasetName() const;
    void        SetPlotDatasetName(std::string &name);

    string ActiveTab() const;
    void   SetActiveTab(const string &t);

    int  GetFlowDimensionality() const;
    void SetFlowDimensionality(int nDims);

    void SetProjectionString(string proj4String) { SetValueString(m_proj4StringTag, "Set Proj4 projection string", proj4String); }

    string GetProjectionString() const
    {
        string defaultv;
        return (GetValueString(m_proj4StringTag, defaultv));
    }

    class DataSetParam : public ParamsBase {
    public:
        DataSetParam(VAPoR::ParamsBase::StateSave *ssave) : ParamsBase(ssave, DataSetParam::GetClassType()) {}

        DataSetParam(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node) : ParamsBase(ssave, node) {}

        virtual ~DataSetParam(){};

        void SetPaths(const std::vector<string> &paths) { SetValueStringVec(m_dataSetPathsTag, "Data set paths", paths); }

        std::vector<string> GetPaths() const { return (GetValueStringVec(m_dataSetPathsTag)); };

        void SetFormat(string format) { SetValueString(m_dataSetFormatTag, "Data set format", format); }

        string GetFormat() const { return (GetValueString(m_dataSetFormatTag, "vdc")); }

        static string GetClassType() { return ("DataSetParam"); }

    private:
        static const string m_dataSetPathsTag;
        static const string m_dataSetFormatTag;
    };

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
    static const string m_imagePathTag;
    static const string m_imageSavePathTag;
    static const string m_pythonPathTag;
    static const string m_flowPathTag;
    static const string m_tfPathTag;
    static const string m_statsDatasetNameTag;
    static const string m_plotDatasetNameTag;
    static const string m_proj4StringTag;
    static const string m_openDataSetsTag;
    static const string _flowDimensionalityTag;

    MouseModeParams *m_mouseModeParams;

    VAPoR::ParamsContainer *m_openDataSets;

    void _init();
};

#endif    // GUISTATEPARAMS_H
