//************************************************************************
//                                                                      *
//           Copyright (C)  2016                                        *
//     University Corporation for Atmospheric Research                  *
//           All Rights Reserved                                        *
//                                                                      *
//************************************************************************/
//
//  File:       plot.h
//
//  Author:     Samuel Li
//              National Center for Atmospheric Research
//              PO 3000, Boulder, Colorado
//
//  Date:       January 2018
//

#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#include "GUIStateParams.h"
#include <vapor/MyPython.h>
#include <vapor/GetAppPath.h>
#include "Plot.h"

#define NPY_NO_DEPRECATED_API NPY_1_8_API_VERSION
#include <numpy/ndarrayobject.h>


// Constructor
Plot::Plot( QWidget* parent )
{
    _controlExec = NULL;
    
    setupUi(this);
    setWindowTitle("Statistics");
    
    Connect();
}


// Destructor
Plot::~Plot()
{}


void Plot::showMe()
{
    show();
    raise();
    activateWindow();
}


void Plot::Update()
{}


bool Plot::Connect()
{
    return true;
}


int Plot::initControlExec(VAPoR::ControlExec* ce) 
{
    if (ce!=NULL) 
        _controlExec = ce;
    else 
        return -1;

    // Store the active dataset name 
    std::vector<std::string> dmNames = _controlExec->getDataStatus()->GetDataMgrNames();
    if( dmNames.empty() )
        return -1;
    else
    {
        GUIStateParams* guiParams = dynamic_cast<GUIStateParams*>
                        (_controlExec->GetParamsMgr()->GetParams( GUIStateParams::GetClassType() ));
        std::string dsName = guiParams->GetStatsDatasetName();
        if( dsName == "" || dsName == "NULL" )      // not initialized yet
            guiParams->SetStatsDatasetName( dmNames[0] );
    }

    return 0;
}


