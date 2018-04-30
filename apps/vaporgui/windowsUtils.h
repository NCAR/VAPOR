#ifndef WINDOWS_UTILS_H
#define WINDOWS_UTILS_H
#ifdef WIN32

    #include <Windows.h>

    #define WINDOWS_SUCCESS             ERROR_SUCCESS
    #define WINDOWS_HKEY_CLASSES_ROOT   HKEY_CLASSES_ROOT
    #define WINDOWS_HKEY_CURRENT_CONFIG HKEY_CURRENT_CONFIG
    #define WINDOWS_HKEY_CURRENT_USER   HKEY_CURRENT_USER
    #define WINDOWS_HKEY_LOCAL_MACHINE  HKEY_LOCAL_MACHINE
    #define WINDOWS_HKEY_USERS          HKEY_USERS

LONG        Windows_OpenRegistry(HKEY root, std::string keyName, HKEY &key);
LONG        Windows_CloseRegistry(HKEY key);
LONG        Windows_GetRegistryString(HKEY hKey, const std::string &strValueName, std::string &strValue, const std::string &strDefaultValue);
LONG        Windows_SetRegistryString(HKEY hKey, const std::string &strValueName, const std::string &strValue);
std::string Windows_GetErrorString(LONG errorCode);

#endif
#endif