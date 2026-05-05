#pragma once
    
#include <QObject>
#include <string>

namespace VAPoR {
    class ControlExec;
}

struct TiffStrings {
    static const std::string FileFilter;
    static const std::string FileSuffix;
};
struct PngStrings {
    static const std::string FileFilter;
    static const std::string FileSuffix;
};

class CaptureController : public QObject {
    Q_OBJECT

    VAPoR::ControlExec *_ce;

public:
    explicit CaptureController(VAPoR::ControlExec *ce);

    std::string GetCaptureFileName();
    bool EnableAnimationCapture(std::string baseFile = "");
    void CaptureSingleImage();
    void EndAnimationCapture();
    void ShowCitationReminder();

signals:
    void captureStarted();
};
