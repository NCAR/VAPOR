#pragma once

#include "VLineEditTemplate.h"

class VDoubleLineEdit3 : public VLineEditTemplate<double> {
    public:
        VDoubleLineEdit3( double value, bool useMenu=true ) 
        : VLineEditTemplate<double>(value, useMenu) {}
};
