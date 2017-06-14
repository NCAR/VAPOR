//************************************************************************
//									*
//		     Copyright (C)  2015				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MessageReporter.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2015
//
//	Description:	Defines the MessageReporting class.  This is used for
//	posting various messages that can occur during the operation of the Vapor GUI
//  This supports log messages and popup messages.
//	Messages are either Info (diagnostic), Warning (immediate information to user), Error (cannot perform) or Fatal (app must end).
//  MyBase::SetErrMsg() results in an Error message, MyBase::SetDiagMsg() results in an Info message.
//  MessageReporter::fatalMsg(), errorMsg(), warningMsg(), and infoMsg() can be invoked by the GUI and produce the appropriate
//  type of message.  All types of messages are written to the Logfile (if it is enabled).  Fatal messages always result in a popup.
//  warnings and errors may result in a popup depending on whether or not they have been silenced.
//  Whenever a message (Error, or Warning) is posted it is compared with the silencedMessages list.  If it is found in the list
//	then it is not displayed in a popup.  When a popup is displayed, it will include a button "silence message(s)".  Clicking that button 
//  will cause all the messages in the popup to be added to the silencedMessages list.
//
#ifndef MESSAGEREPORTER_H
#define MESSAGEREPORTER_H

#include <vapor/MyBase.h>
#include <map>
#include <stdarg.h>

#include <string>
#include <qevent.h>
#include <qobject.h>
#include <qmutex.h>
#include <vector>

namespace VAPoR {
	class ParamsMgr;
}

class MessageReporter : public QObject {
	
public:
	MessageReporter();
	~MessageReporter();
	//This is directly called when unloading messages in customEvent
	static void postErrorMessages(std::vector<std::string>& messageList);
	static MessageReporter* getInstance(){
		if (!theReporter){
			theReporter = new MessageReporter();
		}
		return theReporter;
	}
	static void setDiagMsgCB(bool onOff);

	//Enum describes various message priorities
	enum messagePriority {
		Fatal = 3,
		Error = 2,
		Warning = 1,
		Info = 0
	};
	//Following is usual way in which classes can post messages.
	//If the current message has not been posted more than the max times,
	//based on its type,
	//it is posted again (to log and/or popup)
	static void postMessage(messagePriority t, const char* message){
		getInstance()->postMsg(t,message);
	}
	//Alternatively post a message from the App when rc = -1 is found.
	static void postCurrentMsgs();
	//Following is called by MessageReporter in response to an error message save callback.
	//It adds the message to the list
	static void addErrorMessageCBFcn(const char* message, int errcode);
	static void addDiagMessageCBFcn(const char* message);
	
	void customEvent(QEvent*);
	static void fatalMsg(const char* format, ...); 
	static void errorMsg(const char* format, ...); 
	static void warningMsg(const char* format, ...); 
	static void infoMsg(const char* format, ...); 

	static bool isSilenced(std::string msg);
	static void unSilenceAll() { silencedMessages.clear();}
	static void SetParamsMgr(VAPoR::ParamsMgr* pmgr) {_paramsMgr = pmgr;}
	static void SetFullSilence(bool val) {_fullSilenced = val;}
protected:
	static std::vector<std::string> silencedMessages;
	static MessageReporter* theReporter;
	static void postMsg(messagePriority t, const char* message);
	static void writeLog(messagePriority t, const char* message);
	//Return true if the message needs to be silenced.
	static bool doPopup(messagePriority t, const char* message);
	
	//Utility to make string from args
	static char* convertText(const char* format, va_list args);
	

	static char* messageString;
	static int messageSize;
	//Mutex is serialize access to message list
	
	static QMutex messageListMutex;

	static bool getMessageLock();
	static void releaseMessageLock(){
		messageListMutex.unlock();
	}
	//Storage for list of messages posted during rendering
	static std::vector<std::string> savedErrMsgs;
	static VAPoR::ParamsMgr* _paramsMgr;
	static bool _fullSilenced;

private: 
 static FILE* m_currentLogfile;
 static string m_currentLogfilePath;


 // (Re-) Open the Log file.  Return -1 on failure.
 // If logpath differs from previous logpath, the previous file is closed.
 // If logpath is string of length 0, the current file is closed, 
 // without opening a new file.
 //
 static void openLogfile(string logpath);

 // Write a string to the current log file.
 // Must already be opened.
 // Multiple lines should have embedded \n
 // String should have a terminating \n.
 //
 static void writeToLog(string text);

};
#endif  //MESSAGEREPORTING_H
