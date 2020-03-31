#pragma once

#include "PLineItem.h"

class VPushButton;
class VLineEdit;
class QString;

//! \class PFileSelector
//! Creates a Qt text box and select button that allows users to pick a file path synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PFileSelector : public PLineItem {
    Q_OBJECT

    VPushButton *_button;
    VLineEdit *  _pathTexbox;

    bool        _syncWithSettings = false;
    std::string _syncWithSettingsTag;

public:
    PFileSelector(const std::string &tag, const std::string &label = "");
    //! Sets the fileTypeFilter parameter in the QFileDialog popup functions.
    PFileSelector *SetFileTypeFilter(const std::string &filter);
    //    PFileSelector *UseDefaultPathSetting(const std::string &tag);

protected:
    QString _fileTypeFilter = "All Files (*)";

    void            updateGUI() const override;
    bool            requireParamsMgr() const override;
    virtual QString selectPath(const std::string &defaultPath) const = 0;

private slots:
    void buttonClicked();
};

//! \class PFileOpenSelector
//! A PFileSelector that provides an Open File dialog
//! \copydoc PFileSelector

class PFileOpenSelector : public PFileSelector {
    Q_OBJECT
public:
    using PFileSelector::PFileSelector;

protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};

//! \class PFileSaveSelector
//! A PFileSelector that provides a Save File dialog
//! \copydoc PFileSelector

class PFileSaveSelector : public PFileSelector {
    Q_OBJECT
public:
    using PFileSelector::PFileSelector;

protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};

//! \class PDirectorySelector
//! A PFileSelector that provides a  Select Directory dialog
//! \copydoc PFileSelector

class PDirectorySelector : public PFileSelector {
    Q_OBJECT
public:
    using PFileSelector::PFileSelector;
    PFileSelector *SetFileTypeFilter(const std::string &filter) = delete;

protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};
