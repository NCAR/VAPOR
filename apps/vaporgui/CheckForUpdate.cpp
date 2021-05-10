#include "CheckForUpdate.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDesktopServices>
#include <iostream>
#include <string>
#include <vapor/Version.h>

using std::cout;
using std::endl;
using std::function;
using std::string;
using Wasp::Version;

void CheckForUpdate(function<void(bool updateAvailable, UpdateInfo info)> callback)
{
    static QNetworkAccessManager *                               manager = nullptr;
    static function<void(bool updateAvailable, UpdateInfo info)> _callback;
    _callback = callback;

    if (!manager) {
        manager = new QNetworkAccessManager;

        QObject::connect(manager, &QNetworkAccessManager::finished, manager, [&](QNetworkReply *reply) {
            if (reply->error()) {
                cout << reply->errorString().toStdString() << endl;
                UpdateInfo info;
                info.error = true;
                _callback(false, info);
                return;
            }

            string     currentVersion = Version::GetVersionString();
            bool       updateAvailable = false;
            UpdateInfo info;

            QString       content = reply->readAll();
            QJsonDocument json = QJsonDocument::fromJson(content.toUtf8());
            QJsonArray    array = json.array();
            for (const QJsonValue value : array) {
                QJsonObject release = value.toObject();
                bool        preRelease = release["prerelease"].toBool();
                info.version = release["tag_name"].toString().toStdString();
                info.url = release["html_url"].toString().toStdString();
                updateAvailable = !preRelease && Version::Compare(currentVersion, info.version) < 0;

                if (updateAvailable) break;
            }

            _callback(updateAvailable, info);
        });
    }

    QNetworkRequest req;
#ifdef TESTING_API
    req.setUrl(QUrl("http://localhost:8000/api.json"));
#else
    req.setUrl(QUrl("https://api.github.com/repos/NCAR/VAPOR/releases"));
#endif
    manager->get(req);
}

void UpdateInfo::OpenURL() { QDesktopServices::openUrl(QUrl(QString::fromStdString(url))); }
