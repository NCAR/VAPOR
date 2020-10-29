#pragma once

#include "PWidget.h"

class QListWidget;
class QListWidgetItem;

//! \class PMultiVarSelector
//! \brief Allows the selection of multiple variables. To add a title, use a PLabel.
//! \author Stas Jaroszynski

class PMultiVarSelector : public PWidget {
    QListWidget *_listWidget;
    
public:
    PMultiVarSelector(std::string tag);
    
protected:
    void updateGUI() const override;
    QSize minimumSizeHint() const override;
    bool requireDataMgr() const override { return true; }
    
private:
    QListWidgetItem *addVarToList(const std::string &var) const;
    void itemChanged(QListWidgetItem*);
};
