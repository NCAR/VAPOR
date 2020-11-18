#pragma once

#include "VNumericFormatMenu.h"

class VDoubleLineEditAction;

//! \class VDoubleRangeMenu
//! \ingroup Public_GUI
//! \brief A menu that allows the user to control the range of double values
//! that can be set by a widget.  Also allows for setting the numeric format
//! that the number is displayed with.

class VDoubleRangeMenu : public VNumericFormatMenu {
    Q_OBJECT

public:
    explicit VDoubleRangeMenu(QWidget *parent, bool sciNotation, double decimalDigits, double min, double max, bool rangeChangable);

    void AllowUserRange(bool allowed = true);

protected:
    VDoubleLineEditAction *_minRangeAction;
    VDoubleLineEditAction *_maxRangeAction;

public slots:
    //! Set the minimum value that the current widget can use
    void SetMinimum(double min);

    //! Set the maximum value that the current widget can use
    void SetMaximum(double max);

private slots:
    void _minChanged(double min);
    void _maxChanged(double max);

signals:
    void MinChanged(double min);
    void MaxChanged(double max);
};
