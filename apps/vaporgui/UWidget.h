#pragma once

#include "VContainer.h"
#include "Updateable.h"

class UWidget : public VContainer, public Updateable {
public:
    using VContainer::VContainer;
};
