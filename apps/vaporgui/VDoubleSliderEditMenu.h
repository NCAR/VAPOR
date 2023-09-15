#pragma once

#include <QMenu>
#include "VDoubleRangeMenu.h"

class VCheckBoxAction;

//! \class VDoubleSliderEditMenu
//! \ingroup Public_GUI
//! \brief A menu for VDoubleSliderEdit that allows for setting
//! a min/max range, numeric formatting controls, and dynamic update controls
//! in regard to how many digits are displayed, and whether scientific notation is used.

// clang-format off

class VDoubleSliderEditMenu : public VDoubleRangeMenu {
    Q_OBJECT

public:
    explicit VDoubleSliderEditMenu(QWidget *parent);

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
