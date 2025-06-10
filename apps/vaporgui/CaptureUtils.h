#pragma once

#include <string>

namespace VAPoR {
    class ControlExec;
}

namespace CaptureUtils {
    bool EnableAnimationCapture(VAPoR::ControlExec *ce, std::string baseFile = "");
    void CaptureSingleImage(VAPoR::ControlExec *ce);
    void EndAnimationCapture(VAPoR::ControlExec *ce);
    std::string GetCaptureFileName(VAPoR::ControlExec *ce);
}
