#pragma once

#include <string>

namespace VAPoR {
    class ControlExec;
}

namespace CaptureUtils {
    std::string GetCaptureFileName(VAPoR::ControlExec *ce);
    bool EnableAnimationCapture(VAPoR::ControlExec *ce, std::string baseFile = "");
    void CaptureSingleImage(VAPoR::ControlExec *ce);
    void EndAnimationCapture(VAPoR::ControlExec *ce);
}
