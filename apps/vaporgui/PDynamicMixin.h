#pragma once

#include <string>
class PWidget;

class PDynamicMixin {
public:
    PWidget *EnableDynamicUpdate();
    virtual ~PDynamicMixin() = default;
    
protected:
    void dynamicSetParamsDouble(double v);
    void dynamicSetParamsLong(long v);
    void dynamicSetParamsString(const std::string &v);
    
private:
    PWidget *getPWidget();
};
