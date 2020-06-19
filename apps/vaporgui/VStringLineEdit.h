#pragma once

#include "VLineEditTemplate.h"

class VStringLineEdit3 : public VLineEditTemplate<std::string> {
    public:
        VStringLineEdit3( const std::string& value ) 
        : VLineEditTemplate<std::string>(value) {}
};
