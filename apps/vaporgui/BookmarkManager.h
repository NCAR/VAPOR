#pragma once
#include "Updatable.h"
#include <common.h>

class QMenu;
class VizWinMgr;

class BookmarkManager : public QObject, public Updatable {
    Q_OBJECT
public:
    BookmarkManager(QObject *parent, ControlExec *ce, VizWinMgr *vizWinMgr);
    void RegisterToMenu(QMenu *menu);
    void Update() override;

private:
    ControlExec * const _controlExec;
    ParamsMgr * const _paramsMgr;
    VizWinMgr * const _vizWinMgr;
    QMenu * const _loadBookmarkMenu;
    QMenu * const _deleteBookmarkMenu;

    void createBookmark();
    void populateBookmarkList();
    void loadBookmark(int i);
    void deleteBookmark(int i);
};
