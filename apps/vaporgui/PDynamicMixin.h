#pragma once

#include <string>
class PWidget;

//! \class PDynamicMixin
//! Internal class for PWidgets. Enables dynamic updating of the params database
//! as the user edits a value through intermediate updates.

class PDynamicMixin {
public:
    //! Turns on dynamic update.
    PWidget *EnableDynamicUpdate();
    virtual ~PDynamicMixin() = default;

protected:
    void dynamicSetParamsDouble(double v);
    void dynamicSetParamsLong(long v);
    void dynamicSetParamsString(const std::string &v);

private:
    PWidget *getPWidget();
};
