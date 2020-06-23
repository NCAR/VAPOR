#include <cmath>
#include <iostream>

#include <QMenu>

#include "vapor/VAssert.h"

#include "VSliderEditInterface.h"
#include "VSlider.h"
#include "VActions.h"

VSliderEditInterface::VSliderEditInterface( 
) : VContainer()
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    setFrameStyle(QFrame::Panel | QFrame::Raised );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &VSliderEditInterface::customContextMenuRequested,
        this, &VSliderEditInterface::ShowContextMenu );
}
