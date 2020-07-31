#pragma once

#include <QMenu>

class VCheckBoxAction;
class VSpinBoxAction;

//! \class VNumericFormatMenu
//! \ingroup Public_GUI
//! \brief A menu that allows users to specify how a number is displayed in a line edit,
//! in regard to how many digits are displayed, and whether scientific notation is used.

class VNumericFormatMenu : public QMenu {
    Q_OBJECT

public:
    explicit VNumericFormatMenu(QWidget *parent, bool sciNotation, int decimalDigits);

    //! Set the number of decimal digits used by the clicked line-edit
    void SetDecimalDigits(int digits);

    //! Set whether the current line-edit is using scientific notation
    void SetSciNotation(bool sciNotation);

protected:
    VCheckBoxAction *_sciNotationAction;
    VSpinBoxAction * _decimalAction;

private slots:
    void _decimalDigitsChanged(int digits);
    void _sciNotationChanged(bool sciNotation);

signals:
    void DecimalDigitsChanged(int decimalDigits);
    void SciNotationChanged(bool sciNotation);
};
