#pragma once

#include "VNumericFormatMenu.h"

class VDoubleLineEditAction;

class VDoubleRangeMenu : public VNumericFormatMenu {
    Q_OBJECT

public:
    explicit VDoubleRangeMenu( 
        QWidget* parent, 
        bool sciNotation, double decimalDigits,
        double min, double max 
    );

protected:
    VDoubleLineEditAction* _minRangeAction;
    VDoubleLineEditAction* _maxRangeAction;

public:
    void SetMinRange( double min );
    void SetMaxRange( double max );

private slots:
    void _minChanged( double min );
    void _maxChanged( double max );

signals:
    void MinChanged( double min );
    void MaxChanged( double max );
};
