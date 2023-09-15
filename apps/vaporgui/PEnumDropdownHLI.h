#pragma once

#include "PWidgetHLI.h"
#include "PEnumDropdown.h"

template<class P> class PEnumDropdownHLI : public PEnumDropdown, public PWidgetHLIBase<P, long> {
public:
    PEnumDropdownHLI(const std::string &label, const std::vector<std::string> &items, const std::vector<long> &values, typename PWidgetHLIBase<P, long>::GetterType getter,
                     typename PWidgetHLIBase<P, long>::SetterType setter)
    : PEnumDropdown("", items, values, label), PWidgetHLIBase<P, long>((PWidget *)this, getter, setter)
    {
    }
};
