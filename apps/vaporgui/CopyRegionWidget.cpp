//************************************************************************
//							  *
//	   Copyright (C)  2017					*
//	 University Corporation for Atmospheric Research		  *
//	   All Rights Reserved					*
//							  *
//************************************************************************/
//
//  File:	   CopyRegionWidget.cpp
//
//  Author:	 Scott Pearse
//	  National Center for Atmospheric Research
//	  PO 3000, Boulder, Colorado
//
//  Date:	   March 2017
//
//  Description:	Implements the CopyRegionWidget class.  This provides
//  a widget that is inserted in the "Appearance" tab of various Renderer GUIs
//
#include <sstream>
#include <qwidget.h>
#include <QFileDialog>
#include "vapor/ParamsMgr.h"
#include "vapor/RenderParams.h"
#include "vapor/DataMgrUtils.h"
#include "CopyRegionWidget.h"

using namespace VAPoR;

namespace {
template<typename Out> void split(const std::string &s, char delim, Out result)
{
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) { *(result++) = item; }
}

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}
}    // namespace

CopyRegionWidget::CopyRegionWidget(QWidget *parent) : QWidget(parent), Ui_CopyRegionWidgetGUI()
{
    setupUi(this);

    _paramsMgr = NULL;
    _rParams = NULL;

    connect(copyButton, SIGNAL(released()), this, SLOT(copyRegion()));
}

CopyRegionWidget::~CopyRegionWidget() {}

void CopyRegionWidget::updateCopyCombo()
{
    copyCombo->clear();

    std::vector<string> visNames = _paramsMgr->GetVisualizerNames();
    _visNames.clear();

    _dataSetNames = _paramsMgr->GetDataMgrNames();

    for (int i = 0; i < visNames.size(); i++) {
        for (int ii = 0; ii < _dataSetNames.size(); ii++) {
            // Create a mapping of abreviated visualizer names to their
            // actual string values.
            //
            string visAbb = "Vis" + std::to_string(i);
            _visNames[visAbb] = visNames[i];

            string dataSetName = _dataSetNames[ii];

            std::vector<string> typeNames;
            typeNames = _paramsMgr->GetRenderParamsClassNames(visNames[i]);

            for (int j = 0; j < typeNames.size(); j++) {
                // Abbreviate Params names by removing 'Params" from them.
                // Then store them in a map for later reference.
                //
                string typeAbb = typeNames[j];
                int    pos = typeAbb.find("Params");
                typeAbb.erase(pos, 6);
                _renTypeNames[typeAbb] = typeNames[j];

                std::vector<string> renNames;
                renNames = _paramsMgr->GetRenderParamInstances(visNames[i], _dataSetNames[ii], typeNames[j]);

                for (int k = 0; k < renNames.size(); k++) {
                    string  displayName = visAbb + ":" + dataSetName + ":" + typeAbb + ":" + renNames[k];
                    QString qDisplayName = QString::fromStdString(displayName);
                    copyCombo->addItem(qDisplayName);
                }
            }
        }
    }
}

void CopyRegionWidget::copyRegion()
{
    string copyString = copyCombo->currentText().toStdString();
    if (copyString != "") {
        std::vector<std::string> elems = split(copyString, ':');
        string                   visualizer = _visNames[elems[0]];
        string                   dataSetName = elems[1];
        string                   renType = _renTypeNames[elems[2]];
        string                   renderer = elems[3];

        RenderParams *copyParams = _paramsMgr->GetRenderParams(visualizer, dataSetName, renType, renderer);
        assert(copyParams);

        Box *               copyBox = copyParams->GetBox();
        std::vector<double> minExtents, maxExtents;
        copyBox->GetExtents(minExtents, maxExtents);

        Box *               myBox = _rParams->GetBox();
        std::vector<double> myMin, myMax;
        myBox->GetExtents(myMin, myMax);
        assert(minExtents.size() == maxExtents.size());
        for (int i = 0; i < minExtents.size(); i++) {
            myMin[i] = minExtents[i];
            myMax[i] = maxExtents[i];
        }
        myBox->SetExtents(myMin, myMax);

        emit valueChanged();
    }
}

void CopyRegionWidget::Update(ParamsMgr *paramsMgr, RenderParams *rParams)
{
    assert(paramsMgr);
    assert(rParams);

    _paramsMgr = paramsMgr;
    _rParams = rParams;

    updateCopyCombo();
    adjustSize();
}
