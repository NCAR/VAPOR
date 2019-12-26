#include "VFrame.h"

#include <QVBoxLayout>


VFrame::VFrame() 
{
        setLayout( new QVBoxLayout );
        layout()->setContentsMargins( 0, 0, 0, 0 );
        layout()->setSpacing( 12 );
        setFrameStyle( QFrame::NoFrame );
}

void VFrame::addWidget( QWidget* widget) 
{
        layout()->addWidget(widget);
}
