
#include "ErrorReporter.h"

#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "vapor/MyBase.h"
#include "vapor/Version.h"

#if defined(DARWIN)
    #include <CoreServices/CoreServices.h>
#elif defined(LINUX)
#elif defined(WIN32)
#endif

using std::string;
using std::vector;

void segFaultHandler(int sig)
{
    void * array[128];
    size_t size;
    size = backtrace(array, 128);

    backtrace_symbols_fd(array, size, STDERR_FILENO);
    char **backtrace_str = backtrace_symbols(array, 128);

    string details;
    for (int i = 0; i < size; i++) {
        if (strlen(backtrace_str[i]) == 0) break;
        details += string(backtrace_str[i]) + "\n";
    }

    ErrorReporter::report("A memory error occured", ErrorReporter::Error, details);

    exit(1);
}

void MyBaseErrorCallback(const char *msg, int err_code) { ErrorReporter::getInstance()->log.push_back(ErrorReporter::Message(ErrorReporter::Error, string(msg), err_code)); }

void MyBaseDiagCallback(const char *msg) { ErrorReporter::getInstance()->log.push_back(ErrorReporter::Message(ErrorReporter::Diagnostic, string(msg))); }

ErrorReporter *ErrorReporter::instance;
ErrorReporter *ErrorReporter::getInstance()
{
    if (!instance) instance = new ErrorReporter();
    return instance;
}

void ErrorReporter::showErrors() { report("The action failed"); }

void ErrorReporter::report(string msg, Type severity, string details)
{
    ErrorReporter *e = getInstance();

    QMessageBox box;
    box.setText("An error has occured");
    box.setInformativeText(msg.c_str());
    box.addButton(QMessageBox::Ok);
    box.addButton(QMessageBox::Save);

    if (details == "") {
        while (e->log.size()) {
            details += e->log.back().value + "\n";
            if (e->log.back().type > severity) severity = e->log.back().type;
            e->log.pop_back();
        }
    }
    box.setDetailedText(details.c_str());

    switch (severity) {
    case Diagnostic:
    case Info: box.setIcon(QMessageBox::Information); break;
    case Warning: box.setIcon(QMessageBox::Warning); break;
    case Error: box.setIcon(QMessageBox::Critical); break;
    }

    int ret = box.exec();

    switch (ret) {
    case QMessageBox::Save: {
        QString fileName = QFileDialog::getSaveFileName(NULL, "Save Error Log", QString(), "Text (*.txt);;All Files (*)");
        if (fileName.isEmpty())
            return;
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(NULL, "Unable to open file", file.errorString());
                return;
            }
            QDataStream out(&file);
            out << QString((getSystemInformation() + "\n").c_str());
            out << QString("-------------------\n");
            out << QString((msg + "\n").c_str());
            out << QString("-------------------\n");
            out << QString(details.c_str());
        }
        break;
    }
    case QMessageBox::Ok: break;
    default: break;
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
string                 ErrorReporter::getSystemInformation()
{
    string ret = "Vapor " + Wasp::Version::GetVersionString() + "\n";
#if defined(DARWIN)
    SInt32 major, minor, rev;
    Gestalt(gestaltSystemVersionMajor, &major);
    Gestalt(gestaltSystemVersionMinor, &minor);
    Gestalt(gestaltSystemVersionBugFix, &rev);
    ret += "OS: Mac OS X " + to_string(major) + "." + to_string(minor) + "." + to_string(rev) + "\n";
#elif defined(LINUX)
    return "Linux";
#elif defined(WIN32)
    return "Windows";
#else
    return "Unsupported Platform";
#endif
    return ret;
}
#pragma GCC diagnostic pop

ErrorReporter::ErrorReporter()
{
    signal(SIGSEGV, segFaultHandler);
    Wasp::MyBase::SetErrMsgCB(MyBaseErrorCallback);
    Wasp::MyBase::SetDiagMsgCB(MyBaseDiagCallback);
}
