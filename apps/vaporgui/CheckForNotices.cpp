#include "CheckForNotices.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDesktopServices>
#include <iostream>
#include <string>
#include <stack>
#include <vapor/Version.h>
#include <vapor/STLUtils.h>

using std::cout;
using std::endl;
using std::function;
using std::string;
using Wasp::Version;


void CheckForGHNotices(std::function<void(const std::vector<Notice> &)> callback)
{
    static QNetworkAccessManager *                          manager = nullptr;
    static std::function<void(const std::vector<Notice> &)> _callback;
    static std::vector<Notice>                              _notices;
    static std::stack<Notice>                               _noticesToGet;
    static bool                                             gettingList = true;
    _callback = callback;


    if (!manager) {
        manager = new QNetworkAccessManager;

        QObject::connect(manager, &QNetworkAccessManager::finished, manager, [&](QNetworkReply *reply) {
            if (reply->error()) {
#ifndef NDEBUG
                cout << reply->errorString().toStdString() << endl;
#endif
                return;
            }

#ifdef TESTING_API
            if (STLUtils::Contains(reply->url().toString().toStdString(), "list.json")) {
#else
            if (STLUtils::Contains(reply->url().toString().toStdString(), "api.github.com")) {
#endif
                _notices.clear();
                _noticesToGet = {};
                QString       content = reply->readAll();
                QJsonDocument json = QJsonDocument::fromJson(content.toUtf8());
                QJsonArray    array = json.array();
                for (const QJsonValue value : array) {
                    QJsonObject file = value.toObject();
                    Notice      notice;
                    notice.url = file["download_url"].toString().toStdString();

                    if (!STLUtils::Contains(notice.url, "__example-notice")) _noticesToGet.push(notice);
                }
            } else {
                QString       content = reply->readAll();
                QJsonDocument json = QJsonDocument::fromJson(content.toUtf8());

#define TIME_FORMAT "yyyy-MM-dd"

                Notice notice;
                notice.url = reply->url().toString().toStdString();
                notice.content = json["content"].toString().toStdString();
                notice.date = QDate::fromString(json["date"].toString(), TIME_FORMAT);
                notice.until = QDate::fromString(json["until"].toString(), TIME_FORMAT);

                auto now = QDate::currentDate();

                if (notice.date <= now && notice.until >= now) _notices.push_back(notice);
            }

            if (_noticesToGet.empty()) {
                sort(_notices.begin(), _notices.end(), [](const Notice &a, const Notice &b) { return a.date > b.date; });
                _callback(_notices);
            } else {
                QNetworkRequest req;
                req.setUrl(QUrl(QString::fromStdString(_noticesToGet.top().url)));
                _noticesToGet.pop();
                manager->get(req);
            }
        });
    }

    gettingList = true;
    QNetworkRequest req;
#ifdef TESTING_API
    req.setUrl(QUrl("http://localhost:8000/list.json"));
#else
    req.setUrl(QUrl("https://api.github.com/repos/NCAR/VAPOR/contents/share/notices"));
#endif
    manager->get(req);
}
