#pragma once

#include "VLineEditTemplate.h"

class VIntLineEdit : public VLineEditTemplate<int> {
    public:
        VIntLineEdit( int value, bool useMenu=true ) 
        : VLineEditTemplate<int>(value, useMenu) {}
};
