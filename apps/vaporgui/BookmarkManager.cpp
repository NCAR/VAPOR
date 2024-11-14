#include "BookmarkManager.h"
#include <QApplication>
#include <QMenu>
#include <QInputDialog>
#include <vapor/BookmarkParams.h>
#include <vapor/Base16StringStream.h>
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>
#include "VizWinMgr.h"
#include "QCustomIconSizeProxyStyle.h"


BookmarkManager::BookmarkManager(QObject *parent, VAPoR::ControlExec *ce, VizWinMgr *vizWinMgr)
: QObject(parent),
  _controlExec(ce), _paramsMgr(ce->GetParamsMgr()), _vizWinMgr(vizWinMgr),
  _loadBookmarkMenu(new QMenu("Load Bookmark")),
  _deleteBookmarkMenu(new QMenu("Delete Bookmark"))
{
    _loadBookmarkMenu->setStyle(new QCustomIconSizeProxyStyle(BookmarkParams::DefaultIconSize()));
    _deleteBookmarkMenu->setStyle(new QCustomIconSizeProxyStyle(BookmarkParams::DefaultIconSize()));
}


void BookmarkManager::RegisterToMenu(QMenu *menu)
{
    menu->addAction("Create Bookmark", this, &BookmarkManager::createBookmark);
    menu->addMenu(_loadBookmarkMenu);
    menu->addMenu(_deleteBookmarkMenu);
}


void BookmarkManager::Update()
{
    populateBookmarkList();
}


void BookmarkManager::createBookmark()
{
    bool    ok;
    QString input = QInputDialog::getText(nullptr, "New Bookmark", "Bookmark Name:", QLineEdit::Normal, "", &ok);
    if (!ok) return;

    string title = input.toStdString();
    if (title.empty()) title = "Unnamed Bookmark";

    auto p = _controlExec->GetParams<GUIStateParams>();

    p->BeginGroup("Create Bookmark");

    vector<BookmarkParams> bookmarks;
    for (auto *b : p->GetBookmarks()) bookmarks.push_back(BookmarkParams(*b));
    p->ClearBookmarks();

    Base16StringStream ss;
    XmlNode::streamOut(ss, *_paramsMgr->GetXMLRoot());

    for (auto &b : bookmarks) p->AddBookmark(&b);

    string           activeVizWin = p->GetActiveVizName();
    ViewpointParams *vpp = _paramsMgr->GetViewpointParams(activeVizWin);

    bool useCustomViewport = vpp->GetValueLong(vpp->UseCustomFramebufferTag, 0);
    int  customViewportWidth = vpp->GetValueLong(vpp->CustomFramebufferWidthTag, 0);
    int  customViewportHeight = vpp->GetValueLong(vpp->CustomFramebufferHeightTag, 0);

    const int iconSize = BookmarkParams::DefaultIconSize();
    const int iconDataSize = iconSize * iconSize * 3;
    auto      iconData = std::unique_ptr<unsigned char[]>(new unsigned char[iconDataSize]);

    char iconDataString[64];
    snprintf(iconDataString, 64, ":RAM:%p", iconData.get());
    // The above string is a "file path" that points to an address in memory
    // which tells the visualizer to save the resulting image to ram rather than
    // to disk. The current "image capture" implementation is very buggy spaghetti
    // and this is the cleanest solution without major refactoring.

    vpp->SetValueLong(vpp->UseCustomFramebufferTag, "", true);
    vpp->SetValueLong(vpp->CustomFramebufferWidthTag, "", iconSize);
    vpp->SetValueLong(vpp->CustomFramebufferHeightTag, "", iconSize);

    _vizWinMgr->EnableImageCapture(iconDataString, activeVizWin);

    vpp->SetValueLong(vpp->UseCustomFramebufferTag, "", useCustomViewport);
    vpp->SetValueLong(vpp->CustomFramebufferWidthTag, "", customViewportWidth);
    vpp->SetValueLong(vpp->CustomFramebufferHeightTag, "", customViewportHeight);

    Base16StringStream is;
    is.write((char *)iconData.get(), iconDataSize);

    BookmarkParams *b = p->CreateBookmark();
    b->SetName(title);
    b->SetIcon(iconSize, is.ToString());
    b->SetData(ss.ToString());
    p->EndGroup();
}


void BookmarkManager::populateBookmarkList()
{
    auto bookmarks = _controlExec->GetParams<GUIStateParams>()->GetBookmarks();
    _loadBookmarkMenu->clear();
    _deleteBookmarkMenu->clear();

    if (bookmarks.empty()) {
        _loadBookmarkMenu->addAction("(empty)")->setEnabled(false);
        _deleteBookmarkMenu->addAction("(empty)")->setEnabled(false);
    }

    unsigned char *buf = nullptr;
    size_t         bufSize = 0;

    int i = 0;
    for (auto b : bookmarks) {
        if (bufSize != b->GetIconDataSize()) {
            if (buf) delete[] buf;
            buf = new unsigned char[b->GetIconDataSize()];
            bufSize = b->GetIconDataSize();
        }

        Base16DecoderStream ds(b->GetIconData());
        ds.read((char *)buf, bufSize);
        int    s = b->GetIconSize();
        QImage iconImage(buf, s, s, s * 3, QImage::Format_RGB888);
        QIcon  icon(QPixmap::fromImage(iconImage));

        _loadBookmarkMenu->addAction(icon, QString::fromStdString(b->GetName()), [this, i]() { loadBookmark(i); });
        _deleteBookmarkMenu->addAction(icon, QString::fromStdString(b->GetName()), [this, i]() { deleteBookmark(i); });
        i++;
    }

    if (buf) delete[] buf;
}


void BookmarkManager::loadBookmark(int i)
{
    auto p = _controlExec->GetParams<GUIStateParams>();

    XmlNode             root;
    XmlParser           parser;
    Base16DecoderStream stream(p->GetBookmark(i)->GetData());
    parser.LoadFromFile(&root, stream);

    bool paramsStateSaveEnabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    vector<BookmarkParams> bookmarks;
    for (auto *b : p->GetBookmarks()) bookmarks.push_back(BookmarkParams(*b));

    _controlExec->LoadState(&root);
    p = _controlExec->GetParams<GUIStateParams>();
    p->ClearBookmarks();
    for (auto &b : bookmarks) p->AddBookmark(&b);

    _controlExec->SetDataCacheDirty(); // TODO <small> determine if necessary

    _controlExec->SetSaveStateEnabled(paramsStateSaveEnabled);
    _paramsMgr->UndoRedoClear();
    _paramsMgr->TriggerManualStateChangeEvent();
}


void BookmarkManager::deleteBookmark(int i)
{
    auto p = _controlExec->GetParams<GUIStateParams>();
    p->DeleteBookmark(i);
}