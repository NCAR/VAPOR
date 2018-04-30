#ifdef WIN32
    #include <Windows.h>
    #include <string>
    #include <sstream>
    #include <locale>
    #include <codecvt>
    #include "windowsUtils.h"

LONG Windows_OpenRegistry(HKEY root, std::string keyName, HKEY &key) { return RegOpenKeyEx(root, TEXT(keyName.c_str()), 0, KEY_ALL_ACCESS, &key); }

LONG Windows_CloseRegistry(HKEY key) { return RegCloseKey(key); }

LONG Windows_GetRegistryString(HKEY hKey, const std::string &strValueName, std::string &strValue, const std::string &strDefaultValue)
{
    strValue = strDefaultValue;
    CHAR  szBuffer[8192];
    DWORD dwBufferSize = sizeof(szBuffer);
    LONG  error;
    error = RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (error == ERROR_SUCCESS) strValue = szBuffer;
    return error;
}

LONG Windows_SetRegistryString(HKEY hKey, const std::string &strValueName, const std::string &strValue)
{
    LONG error;
    error = RegSetValueEx(hKey, strValueName.c_str(), 0, REG_SZ, (LPBYTE)strValue.c_str(), strValue.length());
    if (error == ERROR_SUCCESS) BroadcastSystemMessage(0, 0, WM_SETTINGCHANGE, 0, (LPARAM)TEXT("Environment"));
    return error;
}

std::string Windows_GetErrorString(LONG errorCode)
{
    LPVOID lpMsgBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);

    std::wstringstream wss;
    wss << (LPCTSTR)lpMsgBuf;
    std::wstring ws = wss.str();

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string                                               s = converter.to_bytes(ws);
    LocalFree(lpMsgBuf);

    return s;
}
#endif