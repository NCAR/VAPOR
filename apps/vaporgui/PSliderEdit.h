#pragma once

#include "PLineItem.h"
#include "PDynamicMixin.h"

class VDoubleSliderEdit;
class VIntSliderEdit;

//! \class PDoubleSliderEdit
//! Creates a slider and text input combo synced with the paramsdatabase.
//! Due to the way VDoubleSliderEdit and VIntSliderEdit are implemented,
//! duplicating the code was simpler than templatizing.

class PDoubleSliderEdit : public PLineItem, public PDynamicMixin {
    VDoubleSliderEdit *_sliderEdit;
    double             _defaultRangeMin = 0, _defaultRangeMax = 1;

public:
    PDoubleSliderEdit(const std::string &tag, const std::string &label = "");
    //! @copydoc VSliderEdit::SetRange
    PDoubleSliderEdit *SetRange(double min, double max);
    PDoubleSliderEdit *AllowUserRange(bool allowed = true);

protected:
    void updateGUI() const override;

private:
    void valueChanged(double v);
    void valueChangedIntermediate(double v);
    void minimumChanged(double v);
    void maximumChanged(double v);
};

//! \class PIntegerSliderEdit
//! Creates a slider and text input combo synced with the paramsdatabase.
//! Due to the way VDoubleSliderEdit and VIntSliderEdit are implemented,
//! duplicating the code was simpler than templatizing.

class PIntegerSliderEdit : public PLineItem, public PDynamicMixin {
    VIntSliderEdit *_sliderEdit;
    int             _defaultRangeMin = 0, _defaultRangeMax = 1;

public:
    PIntegerSliderEdit(const std::string &tag, const std::string &label = "");
    //! @copydoc VSliderEdit::SetRange
    PIntegerSliderEdit *SetRange(int min, int max);
    PIntegerSliderEdit *AllowUserRange(bool allowed = true);

protected:
    void updateGUI() const override;

private:
    void valueChanged(int v);
    void valueChangedIntermediate(int v);
    void minimumChanged(int v);
    void maximumChanged(int v);
};
