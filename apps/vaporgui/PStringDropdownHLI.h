#pragma once

#include "PWidgetHLI.h"
#include "PStringDropdown.h"

template<class P> class PStringDropdownHLI : public PStringDropdown, public PWidgetHLIBase<P, std::string> {
public:
    PStringDropdownHLI(const std::string &label, const std::vector<std::string> &items, typename PWidgetHLIBase<P, std::string>::GetterType getter,
                       typename PWidgetHLIBase<P, std::string>::SetterType setter)
    : PStringDropdown("", items, label), PWidgetHLIBase<P, std::string>((PWidget *)this, getter, setter)
    {
    }
};
