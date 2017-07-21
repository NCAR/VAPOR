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

  protected:
    ErrorReporter();

  private:
    static ErrorReporter *instance;
    std::vector<Message> log;

    friend void MyBaseErrorCallback(const char *msg, int err_code);
    friend void MyBaseDiagCallback(const char *msg);
};

#endif
