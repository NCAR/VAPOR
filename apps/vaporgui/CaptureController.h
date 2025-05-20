#pragma once

#include <QObject>

class AnimationParams;

namespace VAPoR {
    class ControlExec;
    class ParamsMgr;
}

//! \class CaptureController
//! \brief Decouples image capture logic that was previously in MainForm so it can be
//! performed outside of the MainForm

class CaptureController : public QObject {
    Q_OBJECT
    VAPoR::ControlExec *_ce;

    void _showCitationReminder();
    std::string _getFileName();

    bool _askForCitation = true;

public:
    CaptureController(VAPoR::ControlExec *ce);
    void CaptureSingleImage();
    bool EnableAnimationCapture(std::string baseFile = "");
    void StartAnimationCapture() const;
    void EndAnimationCapture() const;

signals:
    void animationCaptureStarted() const;
};
