#pragma once

#include "PLineItem.h"
#include "PDynamicMixin.h"
//#include "VaporWidgetsFwd.h"

class VSliderEdit;

//! \class PSliderEdit
//! Creates a Qt slider and text input combo synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PSliderEdit : public PLineItem, public PDynamicMixin {
    Q_OBJECT

public:
    PSliderEdit(const std::string &tag, const std::string &label = "");
    //! @copydoc VSliderEdit::SetRange
    PSliderEdit *SetRange(double min, double max);

protected:
    VSliderEdit *_sliderEdit;
};

//! \class PDoubleSliderEdit
//! \copydoc PSliderEdit

class PDoubleSliderEdit : public PSliderEdit {
    Q_OBJECT

public:
    PDoubleSliderEdit(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void valueChanged(double v);
    void valueChangedIntermediate(double v);
};

//! \class PIntegerSliderEdit
//! \copydoc PSliderEdit

class PIntegerSliderEdit : public PSliderEdit {
    Q_OBJECT

public:
    PIntegerSliderEdit(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void valueChanged(int v);
    void valueChangedIntermediate(int v);
};
