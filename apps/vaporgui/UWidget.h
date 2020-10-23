#pragma once

#include "VContainer.h"
#include "Updateable.h"

//! \class UWidget
//! \brief A widget that standardizes support for params updates.
//! \author Stas Jaroszynski

class UWidget : public VContainer, public Updateable {
public:
    using VContainer::VContainer;
};
