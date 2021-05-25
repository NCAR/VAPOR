#pragma once

namespace VAPoR {
class ControlExecutive;
}    // namespace VAPoR

//! \class ControlExecWidget
//! \brief Provides an interface that standardizes objects that make use of the ControlExec.

class ControlExecWidget {
public:
    ControlExecWidget(VAPoR::ControlExecutive *ce);

protected:
    VAPoR::ControlExecutive *_controlExec;
};
