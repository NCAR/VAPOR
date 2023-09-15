#pragma once

#include "PWidget.h"
#include "PGroup.h"

class VSection;

//! \class PSection
//! Same as a PGroup however collated inside of a VSection
//! \copydoc PGroup
//! \copydoc VSection

class PSection : public PWidget {
    Q_OBJECT

    VSection *_vsection;
    PGroup *  _pgroup;

public:
    PSection(const std::string &label = "", const PGroup::List &widgets = {});
    //! @copydoc PGroup::Add
    PSection *Add(PWidget *pw);
    PSection *Add(const PGroup::List &widgets);

protected:
    void updateGUI() const override;
};
