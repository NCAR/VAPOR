#pragma once

#include "PLineItem.h"

//! \class PCornerSelector
//! \brief Widget for selecting a corner or side of a square
//! \author Stas Jaroszynski
//!
//! Widget that presents a collection of checkboxes orientated in a square, allowing the user to
//! to select either a corner or side.

class PCornerSelector : public PLineItem {
    class Check;
    QWidget *            _w;
    std::vector<Check *> _checks;
    float                padding = 0.03;

public:
    PCornerSelector(std::string tag, std::string title);

protected:
    static bool equalsf(float a, float b) { return (std::abs(b - a) <= 0.05); }

    void                        updateGUI() const override;
    void                        checked(bool on);
    virtual std::vector<double> getValue() const;
    virtual void                setValue(const std::vector<double> &v);
};

//! \class PColorbarCornerSelector
//! \brief Specialization of PCornerSelector for ColorbarPbase
//! \author Stas Jaroszynski
//! \copydoc PCornerSelector

class PColorbarCornerSelector : public PCornerSelector {
public:
    PColorbarCornerSelector();
    virtual std::vector<double> getValue() const override;
    virtual void                setValue(const std::vector<double> &v) override;
};
