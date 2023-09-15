#pragma once

#include "PWidget.h"

class VPushButton;
class QString;


//! \class PFileButton
//! Creates a Qt text box and select button that allows users to pick a file path synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PFileButton : public PWidget {
    Q_OBJECT

private:
    typedef std::function<void(std::string)> Callback;
    Callback _cb;
    VPushButton* _button;

public:
    PFileButton(const std::string label, Callback cb);
    //! Sets the fileTypeFilter parameter in the QFileDialog popup functions.
    PFileButton *SetFileTypeFilter(const std::string &filter);
    //    PFileButton *UseDefaultPathSetting(const std::string &tag);

protected:
    QString _fileTypeFilter = "All Files (*)";

    void            updateGUI() const override;
    bool            requireParamsMgr() const override;
    virtual QString selectPath(const std::string &defaultPath) const = 0;

//private slots:
    void clicked();
};

//! \class PFileOpenSelector
//! A PFileButton that provides an Open File dialog
//! \copydoc PFileReader

class PFileReader : public PFileButton {
    Q_OBJECT
public:
    using PFileButton::PFileButton;

protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};

//! \class PFileSaveSelector
//! A PFileButton that provides a Save File dialog
//! \copydoc PFileReader

class PFileWriter : public PFileButton {
    Q_OBJECT
public:
    using PFileButton::PFileButton;

protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};
