#include <cmath>
#include <iostream>

#include <QMenu>

#include "vapor/VAssert.h"

#include "VAbstractSliderEdit.h"
#include "VSlider.h"
#include "VActions.h"

VAbstractSliderEdit::VAbstractSliderEdit( 
) : VContainer()
{
    setContextMenuPolicy( Qt::CustomContextMenu );

    setFrameStyle(QFrame::Panel | QFrame::Raised );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &VAbstractSliderEdit::customContextMenuRequested,
        this, &VAbstractSliderEdit::ShowContextMenu );
}
