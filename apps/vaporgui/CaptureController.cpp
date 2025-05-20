#include "CaptureController.h"
#include "CitationReminder.h"
#include "ErrorReporter.h"

#include "vapor/ControlExecutive.h"
#include "vapor/AnimationParams.h"
#include "vapor/GUIStateParams.h"
#include "vapor/FileUtils.h"
#include "vapor/STLUtils.h"

#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>

#include <sstream>
#include <iomanip>

CaptureController::CaptureController(VAPoR::ControlExec *ce) : _ce(ce) {
    std::cout << "nullptr? " << (_ce==nullptr) << std::endl;
}

bool CaptureController::EnableAnimationCapture(std::string filePath) {
    if (filePath.empty()) {
        filePath = _getFileName();
        std::cout << "nullptr2? " << (_ce==nullptr) << std::endl;
        if (filePath.empty()) return false;
    }

    //GUIStateParams *p = (GUIStateParams*)_pm->GetParams(GUIStateParams::GetClassType());
    GUIStateParams* gsp = _ce->GetParams<GUIStateParams>();
    string vizName = gsp->GetActiveVizName();
    int rc = _ce->EnableAnimationCapture(vizName, true, filePath);

    if (rc < 0) {
        MSG_ERR("Error capturing image");
        return false;
    }
    return true;
}

void CaptureController::CaptureSingleImage() {
    std::string filePath = _getFileName();
    if (filePath.empty()) return;

    GUIStateParams *gsp = gsp = _ce->GetParams<GUIStateParams>();
    int rc = _ce->EnableImageCapture(filePath, gsp->GetActiveVizName());
    if (rc < 0) MSG_ERR("Error capturing image");
}

// Signal AnimationController to start through connection in MainForm
void CaptureController::StartAnimationCapture() const {
    emit animationCaptureStarted(); 
}

void CaptureController::EndAnimationCapture() const {
    //// Turn off capture mode for the current active visualizer (if it is on!)
    //if (_capturingAnimationVizName.empty()) return;
    //GUIStateParams *p = GetStateParams();
    //GUIStateParams *gsp = (GUIStateParams*)_pm->GetParams(GUIStateParams::GetClassType());
    //string          vizName = gsp->GetActiveVizName();
    //if (vizName != _capturingAnimationVizName) { MSG_WARN("Terminating capture in non-active visualizer"); }
    //if (_controlExec->EnableAnimationCapture(_capturingAnimationVizName, false)) MSG_WARN("Image Capture Warning;\nCurrent active visualizer is not capturing images");
    
    GUIStateParams *gsp = gsp = _ce->GetParams<GUIStateParams>();
    string vizName = gsp->GetActiveVizName();
    if (_ce->EnableAnimationCapture(vizName, false)) MSG_WARN("Image Capture Warning;\nCurrent active visualizer is not capturing images");
}

std::string CaptureController::_getFileName() {
    _showCitationReminder();

    std::string filter, defaultSuffix;
    //AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    std::cout << "nullptr3? " << (_ce==nullptr) << std::endl;
    AnimationParams* ap = _ce->GetParams<AnimationParams>();
    if (ap->GetValueLong(AnimationParams::CaptureTypeTag, 0) == 0) { // .tiff
         filter = "TIFF (*.tif)";
         defaultSuffix =  "tif";
    }
    else {                                                           // .png
         filter = "PNG (*.png)";
         defaultSuffix = "png";
    }

    // DontConfirmOverwrite because we do this manually after modifying the user's input
    // in the case of capturing time series.  DontUseNativeDialog becuase DontConfirmOverwrite will
    // not work on MacOS without it.
    std::string defaultPath = ap->GetValueString(AnimationParams::CaptureFileDirTag, FileUtils::HomeDir());
    std::string fileName = QFileDialog::getSaveFileName(
        nullptr, 
        "Select Filename", 
        QString::fromStdString(defaultPath), 
        QString::fromStdString(filter),
        nullptr,
        (QFileDialog::DontConfirmOverwrite | QFileDialog::DontUseNativeDialog)
        ).toStdString();
    if (fileName.empty()) return "";

    // Qt does not always append the filetype to the selection from the QFileDialog, so manually add it if this happens
    if (fileName.find(defaultSuffix) == std::string::npos) fileName = fileName + "." + defaultSuffix;

    // Add zero padding if we are capturing TimeSeries
    auto captureMode = ap->GetValueLong(AnimationParams::CaptureModeTag, AnimationParams::SingleImage);
    if (captureMode == AnimationParams::TimeSeries) {
        std::ostringstream oss;
        oss << FileUtils::RemoveExtension(FileUtils::Basename(fileName));
        oss << std::setw(5) << std::setfill('0');
        oss << "." << FileUtils::Extension(fileName);
        fileName = FileUtils::JoinPaths({FileUtils::Dirname(fileName), oss.str()});
    }

    // Warn user about overwriting file
    if (FileUtils::Exists(fileName)) {
        QMessageBox::StandardButton overwrite;
        overwrite = QMessageBox::question(
            nullptr,
            "File exists",
            QString::fromStdString(FileUtils::Basename(fileName) + " exists.\nDo you want to overwrite?"),
            QMessageBox::Yes | QMessageBox::No
        );
        if (overwrite == QMessageBox::No) return "";
    }

    ap->SetValueString(AnimationParams::CaptureFileDirTag, "Capture animation file directory", FileUtils::Dirname(fileName));

    if (captureMode == AnimationParams::SingleImage) {
        ap->SetValueString(AnimationParams::CaptureFileNameTag, "Capture animation file name", FileUtils::Basename(fileName));
        ap->SetValueString(AnimationParams::CaptureFileTimeTag, "Capture file time", STLUtils::GetCurrentDateTimestamp());
    }
    else {
        ap->SetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, "Capture animation file name", FileUtils::Basename(fileName));
        ap->SetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "Capture file time", STLUtils::GetCurrentDateTimestamp());
    }

    return fileName;
}

void CaptureController::_showCitationReminder() {
    // Disable citation reminder in Debug build
#ifndef NDEBUG
    return;
#endif
    if (!_askForCitation) return;
    _askForCitation = false;
    CitationReminder::Show();
}

