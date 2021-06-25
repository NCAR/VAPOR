#pragma once

#include <string>
#include <functional>
#include <vector>
#include <QDateTime>

struct Notice {
    std::string url;
    std::string content;
    
    QDate date;
    QDate until;
};

//! Uses the GitHub API to check for any notices which are json files on the main branch in share/notices.
//! There is an example file there which specifies the format and features.

void CheckForGHNotices(std::function<void(const std::vector<Notice> &)> callback);
