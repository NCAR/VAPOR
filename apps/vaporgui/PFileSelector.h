#pragma once

#include "PLineItem.h"

class VPushButton;
class VLineEdit;
class QString;


class PFileSelector : public PLineItem {
    Q_OBJECT
    
    VPushButton *_button;
    VLineEdit *_pathTexbox;
    
    bool _syncWithSettings = false;
    std::string _syncWithSettingsTag;
    
public:
    PFileSelector(const std::string &tag, const std::string &label="");
    PFileSelector *SetFileTypeFilter(const std::string &filter);
//    PFileSelector *UseDefaultPathSetting(const std::string &tag);
    
protected:
    QString _fileTypeFilter = "All Files (*)";
    
    void updateGUI() const override;
    bool requireParamsMgr() const override;
    virtual QString selectPath(const std::string &defaultPath) const = 0;
        
private slots:
    void buttonClicked();
};



class PFileOpenSelector : public PFileSelector {
    Q_OBJECT
public:
    using PFileSelector::PFileSelector;
protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};

class PFileSaveSelector : public PFileSelector {
    Q_OBJECT
public:
    using PFileSelector::PFileSelector;
protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};

class PDirectorySelector : public PFileSelector {
    Q_OBJECT
public:
    using PFileSelector::PFileSelector;
    PFileSelector *SetFileTypeFilter(const std::string &filter) = delete;
protected:
    virtual QString selectPath(const std::string &defaultPath) const override;
};


