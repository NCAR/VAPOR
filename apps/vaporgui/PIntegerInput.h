#pragma once

#include "PLineItem.h"
#include "PDynamicMixin.h"

class VIntSpinBox;

//! \class PIntegerInput
//! Creates a Qt text input for double values using a spinbox synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PIntegerInput : public PLineItem, public PDynamicMixin {
    Q_OBJECT

    VIntSpinBox *_spinbox;

public:
    PIntegerInput(const std::string &tag, const std::string &label = "");
    //! @copydoc VIntSpinBox::SetRange
    PIntegerInput *SetRange(int min, int max);

protected:
    void updateGUI() const override;

private slots:
    void spinboxValueChanged(int i);
    void valueChangedIntermediate(int i);
};
