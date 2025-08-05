#pragma once
    
#include <QObject>
#include <string>

namespace VAPoR {
    class ControlExec;
}

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
