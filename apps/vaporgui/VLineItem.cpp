#include <iostream>

#include "VLineItem.h"
#include <QHBoxLayout>
#include <QLabel>

const int VLineItem::_LEFT_MARGIN   = 0;
const int VLineItem::_TOP_MARGIN    = 0;
const int VLineItem::_RIGHT_MARGIN  = 0;
const int VLineItem::_BOTTOM_MARGIN = 0;

VLineItem::VLineItem(const std::string &label, QLayoutItem *centerItem, QWidget *rightWidget)
: VLineItem(label)
{
    layout()->addItem(centerItem);
    //rightWidget->setMinimumWidth( width()/3. );
    // Our rightWidget is a VContainer, that may hold multiple other VContainers.
    // Find out how many VContainers are at the branch below the top level VContainer.
    //int count = rightWidget->layout()->itemAt(0)->layout()->count();
    /*QLayout* rightLayout = rightWidget->layout();
    if ( rightLayout != nullptr ) {
        int count = rightLayout->count();
        std::cout << "count " << count << std::endl;
        if ( count > 1 ) {
            rightWidget->setMaximumWidth( width()/2. );
            //rightWidget->setMinimumWidth( width()/2. );
            std::cout << "trying to lengthen composite widgets" << std::endl;
            for (int i=0; i<count; i++) {
                QWidget* widget = rightLayout->itemAt(i)->widget();
                widget->setMinimumWidth( rightWidget->width() / (2. * count ) );
            }
        }    
        else {
            rightWidget->setMaximumWidth( width()/3. );
        }
    }*/
    //rightWidget->setMaximumWidth( width()/3. );
    layout()->addWidget(rightWidget);
}

VLineItem::VLineItem(const std::string &label, QWidget *centerWidget, QWidget *rightWidget)
: VLineItem(label)
{
    //centerWidget->setMinimumWidth( width()/3. );
    //centerWidget->setMinimumWidth( width()/4. );
    layout()->addWidget(centerWidget);
    //rightWidget->setMinimumWidth( width()/3. );

    //rightWidget->setMaximumWidth( width()/3. );
    //rightWidget->setMinimumWidth( width()/4. );
    layout()->addWidget(rightWidget);
}

VLineItem::VLineItem(const std::string &label, QWidget *rightWidget)
: VLineItem(
    label, 
    (QLayoutItem*)new QSpacerItem(
        0,
        20, 
        //QSizePolicy::Ignored, 
        //QSizePolicy::Expanding, 
        //QSizePolicy::MinimumExpanding, 
        //QSizePolicy::Minimum, 
        QSizePolicy::Maximum, 
        QSizePolicy::Minimum), 
    rightWidget
) {}

VLineItem::VLineItem(const std::string &label)
{
    setLayout(new QHBoxLayout);
    layout()->setContentsMargins(
        _LEFT_MARGIN,
        _TOP_MARGIN,
        _RIGHT_MARGIN,
        _BOTTOM_MARGIN
    );

    QLabel* qLabel = new QLabel(label.c_str());
    qLabel->setMinimumWidth( width()/3. );
    layout()->addWidget( qLabel );
}
