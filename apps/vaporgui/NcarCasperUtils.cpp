#include "NcarCasperUtils.h"
#include <QMessageBox>
#include <QCheckBox>
#include <QApplication>
#include <vapor/STLUtils.h>
#include <vapor/ControlExecutive.h>
#include <vapor/SettingsParams.h>
#include "ErrorReporter.h"
#ifndef _WIN32
    #include <unistd.h>
#endif

void NcarCasperUtils::CheckForCasperVGL(VAPoR::ControlExec *ce)
{
#ifndef Darwin
#ifndef WIN32
    auto sp = ce->GetParams<SettingsParams>();
    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);
    if (!STLUtils::BeginsWith(hostname, "casper")) return;
    if (getenv("VGL_ISACTIVE")) return;

    const char *message = "In order to utilize Casper's GPU fully, Vapor needs to be launched using vglrun. The button below will relaunch Vapor with vglrun.";
    printf("WARNING: %s\n", message);
    if (!sp->GetCasperCheckForVGL()) return;

    printf("\t Displaying warning popup in GUI\n");
    QMessageBox *popup = new QMessageBox(
        QMessageBox::Icon::Warning,
        "Warning",
        message
    );
    QCheckBox *cb = new QCheckBox("Don't show again");
    cb->setChecked(false);
    popup->addButton("Dismiss", QMessageBox::RejectRole);
    popup->addButton("Relaunch", QMessageBox::AcceptRole);
    popup->setCheckBox(cb);

    auto finish = [sp, cb](int result) {
        if (cb->isChecked()) {
            sp->SetCasperCheckForVGL(false);
            sp->SaveSettings();
        }

        if (result == QMessageBox::Rejected)
            return;

        auto qtArgs = QApplication::instance()->arguments();

        string appPath; // This information is supposed to be in qtArgs but AppImage will overwrite it.
        if (getenv("APPIMAGE"))
            appPath = getenv("APPIMAGE");
        else
            appPath = QApplication::instance()->applicationFilePath().toStdString();

        vector<const char*> prepend = {"vglrun"};
        char ** args = new char*[prepend.size() + qtArgs.size() + 1];
        for (int i = 0; i < prepend.size(); i++)
            args[i] = strdup(prepend[i]);
        args[0+prepend.size()] = strdup(appPath.c_str());
        for (int i = 1; i < qtArgs.size(); i++)
            args[i+prepend.size()] = strdup(qtArgs[i].toStdString().c_str());
        args[prepend.size() + qtArgs.size()] = nullptr;


        // string s = "";
        // for (int i=0; i < prepend.size() + qtArgs.size(); i++)
        //     s += string() + args[i] + " ";
        // printf("COMMAND = %s\n", s.c_str());

        execvp(args[0], args);
        MSG_WARN("Failed to restart vapor using vglrun");
    };

    QObject::connect(popup, &QMessageBox::finished, finish);
    popup->show();
#endif
#endif
}