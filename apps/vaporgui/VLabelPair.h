#include "VHBoxWidget.h"

class QLabel;

//! \class VLabelPair
//! \brief A VHBoxWidget that contains a pair of labels that can be set independently

class VLabelPair : public VHBoxWidget {
    Q_OBJECT

    QLabel *_leftLabel;
    QLabel *_rightLabel;

public:
    VLabelPair();
    void SetLeftText(const std::string &text) const;
    void SetRightText(const std::string &text) const;
};
