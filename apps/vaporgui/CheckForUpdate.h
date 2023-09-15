#pragma once

#include <string>
#include <functional>

class QNetworkAccessManager;

struct UpdateInfo {
    std::string version;
    std::string url;
    bool        error = false;

    void OpenURL();
};

//! Uses the GitHub API to check Vapor's latest release version and compares it against the current version. If an update
//! is available, info will be populated.

void CheckForUpdate(std::function<void(bool updateAvailable, UpdateInfo info)> callback);
