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
    //void captureEnded();
    //void imageCaptured();
    //void errorOccurred(const std::string &message);
};

//namespace CaptureUtils {
//    std::string GetCaptureFileName(VAPoR::ControlExec *ce);
//    bool EnableAnimationCapture(VAPoR::ControlExec *ce, std::string baseFile = "");
//    void CaptureSingleImage(VAPoR::ControlExec *ce);
//    void EndAnimationCapture(VAPoR::ControlExec *ce);
//}
