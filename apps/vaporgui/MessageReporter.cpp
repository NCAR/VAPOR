//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MessageReporter.cpp
//
//	Author:		Alan Norton (alan@ucar.edu)
//			National Center for Atmospheric Research
//			1850 Table Mesa Drive
//			PO 3000, Boulder, Colorado
//
//	Date:		April 2005
//
//	Description:	Implements the MessageReporter class
//

#include <vapor/glutil.h> // Must be included first
#define ALLOC_SIZE 4096
#ifdef WIN32
#define VSNPRINTF _vsnprintf
#include "windows.h"
#include "Winnetwk.h"
#else
#define VSNPRINTF vsnprintf
#include <unistd.h>
#endif
#include <sys/types.h>
#include <vector>
#include <string>
#include <qstring.h>
#include <qmessagebox.h>
#include <qmutex.h>
#include <qapplication.h>
#include <QPushButton>
#include <vapor/ParamsMgr.h>
#include "MainForm.h"
#include "TabManager.h"
#include "vapor/AppSettingsParams.h"
#include "MessageReporter.h"

#include <QEvent>
#ifdef WIN32
#pragma warning(disable : 4996)
#endif

using namespace VAPoR;

char *MessageReporter::messageString = 0;
int MessageReporter::messageSize = 0;
MessageReporter *MessageReporter::theReporter = 0;
std::vector<std::string> MessageReporter::savedErrMsgs;
QMutex MessageReporter::messageListMutex;
std::vector<std::string> MessageReporter::silencedMessages;
ParamsMgr *MessageReporter::_paramsMgr = 0;
bool MessageReporter::_fullSilenced = false;

FILE *MessageReporter::m_currentLogfile = NULL;
string MessageReporter::m_currentLogfilePath = "";

MessageReporter::MessageReporter() {

    m_currentLogfile = NULL;
    m_currentLogfilePath = "";

#ifdef DEAD
    MyBase::SetDiagMsgFilePtr(stderr);
#endif

    MyBase::SetErrMsgFilePtr(stderr);
#ifdef DEAD
    MyBase::SetErrMsgCB(addErrorMessageCBFcn);

    AppSettingsParams *aParams = (AppSettingsParams *)
                                     _paramsMgr->GetParams(AppSettingsParams::GetClassType());

    if (!aParams->GetCurrentLogfileEnabled())
        return;

    openLogfile(aParams->GetCurrentLogFileName());

    MyBase::SetDiagMsgCB(addDiagMessageCBFcn);
#endif
}

MessageReporter::~MessageReporter() {
    openLogfile("");

    if (messageString)
        delete messageString;
}

void MessageReporter::fatalMsg(const char *format, ...) {
    getInstance();
    va_list args;
    va_start(args, format);

    char *message = convertText(format, args);
    va_end(args);
    postMessage(Fatal, message);
}
void MessageReporter::errorMsg(const char *format, ...) {
    getInstance();
    va_list args;
    va_start(args, format);

    char *message = convertText(format, args);
    va_end(args);
    postMessage(Error, message);
}
void MessageReporter::warningMsg(const char *format, ...) {
    getInstance();
    va_list args;
    va_start(args, format);
    char *message = convertText(format, args);
    va_end(args);
    postMessage(Warning, message);
}
void MessageReporter::infoMsg(const char *format, ...) {

    getInstance();
    va_list args;
    va_start(args, format);

    char *message = convertText(format, args);
    va_end(args);
    postMessage(Info, message);
}

void MessageReporter::
    postMsg(messagePriority t, const char *message) {
    if (_fullSilenced)
        return;
    assert(t >= 0 && t <= 3);

    writeLog(t, message);

    if (t == Fatal) {
        doPopup(t, message);
    } else {
        if (!isSilenced(string(message))) {
            doPopup(t, message);
        }
        MyBase::SetErrCode(0);
    }
}
void MessageReporter::
    writeLog(messagePriority t, const char *message) {
    AppSettingsParams *aParams = (AppSettingsParams *)
                                     _paramsMgr->GetParams(AppSettingsParams::GetClassType());

    if (!aParams->GetCurrentLogfileEnabled())
        return;
    std::string msg;
    switch (t) {
    case Fatal:
        msg = "FATAL: ";
        break;
    case Error:
        msg = "ERROR: ";
        break;
    case Warning:
        msg = "WARN: ";
        break;
    case Info:
        msg = "INFO: ";
        break;
    default:
        assert(0);
    }
    msg += string(message);
    writeToLog(msg);
}

bool MessageReporter::
    doPopup(messagePriority t, const char *message) {
    QString title;
    QMessageBox::Icon msgIcon = QMessageBox::Information;
    static int count = 0; //Use to slightly jitter the popups

    switch (t) {
    case Fatal:
        title = "VAPOR Fatal Error";
        msgIcon = QMessageBox::Critical;
        break;
    case Error:
        title = "VAPOR Error";
        msgIcon = QMessageBox::Critical;
        break;
    case Warning:
        title = "VAPOR Warning";
        msgIcon = QMessageBox::Warning;
        break;
    default:
        assert(0);
    }

    QMessageBox *msgBox = new QMessageBox(msgIcon, title, message, QMessageBox::Ok, MainForm::getInstance());

    QPushButton *silenceButton = msgBox->addButton("Silence This Message", QMessageBox::AcceptRole);
    silenceButton->setToolTip("Click to prevent the message(s) from being displayed again.\nMessages can be un-silenced from the Application Settings.\n");
    msgBox->adjustSize();

#ifdef DEAD
    QPoint tabPsn = MainForm::getTabManager()->mapToGlobal(QPoint(0, 0));
    tabPsn.setY(tabPsn.y() + (5 * count++) % 20);

    //pnt is the absolute position of the tab manager?
    msgBox->move(tabPsn);
#endif
    msgBox->exec();
    bool silenceIt = (msgBox->clickedButton() == silenceButton);

    delete msgBox;

    return silenceIt;
}

char *MessageReporter::
    convertText(const char *format, va_list args) {

    if (!messageString) {
        messageString = new char[ALLOC_SIZE];
        messageSize = ALLOC_SIZE;
    }
#ifdef Darwin
    VSNPRINTF(messageString, messageSize, format, args);
    return messageString;
#else
    bool done = false;
    while (!done) {
        int rc = VSNPRINTF(messageString, messageSize, format, args);
        if (rc < (messageSize - 1)) {
            done = true;
        } else {
            if (messageString)
                delete[] messageString;
            messageString = new char[messageSize + ALLOC_SIZE];
            assert(messageString != NULL);
            messageSize += ALLOC_SIZE;
        }
    }
    return messageString;
#endif
}

//This is
//called by customEvent when messages are to be posted.
//It must compare each message with the silenced warnings or errors.  If no match,
//then they are displayed in a popup

void MessageReporter::postErrorMessages(vector<std::string> &messageList) {
    if (_fullSilenced)
        return;
    AppSettingsParams *aParams = (AppSettingsParams *)
                                     _paramsMgr->GetParams(AppSettingsParams::GetClassType());

    assert(aParams);
    if (aParams->GetCurrentMessageSilence())
        return;

    std::string fullMessage;
    for (int i = 0; i < messageList.size(); i++) {
        writeLog(Error, messageList[i].c_str());
        if (isSilenced(messageList[i]))
            continue;
        fullMessage += messageList[i];
    }
    if (fullMessage.length() > 0) {
        bool doSilence = doPopup(Error, fullMessage.c_str());
        if (doSilence) { //Silence all of the messages in the messageList.
            for (int i = 0; i < messageList.size(); i++) {
                if (!isSilenced(messageList[i]))
                    silencedMessages.push_back(messageList[i]);
            }
        }
#ifdef DEAD
        MainForm::getTabManager()->GetFrontEventRouter()->updateUrgentTabState();
#endif
    }
}

void MessageReporter::addErrorMessageCBFcn(const char *message, int errcode) {

#ifdef DEAD
    if (!getMessageLock())
        assert(0);
    savedErrMsgs.push_back(std::string(message));
    if (savedErrMsgs.size() == 1) {
        QEvent *postMessageEvent = new QEvent((QEvent::Type)65432);
        QApplication::postEvent(MessageReporter::getInstance(), postMessageEvent);
    }
    releaseMessageLock();
    MainForm::getTabManager()->GetFrontEventRouter()->updateUrgentTabState();
#endif
}
void MessageReporter::addDiagMessageCBFcn(const char *message) {
    if (_fullSilenced)
        return;
    writeLog(Info, message);
}

void MessageReporter::customEvent(QEvent *e) {
    if (e->type() != 65432) {
        assert(0);
        return;
    }
    if (!getMessageLock())
        assert(0);
    if (savedErrMsgs.size() == 0) {
        releaseMessageLock();
        return;
    }

    //Copy all the saved messages.  We can't post them
    //until the lock is released, to avoid deadlock.
    std::vector<std::string> tempMsgs(savedErrMsgs);

    savedErrMsgs.clear();

    releaseMessageLock();
    //Make sure each message ends with \n
    for (int i = 0; i < tempMsgs.size(); i++) {
        string thisMsg = tempMsgs[i];
        if (thisMsg.size() > 0) {
            char &lastChar = thisMsg.at(thisMsg.size() - 1);
            if (lastChar != '\n') {
                thisMsg.append("\n");
                tempMsgs[i] = thisMsg;
            }
        }
    }
    postErrorMessages(tempMsgs);
}
void MessageReporter::postCurrentMsgs() {

    if (!getMessageLock())
        assert(0);
    if (savedErrMsgs.size() == 0) {
        releaseMessageLock();
        return;
    }

    //Copy all the saved messages.  We can't post them
    //until the lock is released, to avoid deadlock.
    std::vector<std::string> tempMsgs(savedErrMsgs);

    savedErrMsgs.clear();

    releaseMessageLock();
    postErrorMessages(tempMsgs);
}
bool MessageReporter::getMessageLock() { //Lock on adding to message list

    for (int i = 0; i < 10; i++) {
        if (messageListMutex.tryLock())
            return true;
#ifdef WIN32
        Sleep(100);
#else
        sleep(1);
#endif
    }

    return false;
}
bool MessageReporter::isSilenced(string msg) {
    AppSettingsParams *aParams = (AppSettingsParams *)
                                     _paramsMgr->GetParams(AppSettingsParams::GetClassType());

    if (aParams->GetCurrentMessageSilence())
        return true;
    for (int i = 0; i < silencedMessages.size(); i++) {
        if (silencedMessages[i] == msg)
            return true;
    }
    return false;
}
void MessageReporter::setDiagMsgCB(bool onOff) {
    if (onOff)
        MyBase::SetDiagMsgCB(addDiagMessageCBFcn);
    else
        MyBase::SetDiagMsgCB(0);
}

void MessageReporter::openLogfile(string logpath) {

    // Save name, reopen the log file
    //
    if (m_currentLogfile && m_currentLogfile != stderr) {
        fclose(m_currentLogfile);
        m_currentLogfile = NULL;
        m_currentLogfilePath.clear();
        if (logpath.length() == 0)
            return;
    }

    m_currentLogfilePath.clear();
    m_currentLogfile = NULL;

    //Special case for stderr log file
    bool debug_set = false;
#ifndef WIN32 //On windows, continue to write to logfile, not console with VAPOR_DEBUG
    debug_set = getenv("VAPOR_DEBUG");
#endif

    if (logpath == "stderr" || debug_set) {
        m_currentLogfile = stderr;
        m_currentLogfilePath = "stderr";
        return;
    }

    //Opening a new log file, not using stderr
    FILE *fp = fopen(logpath.c_str(), "w");
    if (!fp) {
        cerr << "vaporgui: Failed to open log file" << endl;
        return;
    }

    //turn off buffering
    int retcode = setvbuf(m_currentLogfile, 0, _IONBF, 2);
    if (retcode) {
        cerr << "vaporgui: Unable to turn off log file buffering" << endl;
        return;
    }

    m_currentLogfilePath = logpath;
    m_currentLogfile = fp;
}

void MessageReporter::writeToLog(string text) {

    if (!m_currentLogfile) {
        return;
    }

    int rc = fprintf(m_currentLogfile, "%s", text.c_str());
    if (rc != text.length()) {
        cerr << "vaporgui: Error writing text to log file" << endl;
        return;
    }
}
