#include "CLIToolInstaller.h"
#include <QMessageBox>
#include <common.h>
#include <vapor/ResourcePath.h>

#ifdef WIN32
    #include <windows.h>
#endif

using namespace std;
using namespace Wasp;

void CLIToolInstaller::Install()
{
    QMessageBox box;
    box.addButton(QMessageBox::Ok);

#ifdef Darwin
    string home = GetResourcePath("");
    string binPath = home + "/MacOS";
    home.erase(home.size() - strlen("Contents/"), strlen("Contents/"));

    vector<string> profilePaths = {
        string(getenv("HOME")) + "/.profile",
        string(getenv("HOME")) + "/.zshrc",
    };
    bool success = true;

    for (auto profilePath : profilePaths) {
        FILE *prof = fopen(profilePath.c_str(), "a");
        success &= (bool)prof;
        if (prof) {
            fprintf(prof, "\n\n");
            fprintf(prof, "export VAPOR_HOME=\"%s\"\n", home.c_str());
            fprintf(prof, "export PATH=\"%s:$PATH\"\n", binPath.c_str());
            fclose(prof);
        }
    }

    if (success) {
        box.setText("Installation Successful");
        box.setInformativeText("Environmental variables set in ~/.profile and ~/.zshrc. The vapor command line utilities should be available from the terminal.");
        box.setIcon(QMessageBox::Information);
    } else {
        box.setText("Unable to set environmental variables");
        box.setIcon(QMessageBox::Critical);
    }
#endif

#ifdef WIN32
    HKEY   key;
    long   error;
    long   errorClose;
    bool   pathWasModified = false;
    string home = GetResourcePath("");

    error = Windows_OpenRegistry(WINDOWS_HKEY_CURRENT_USER, "Environment", key);
    if (error == WINDOWS_SUCCESS) {
        string path;
        error = Windows_GetRegistryString(key, "Path", path, "");
        if (error == WINDOWS_ERROR_FILE_NOT_FOUND) {
            error = WINDOWS_SUCCESS;
            path = "";
        }
        if (error == WINDOWS_SUCCESS) {
            bool   alreadyExists = false;
            size_t index;
            if (path.find(";" + home + ";") != std::string::npos)
                alreadyExists = true;
            else if ((index = path.find(";" + home)) != std::string::npos && index + home.length() + 1 == path.length())
                alreadyExists = true;
            else if ((index = path.find(home + ";")) != std::string::npos && index == 0)
                alreadyExists = true;
            else if (path == home)
                alreadyExists = true;

            if (!alreadyExists) {
                if (path.length() > 0) path += ";";
                path += home;
                error = Windows_SetRegistryString(key, "Path", path);
                if (error == WINDOWS_SUCCESS) pathWasModified = true;
            }
        }
        errorClose = Windows_CloseRegistry(key);
    }

    if (error == WINDOWS_SUCCESS && errorClose == WINDOWS_SUCCESS) {
        // This tells windows to re-load the environment variables
        SendMessage(HWND_BROADCAST, WM_WININICHANGE, NULL, (LPARAM) "Environment");

        box.setIcon(QMessageBox::Information);
        if (pathWasModified)
            box.setText("Vapor conversion utilities were added to your path");
        else
            box.setText("Your path is properly configured");
    } else {
        box.setIcon(QMessageBox::Critical);
        box.setText("Unable to set environmental variables");
        string errString = "";
        if (error != WINDOWS_SUCCESS) errString += Windows_GetErrorString(error) + "\n";
        if (errorClose != WINDOWS_SUCCESS) errString += "CloseRegistry: " + Windows_GetErrorString(errorClose);
        box.setInformativeText(QString::fromStdString(errString));
    }
#endif

    box.exec();
}
