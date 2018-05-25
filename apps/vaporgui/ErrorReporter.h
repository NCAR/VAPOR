//************************************************************************
//                                                                       *
//                        Copyright (C)  2017                            *
//           University Corporation for Atmospheric Research             *
//                        All Rights Reserved                            *
//                                                                       *
//************************************************************************
//
//	File:			ErrorReporter.h
//
//	Author:			Stas Jaroszynski (stasj@ucar.edu)
//					National Center for Atmospheric Research
//					PO 3000, Boulder, Colorado
//
//	Date:			July 2017
//
//	Description:	Defines the ErrorReporting class.  This is used for
//	posting various messages that can occur during the operation of the Vapor GUI
//  This is a singleton class that registers callbacks with the Vapor error api
//  and keeps tracks of errors that occur. The GUI can then invoke an error message
//  which will display the GUI provided message and provide a details area which
//  contains the full error log accumilated since the last error message  which can
//  be saved to a text file.
//  This class also registeres a signal handler for SIGSEGV and displays an error
//  window with the current backtrace.

#ifndef ERRORREPORTER_H
#define ERRORREPORTER_H

#include <string>
#include <vector>
#include <cassert>
#include <QMessageBox>

//! \class ErrorReporterPopup
//! \ingroup Public_GUI
//! \brief A helper class for ErrorReporter that is neccessary because the Qt gui is in a separate thread
//! \author Stas Jaroszynski
//! \version 1.0
//! \date May 2018

class ErrorReporterPopup : public QMessageBox {
    Q_OBJECT;

public:
    ErrorReporterPopup(QWidget *parent, int id);
    void setLogText(std::string text);
    bool isDead() const { return dead; };

private slots:
    void doAction(QAbstractButton *button);

private:
    bool        dead;
    std::string _logText;
};

#define ERRORREPORTER_DEFAULT_MESSAGE "The action failed"

//! \class ErrorReporter
//! \ingroup Public_GUI
//! \brief A utility singleton class that provides error reporting functinality
//! \author Stas Jaroszynski
//! \version 3.0
//! \date July 2017

//! ErrorReporter class provides error reporting functionality. Registers error
//! callbacks with MyBase and registers signal handler for SIGSEGV

#define MSG_ERR(M) (ErrorReporter::GetInstance()->Report(M, ErrorReporter::Error))

#define MSG_WARN(M) (ErrorReporter::GetInstance()->Report(M, ErrorReporter::Warning))

#define MSG_DIAG(M) (ErrorReporter::GetInstance()->Report(M, ErrorReporter::Diagnostic))

class ErrorReporter {
public:
    ErrorReporter(QWidget *parent);
    enum Type { Diagnostic = 0, Info = 1, Warning = 2, Error = 3 };

    struct Message {
        Type        type;
        std::string value;
        int         err_code;

        Message(Type type_, std::string value_, int err_code_ = 0) : type(type_), value(value_), err_code(err_code_) {}
    };

    //! Returns the singleton instance of this class with lazy initialization
    //! \retval ErrorReporter instance
    static ErrorReporter *GetInstance() { return (_instance); };

    //! Displays the current log of errors with the default message ERRORREPORTER_DEFAULT_MESSAGE
    static void ShowErrors();

    //! Displays an error message with the log of errors and outputs the message to the log file
    //! \param string msg to display explaining error cause/implications
    //! \param Type severity of message
    //! \param string details of error. Default to current erros in log
    static void Report(std::string msg, Type severity = Diagnostic, std::string details = "");

    //! Returns basic system OS information
    //! \retval string containing OS information
    static std::string GetSystemInformation();

    //! Opens log file and begins logging error and diagnostic messages
    //! \retval int returns -1 on failure
    static int OpenLogFile(std::string path);

protected:
    ~ErrorReporter();

private:
    ErrorReporter();
    static ErrorReporter *            _instance;
    std::vector<Message>              _log;
    std::vector<Message>              _fullLog;
    std::string                       _logFilePath;
    FILE *                            _logFile;
    QWidget *                         _parent;
    std::vector<ErrorReporterPopup *> _boxes;

    friend void _myBaseErrorCallback(const char *msg, int err_code);
    friend void _myBaseDiagCallback(const char *msg);
};

#endif
