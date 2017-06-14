//************************************************************************
//															*
//		     Copyright (C)  2011										*
//     University Corporation for Atmospheric Research					*
//		     All Rights Reserved										*
//															*
//************************************************************************/
//
//	File:		boxsliders.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		July 2011
//
//	Description:	Defines the BoxSliderFrame class.  This provides
//		a frame that supports sliders and textboxes to control a box
//
#ifndef BOXSLIDERFRAME_H
#define BOXSLIDERFRAME_H
#include <cassert>
#include <QFrame>
#include <qwidget.h>
#include "boxframe.h"
#include <vector>

namespace VAPoR {
class DataStatus;
class ControlExec;
}    // namespace VAPoR

//! \class BoxSliderFrame
//! \ingroup Public_GUI
//! \brief A QFrame that contains sliders and text boxes controlling the extents of a box
//! \author Alan Norton
//! \version 3.0
//! \date    May 2015

//!	This QFrame should be embedded in various EventRouter .ui files to support controlling box extents.
//! EventRouter implementors must do the following:
//!
//!		If a slider or text box should not be displayed, invoke hide() on that gui element in the
//! EventRouter constructor.
//!
//!		In EventRouter::updateTab(), invoke setFullDomain(), setBoxExtents(), setNumRefinements() and setVariableName()
//! based on the current state of the Box.
//!
//!		Whenever the box extents are changed in the GUI (outside the BoxSliderFrame), invoke setBoxExtents().
//!
//!		Whenever the refinement level is changed in the GUI, invoke setNumRefinements()
//!
//!		Implement the slot EventRouter::changeExtents() and connect it to the signal BoxSliderFrame::changeExtents().
class BoxSliderFrame : public QFrame, public Ui_boxframe {
    Q_OBJECT

public:
    BoxSliderFrame(QWidget *parent = 0);
    ~BoxSliderFrame();

    //! Specify the full domain of the data in which the box extents reside
    //! \param[in] exts double[6] array of extents
    void setFullDomain(const double exts[6]);
    void setFullDomain(std::vector<double> minExt, std::vector<double> maxExt)
    {
        assert(minExt.size() == maxExt.size());
        assert(minExt.size() == 3);
        double exts[6];
        for (int i = 0; i < minExt.size(); i++) {
            exts[i] = minExt[i];
            exts[i + 3] = maxExt[i];
        }
        setFullDomain(exts);
    }

    //! Specify the box extents associated with the box slider.
    //! \param[in] minExts vector<double> of minimum box extents
    //! \param[in] maxExts vector<double> of maximum box extents
    void setBoxExtents(const std::vector<double> &minExts, const std::vector<double> &maxExts);

    //! Identify the current extents of the box in user coordinates.  Should be invoked in
    //! EventRouter::changeExtents() to update the current extents after a change.
    void getBoxExtents(double[6]);
    void getBoxExtents(std::vector<double> &minExt, std::vector<double> &maxExt)
    {
        minExt.clear();
        maxExt.clear();
        double extents[6];
        getBoxExtents(extents);
        for (int i = 0; i < 3; i++) {
            minExt.push_back(extents[i]);
            maxExt.push_back(extents[i + 3]);
        }
    }

    //! Specify the variable whose extents define the domain of the box.
    //! \param[in] vname Variable name.
    void setVariableName(std::string vname) { _varname = vname; }

    //! Specify the number of refinements of the extent-defining variable to be used in identifying
    //! the voxel size.
    //! \param[in] numrefs Number of refinements.
    void        setNumRefinements(int numrefs) { _numRefinements = numrefs; }
    static void setDataStatus(VAPoR::DataStatus *ds) { _dataStatus = ds; }
    static void SetControlExec(VAPoR::ControlExec *ce) { _controlExec = ce; }
signals:
    //! \sa This signal indicates that the user has changed the extents of the box, and the
    //! eventRouter must respond to this change.  This signal must be connected to the
    //! changeExtents() slot of the EventRouter.
    void extentsChanged();

#ifndef DOXYGEN_SKIP_THIS
protected slots:
    void boxTextChanged(const QString &);
    void boxReturnPressed();
    void xSliderCtrChange();
    void ySliderCtrChange();
    void zSliderCtrChange();
    void xSliderSizeChange();
    void ySliderSizeChange();
    void zSliderSizeChange();
    void nudgeXCenter(int);
    void nudgeYCenter(int);
    void nudgeZCenter(int);
    void nudgeXSize(int);
    void nudgeYSize(int);
    void nudgeZSize(int);

private:
    static VAPoR::DataStatus * _dataStatus;
    static VAPoR::ControlExec *_controlExec;
    void                       updateGuiValues(const double mid[3], const double size[3]);
    void                       confirmText();
    void                       nudgeCenter(std::string varname, int val, int dir);
    void                       nudgeSize(int val, int dir);
    bool                       _textChangedFlag;
    bool                       _silenceSignals;
    double                     _boxExtents[6];
    double                     _domainExtents[6];
    int                        _voxelDims[3];
    int                        _numRefinements;
    int                        _lastCenterSlider[3];
    int                        _lastSizeSlider[3];
    std::string                _varname;
#endif    // DOXYGEN_SKIP_THIS
};

#endif
