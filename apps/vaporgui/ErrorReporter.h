//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		ErrorReporter.h
//
//	Author:		Stas Jaroszynski
//				National Center for Atmospheric Research
//				PO 3000, Boulder, Colorado
//
//	Date:		July 2017
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

class ErrorReporter {

  public:
    enum Type { Diagnostic = 0,
                Info = 1,
                Warning = 2,
                Error = 3 };

    struct Message {
        Type type;
        std::string value;
        int err_code;

        Message(Type type_, std::string value_, int err_code_ = 0)
            : type(type_), value(value_), err_code(err_code_) {}
    };

    static ErrorReporter *getInstance();
    static void showErrors();
    static void report(std::string msg, Type severity = Diagnostic, std::string details = "");
    static std::string getSystemInformation();

  protected:
    ErrorReporter();
    ~ErrorReporter();

  private:
    static ErrorReporter *instance;
    std::vector<Message> log;
    std::vector<Message> fullLog;
    FILE *logFile;

    friend void MyBaseErrorCallback(const char *msg, int err_code);
    friend void MyBaseDiagCallback(const char *msg);
};

#endif
