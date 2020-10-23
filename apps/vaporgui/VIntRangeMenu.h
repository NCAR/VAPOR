#pragma once

#include "VNumericFormatMenu.h"

class VIntLineEditAction;

//! \class VIntRangeMenu
//! \ingroup Public_GUI
//! \brief A menu that allows the user to control the range of integer values
//! that can be set by a widget.  Also allows for setting the numeric format
//! that the number is displayed with.

class VIntRangeMenu : public VNumericFormatMenu {
    Q_OBJECT

public:
    explicit VIntRangeMenu(QWidget *parent, bool sciNotation, int decimalDigits, int min, int max, bool rangeChangable = false);

    void AllowUserRange(bool allowed = true);

protected:
    VIntLineEditAction *_minRangeAction;
    VIntLineEditAction *_maxRangeAction;

public:
    //! Set the minimum value that the current widget can use
    void SetMinimum(int min);

    //! Set the maximum value that the current widget can use
    void SetMaximum(int max);

private slots:
    void _minChanged(int min);
    void _maxChanged(int max);

signals:
    void MinChanged(int min);
    void MaxChanged(int max);
};
