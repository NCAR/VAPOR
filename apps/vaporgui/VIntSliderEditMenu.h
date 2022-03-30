#pragma once

#include <QMenu>
#include "VIntRangeMenu.h"

class VCheckBoxAction;

//! \class VIntSliderEditMenu
//! \ingroup Public_GUI
//! \brief A menu for VIntSliderEdit that allows for setting
//! a min/max range, numeric formatting controls, and dynamic update controls
//! in regard to how many digits are displayed, and whether scientific notation is used.

// clang-format off

class VIntSliderEditMenu : public VIntRangeMenu {
    Q_OBJECT

public:
    explicit VIntSliderEditMenu(
        QWidget *parent, 
        bool sciNotation, 
        int decimalDigits, 
        double min, 
        double max, 
        bool rangeChangeable
    );

    //! Allow the toggling of the dynamic update checkbox in the right-click menu
    void AllowDynamicUpdate() const;

protected:
    VCheckBoxAction *_dynamicUpdateAction;

public slots:
    //! Check/Uncheck the dynamic update checkbox
    void SetDynamicUpdate(bool enabled);

private slots:
    void _dynamicUpdateChanged(bool dynamicUpdateEnabled);

signals:
    void DynamicUpdateChanged(bool dynamicUpdateEnabled);
};

// clang-format on
