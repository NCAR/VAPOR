#pragma once

#include "VContainer.h"
#include "ParamsUpdatable.h"

//! \class UWidget
//! \brief A widget that standardizes support for params updates.
//! \author Stas Jaroszynski

class UWidget : public VContainer, public ParamsUpdatable {
public:
    using VContainer::VContainer;
};
