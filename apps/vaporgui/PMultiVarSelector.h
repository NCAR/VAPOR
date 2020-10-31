#pragma once

#include "PWidget.h"

class QListWidget;
class QListWidgetItem;

//! \class PMultiVarSelector
//! \brief Allows the selection of multiple variables. To add a title, use a PLabel.
//! \author Stas Jaroszynski
//! By default this shows the vars with the same dimension as the renderer. This can be changed
//! with DisplayVars(Mode)

class PMultiVarSelector : public PWidget {
public:
    enum Mode { Auto, D2, D3, Both };

    PMultiVarSelector(std::string tag);
    PMultiVarSelector *DisplayVars(Mode mode);

protected:
    void  updateGUI() const override;
    QSize minimumSizeHint() const override;
    bool  requireDataMgr() const override { return true; }

private:
    QListWidget *_listWidget;
    Mode         _mode = Auto;

    QListWidgetItem *_addVarToList(const std::string &var) const;
    void             _itemChanged(QListWidgetItem *);
};
