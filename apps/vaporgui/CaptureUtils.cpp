#include "CaptureUtils.h"
#include "CitationReminder.h"
#include "ErrorReporter.h"
#include "vapor/ControlExecutive.h"
#include "vapor/AnimationParams.h"
#include "vapor/GUIStateParams.h"
#include "vapor/FileUtils.h"
#include "vapor/STLUtils.h"
#include <QMessageBox>
#include <QFileDialog>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace VAPoR;

namespace CaptureUtils {

static void ShowCitationReminder() {
#ifndef NDEBUG
    return;
#endif
    static bool askForCitation = true;
    if (!askForCitation) return;
    askForCitation = false;
    CitationReminder::Show();
}

std::string GetCaptureFileName(ControlExec *ce) {
    ShowCitationReminder();

    auto *ap = ce->GetParams<AnimationParams>();
    std::string filter = ap->GetValueLong(AnimationParams::CaptureTypeTag, 0) == 0 ? "TIFF (*.tif)" : "PNG (*.png)";
    std::string suffix = ap->GetValueLong(AnimationParams::CaptureTypeTag, 0) == 0 ? "tif" : "png";
    std::string defaultPath = ap->GetValueString(AnimationParams::CaptureFileDirTag, FileUtils::HomeDir());

    // - For this QFileDialog, QFileDialog::DontConfirmOverwrite is needed because we do file overwrite confirmation 
    //   manually after modifying the user's input filename for capturing time series
    // 
    // - QFileDialog::DontUseNativeDialog is needed becuase QFileDialog::DontConfirmOverwrite will not work on MacOS without it
    //
    std::string file = QFileDialog::getSaveFileName(nullptr, "Select Filename", QString::fromStdString(defaultPath), QString::fromStdString(filter), nullptr, QFileDialog::DontConfirmOverwrite | QFileDialog::DontUseNativeDialog).toStdString();
    if (file.empty()) return "";


    // Qt does not always append the filetype to the selection from the QFileDialog, so manually add it if this happens
    if (file.find(suffix) == std::string::npos)
        file += "." + suffix;

    // Add zero padding if we are capturing TimeSeries
    if (ap->GetValueLong(AnimationParams::CaptureModeTag, AnimationParams::SingleImage) == AnimationParams::TimeSeries) {
        std::ostringstream oss;
        oss << FileUtils::RemoveExtension(FileUtils::Basename(file)) << std::setw(5) << std::setfill('0') << "." << FileUtils::Extension(file);
        file = FileUtils::JoinPaths({FileUtils::Dirname(file), oss.str()});
    }

    // Warn user about overwriting file
    if (FileUtils::Exists(file)) {
        QMessageBox::StandardButton confirm = QMessageBox::question(nullptr, "File exists", QString::fromStdString(FileUtils::Basename(file) + " exists. Overwrite?"), QMessageBox::Yes | QMessageBox::No);
        if (confirm == QMessageBox::No) return "";
    }

    ap->SetValueString(AnimationParams::CaptureFileDirTag, "Capture animation file directory", FileUtils::Dirname(file));
    if (ap->GetValueLong(AnimationParams::CaptureModeTag, AnimationParams::SingleImage) == AnimationParams::SingleImage) {
        ap->SetValueString(AnimationParams::CaptureFileNameTag, "Capture animation file name", FileUtils::Basename(file));
        ap->SetValueString(AnimationParams::CaptureFileTimeTag, "Capture file time", STLUtils::GetCurrentDateTimestamp());
    } else {
        ap->SetValueString(AnimationParams::CaptureTimeSeriesFileNameTag, "Capture animation file name", FileUtils::Basename(file));
        ap->SetValueString(AnimationParams::CaptureTimeSeriesTimeTag, "Capture file time", STLUtils::GetCurrentDateTimestamp());
    }

    return file;
}

bool EnableAnimationCapture(ControlExec *ce, std::string baseFile) {
    if (baseFile.empty())
        baseFile = GetCaptureFileName(ce);

    if (baseFile.empty()) return false;

    string vizName = ce->GetParams<GUIStateParams>()->GetActiveVizName();
    return ce->EnableAnimationCapture(vizName, true, baseFile) >= 0;
}

void CaptureSingleImage(ControlExec *ce) {
    std::string filePath = GetCaptureFileName(ce);
    if (filePath.empty()) return;

    string vizName = ce->GetParams<GUIStateParams>()->GetActiveVizName();
    if (ce->EnableImageCapture(filePath, vizName) < 0)
        MSG_ERR("Error capturing image");
}

void EndAnimationCapture(ControlExec *ce) {
    string vizName = ce->GetParams<GUIStateParams>()->GetActiveVizName();
    if (ce->EnableAnimationCapture(vizName, false))
        MSG_WARN("Image Capture Warning; active visualizer not capturing images");
}

}  // namespace CaptureUtils

