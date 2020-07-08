//
// $Id$
//
#include <string>
#include <iostream>
#include <sstream>
#include <cctype>
#include <sys/stat.h>
#include "vapor/CMakeConfig.h"
#include "vapor/FileUtils.h"
#ifdef  Darwin
#include <CoreFoundation/CFBundle.h>
#include <CoreFoundation/CFString.h>
#include <CoreServices/CoreServices.h>
#endif
#include <vapor/MyBase.h>
#define INCLUDE_DEPRECATED_GET_APP_PATH
#include "vapor/GetAppPath.h"
#ifdef WIN32
#pragma warning(disable : 4996)
#endif
using namespace std;
using namespace Wasp;

#ifdef  Darwin
string get_path_from_bundle(const string &app) {
    string path;
	path.clear();

	string bundlename = app + ".app";

    //
    // Get path to document directory from the application "Bundle";
    //

    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (!mainBundle) return(".");

    CFURLRef url = CFBundleCopyBundleURL(mainBundle);
    if (! url) return(".");

    const int kBufferLength = 1024;
    UInt8 buffer[kBufferLength];
    char componentStr[kBufferLength];

    CFIndex componentLength = CFURLGetBytes(url, buffer, kBufferLength);
    if (componentLength < 0) return("");
    buffer[componentLength] = 0;

    CFRange range;
    CFRange rangeIncludingSeparators;

    range = CFURLGetByteRangeForComponent(
        url, kCFURLComponentPath, &rangeIncludingSeparators
    );
    if (range.location == kCFNotFound) return(".");

    strncpy(componentStr, (const char *)&buffer[range.location], range.length);
    componentStr[range.length] = 0;
	string s = componentStr;

	// Spaces are returned as %20. Quick fix below
	size_t start;
	while ((start = s.find("%20")) != std::string::npos)
		s.replace(start, 3, " ");

    path = s;
    return(path);

}
#endif

bool pathExists(const string path)
{
    struct STAT64 statbuf;
    return STAT64(path.c_str(), &statbuf) >= 0;
}

std::string Wasp::GetAppPath(
	const string &app, 
	const string &resource,
	const vector <string> &paths,
	bool forwardSeparator
) {
	string myResource = resource;

#ifndef Darwin
	if (myResource == "home") {
		myResource.clear();
	}
#endif
	ostringstream oss;

	oss << "GetAppPath(" << app << ", " << myResource;
	for (int i=0; i<paths.size(); i++) {
		oss << ", " << paths[i];
	}
	oss << ")" << endl;
	MyBase::SetDiagMsg("%s", oss.str().c_str());

	string separator, otherSeparator;
	if (!forwardSeparator) {
		separator = "\\";
		otherSeparator = "/";
	}
	else {
		separator = "/";
		otherSeparator = "\\";
	}

	string path;
	path.clear();

	string myapp = app;	// upper case app name
	for (string::iterator itr = myapp.begin(); itr != myapp.end(); itr++) {
		if (islower(*itr)) *itr = toupper(*itr);
	}

	string env(myapp);
#ifdef WIN32
	env.append("3");
#endif
	env.append("_HOME");

	if ( ! (
		(myResource.compare("lib") == 0) || 
		(myResource.compare("bin") == 0) ||
		(myResource.compare("share") == 0) ||
		(myResource.compare("plugins") == 0) ||
		(myResource.compare("home") == 0) ||
		(myResource.compare("") == 0)
	)) {
		MyBase::SetDiagMsg("GetAppPath() return : empty (unknown resources)");
		return("");	// empty path, invalid resource
	}

	char *homestr = getenv(env.c_str());

#ifdef	Darwin
	if (homestr) {
		string s = homestr;
		if (s.find(".app") != string::npos) {
			path.assign(homestr);
			path.append(separator);
			homestr = NULL;
		} 
	}
#endif

    if (homestr) {
		path.assign(homestr);
		if (! myResource.empty()) {
			path.append(separator);
			path.append(myResource);
		}
	}
#ifdef	Darwin
	else {
		if (path.empty()) {
			path = get_path_from_bundle(myapp);
		}
		if (! path.empty()) {
			path.append("Contents/");
			if (
				(myResource.compare("bin")==0) || 
				(myResource.compare("")==0)) {
				path.append("MacOS");
			}
			else if (myResource.compare("share") == 0) {
				path.append("share");
			}
			else if (myResource.compare("lib") == 0) {
				path.append("lib");
			}
			else if (myResource.compare("home") == 0) {
				path.erase(path.size() - 1, 1);
			}
			else {	// must be plugins
				path.append("Plugins");
			}
		}
	}
#endif
    
    if (!pathExists(path)) {
        path = "";
    }

	if (path.empty()) {
		if (myResource.compare("share") == 0) {
			path.append(SOURCE_DIR);
			path.append(separator);
			path.append("share");
		}
	}
	
	if (path.empty()) {
		MyBase::SetDiagMsg("GetAppPath() return : empty (path empty)");
		return(path);
	}
	//We may need to reverse the slashes in vapor_home
	bool foundSep = true;
	while (foundSep){
		std::size_t found = path.find(otherSeparator);
		if (found != std::string::npos){
			path.replace(found,1,separator);
		} else foundSep = false;
	}
	for (int i=0; i<paths.size(); i++) {
		path.append(separator);
		path.append(paths[i]);
	}

	if (!pathExists(path)) {
		MyBase::SetDiagMsg("GetAppPath() return : empty (path does not exist)");
		return("");
	}

	MyBase::SetDiagMsg("GetAppPath() return : %s", path.c_str());
	return(path);	
}
