#pragma once

#include <QFileDialog>
#include "vapor/ImageParams.h"
#include "vapor/GeoImageTMS.h"
#include "vapor/TMSUtils.h"
#include "PGroup.h"
#include "VComboBox.h"

//
// PWidget for selecting TMS file level-of-detail
//

class PTMSLODInput : public PLineItem {
    Q_OBJECT
    VComboBox *_vComboBox;

public:
    PTMSLODInput() : PLineItem("", "TMS level of detail", _vComboBox = new VComboBox({"0"}))
    {
        QString tooltip = "TMS images can be displayed at varying resolutions.\n"
                          "This setting allows for manual resolution selection.";
        _vComboBox->setToolTip(tooltip);
        connect(_vComboBox, &VComboBox::IndexChanged, this, &PTMSLODInput::dropdownIndexChanged);
    }

protected:
    virtual void updateGUI() const override
    {
        VAPoR::ImageParams *rp = dynamic_cast<VAPoR::ImageParams *>(getParams());
        VAssert(rp && "Params must be ImageParams");

        std::string imageFile = rp->GetImagePath();
        if (Wasp::TMSUtils::IsTMSFile(imageFile)) {
            _vComboBox->setEnabled(true);
        } else {
            _vComboBox->setEnabled(false);    // Disable if not using a TMS image
            return;
        }

        std::vector<std::string> options{"default"};
        int                      lods = rp->GetNumTMSLODs();
        for (int i = 0; i < lods; i++) { options.push_back(std::to_string(i)); }
        _vComboBox->SetOptions(options);
        _vComboBox->SetIndex(rp->GetTMSLOD() + 1);
    };

private slots:
    void dropdownIndexChanged(int i)
    {
        VAPoR::ImageParams *rp = dynamic_cast<VAPoR::ImageParams *>(getParams());
        VAssert(rp && "Params must be ImageParams");
        rp->SetTMSLOD(i - 1);
    }
};
