//************************************************************************
//                                                                       *
//                        Copyright (C)  2017                            *
//           University Corporation for Atmospheric Research             *
//                        All Rights Reserved                            *
//                                                                       *
//************************************************************************
//
//	File:			ErrorReporter.cpp
//
//	Author:			Stas Jaroszynski (stasj@ucar.edu)
//					National Center for Atmospheric Research
//					1850 Table Mesa Drive
//					PO 3000, Boulder, Colorado
//
//	Date:			July 2017
//
//	Description:	Implements the ErrorReporter class

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

#if !defined(WIN32)
#include <execinfo.h>
#include <unistd.h>
#endif

#if defined(DARWIN)
#include <CoreServices/CoreServices.h>
#elif defined(LINUX)
#include <sys/utsname.h>
#elif defined(WIN32)
#include <windows.h>
#endif

#include <QWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QPushButton>

#include "vapor/MyBase.h"
#include "vapor/Version.h"

#include "ErrorReporter.h"

using std::string;
using std::vector;

void _segFaultHandler(int sig) {
    string details;
#if !defined(WIN32)
    void *array[128];
    size_t size;
    size = backtrace(array, 128);

    backtrace_symbols_fd(array, size, STDERR_FILENO);
    char **backtrace_str = backtrace_symbols(array, 128);

    for (int i = 0; i < size; i++) {
        if (strlen(backtrace_str[i]) == 0)
            break;
        details += string(backtrace_str[i]) + "\n";
    }
#endif

    ErrorReporter::Report("A memory error occured", ErrorReporter::Error, details);
    exit(1);
}

void _myBaseErrorCallback(const char *msg, int err_code) {
    ErrorReporter *e = ErrorReporter::GetInstance();
    e->_log.push_back(ErrorReporter::Message(ErrorReporter::Error, string(msg), err_code));
    e->_fullLog.push_back(ErrorReporter::Message(ErrorReporter::Error, string(msg), err_code));

    if (e->_logFile) {
        fprintf(e->_logFile, "Error[%i]: %s\n", err_code, msg);
    }
}

void _myBaseDiagCallback(const char *msg) {
    ErrorReporter *e = ErrorReporter::GetInstance();
    e->_fullLog.push_back(ErrorReporter::Message(ErrorReporter::Diagnostic, string(msg)));

    if (e->_logFile) {
        fprintf(e->_logFile, "Diagnostic: %s\n", msg);
    }
}

ErrorReporter *ErrorReporter::_instance;
ErrorReporter *ErrorReporter::GetInstance() {
    if (!_instance) {
        _instance = new ErrorReporter();
    }
    return _instance;
}

void ErrorReporter::ShowErrors() {
    Report(ERRORREPORTER_DEFAULT_MESSAGE);
}

void ErrorReporter::Report(string msg, Type severity, string details) {
    ErrorReporter *e = GetInstance();
    if (e->_logFile) {
        fprintf(e->_logFile, "Report[%i]: %s\n%s\n", (int)severity, msg.c_str(), details.c_str());
    }

    QMessageBox box;
    box.setText("An error has occured");
    box.setInformativeText(msg.c_str());
    //box.addButton(QMessageBox::Ok);
    //box.addButton(QMessageBox::Save);
    box.addButton("Ok", QMessageBox::AcceptRole);
    box.addButton("Save Log", QMessageBox::ApplyRole);

    if (details == "") {
        while (e->_log.size()) {
            details += e->_log.back().value + "\n";
            if (e->_log.back().type > severity)
                severity = e->_log.back().type;
            e->_log.pop_back();
        }
    }
    box.setDetailedText(details.c_str());

    switch (severity) {
    case Diagnostic:
    case Info:
        box.setIcon(QMessageBox::Information);
        break;
    case Warning:
        box.setIcon(QMessageBox::Warning);
        break;
    case Error:
        box.setIcon(QMessageBox::Critical);
        break;
    }

    int ret = box.exec();
    QAbstractButton *clicked = box.clickedButton();
    QMessageBox::ButtonRole role = box.buttonRole(clicked);

    switch (role) {
    case QMessageBox::ApplyRole: {
        QString fileName = QFileDialog::getSaveFileName(NULL, "Save Error Log", QString(), "Text (*.txt);;All Files (*)");
        if (fileName.isEmpty())
            return;
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::information(NULL, "Unable to open file", file.errorString());
                return;
            }
            QTextStream out(&file);
            out << QString((GetSystemInformation() + "\n").c_str());
            out << QString("-------------------\n");
            out << QString((msg + "\n").c_str());
            out << QString("-------------------\n");
            out << QString(details.c_str());
        }
        break;
    }
    case QMessageBox::AcceptRole:
        break;
    default:
        printf("Uknown Messagebox role %i\n", role);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
string ErrorReporter::GetSystemInformation() {
    string ret = "Vapor " + Wasp::Version::GetVersionString() + "\n";
#if defined(DARWIN)
    SInt32 major, minor, rev;
    Gestalt(gestaltSystemVersionMajor, &major);
    Gestalt(gestaltSystemVersionMinor, &minor);
    Gestalt(gestaltSystemVersionBugFix, &rev);
    ret += "OS: Mac OS X " + to_string(major) + "." + to_string(minor) + "." + to_string(rev) + "\n";
#elif defined(LINUX)
    // struct utsname info;
    // uname(&info);
    // ret += "OS: " + string(info.sysname) + " " + string(info.release) + " " + string(info.version) "\n";
    // ret += "Distro:\n";
    // char buffer[128];
    // std::shared_ptr<FILE> pipe(popen("lsb_release", "r"), pclose);
    // if (pipe) {
    // 	while (!feof(pipe.get())) {
    // 		if (fgets(buffer, 128, pipe.get()) != std::nullptr)
    // 			ret += buffer;
    // 	}
    // } else  {
    // 	fprintf(stderr, "popen failed\n");
    // }
#elif defined(WIN32)
    // DWORD version = 0;
    // DWORD major = 0;
    // DWORD minor = 0;
    // DWORD build = 0;
    //
    // vrsion = GetVersion();
    //
    // major = (DWORD)(LOBYTE(LOWORD(version)));
    // minor = (DWORD)(HIBYTE(LOWORD(version)));
    //
    // if (version < 0x80000000) {
    //     build = (DWORD)(HIWORD(version));
    // }

    // ret += "OS: Windows " + to_string(major) + "." + to_string(minor) + " (" + to_string(build) + ")\n";
#else
    return "Unsupported Platform";
#endif
    return ret;
}
#pragma GCC diagnostic pop

int ErrorReporter::OpenLogFile(std::string path) {
    ErrorReporter *e = ErrorReporter::GetInstance();
    e->_logFilePath = path;
    e->_logFile = fopen(path.c_str(), "w");

    if (!e->_logFile) {
        Wasp::MyBase::SetErrMsg(errno, "Failed to open log file \"%s\"", path.c_str());
        return -1;
    }
    return 0;
}

ErrorReporter::ErrorReporter() {
    signal(SIGSEGV, _segFaultHandler);
    Wasp::MyBase::SetErrMsgCB(_myBaseErrorCallback);
    Wasp::MyBase::SetDiagMsgCB(_myBaseDiagCallback);

    _logFilePath = "";
    _logFile = NULL;
}

ErrorReporter::~ErrorReporter() {
    if (_logFile) {
        fclose(_logFile);
    }
}
