#pragma once

#include "VNumericFormatMenu.h"

class VDoubleLineEditAction;

class VDoubleRangeMenu : public VNumericFormatMenu {
    Q_OBJECT

public:
    explicit VDoubleRangeMenu( 
        QWidget* parent, 
        bool sciNotation, 
        double decimalDigits,
        double min, 
        double max,
        bool rangeChangable
    );

protected:
    VDoubleLineEditAction* _minRangeAction;
    VDoubleLineEditAction* _maxRangeAction;

public:
    void SetMinimum( double min );
    void SetMaximum( double max );

private slots:
    void _minChanged( double min );
    void _maxChanged( double max );

signals:
    void MinChanged( double min );
    void MaxChanged( double max );
};
