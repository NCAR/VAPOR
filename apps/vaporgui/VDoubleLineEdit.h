#pragma once

#include "VLineEditTemplate.h"

class VDoubleLineEdit : public VLineEditTemplate<double> {
    public:
        VDoubleLineEdit( double value, bool useMenu=true ) 
        : VLineEditTemplate<double>(value, useMenu) {}
};
