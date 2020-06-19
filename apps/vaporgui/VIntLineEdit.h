#pragma once

#include "VLineEditTemplate.h"

class VIntLineEdit3 : public VLineEditTemplate<int> {
    public:
        VIntLineEdit3( int value, bool useMenu=true ) 
        : VLineEditTemplate<int>(value, useMenu) {}
};
