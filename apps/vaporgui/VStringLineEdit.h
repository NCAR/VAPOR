#pragma once

#include "VLineEditTemplate.h"

class VStringLineEdit : public VLineEditTemplate<std::string> {
    public:
        VStringLineEdit( const std::string& value ) 
        : VLineEditTemplate<std::string>(value) {}
};
