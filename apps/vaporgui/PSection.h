#pragma once

#include "PWidget.h"

class VSection;
class PGroup;

//! \class PSection
//! Same as a PGroup however collated inside of a VSection
//! \copydoc PGroup
//! \copydoc VSection

class PSection : public PWidget {
    Q_OBJECT
    
    PGroup *_pgroup;
    
public:
    PSection(const std::string &label="");
    //! @copydoc PGroup::Add
    PSection *Add(PWidget *pw);

protected:
    VSection *_vsection;

    void updateGUI() const override;
};
