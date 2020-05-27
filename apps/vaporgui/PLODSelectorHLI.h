#pragma once

#include "PWidgetHLI.h"
#include "PLODSelector.h"

template<class P>
class PLODSelectorHLI :
    public PLODSelector,
    public PWidgetHLIBase<P, long>
{
    public:

    PLODSelectorHLI(
        typename PWidgetHLIBase<P, long>::GetterType getter,
        typename PWidgetHLIBase<P, long>::SetterType setter
    ) : PLODSelector(),
        PWidgetHLIBase<P, long> (
            (PWidget*)this,
            getter,
            setter
        ) {}
};
