#pragma once

#include <string>
#include <functional>

class QNetworkAccessManager;

struct UpdateInfo {
    std::string version;
    std::string url;
    bool error = false;
    
    void OpenURL();
};

void CheckForUpdate(std::function<void(bool updateAvailable, UpdateInfo info)> callback);
