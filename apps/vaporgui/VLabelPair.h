#include "VHBoxWidget.h"

class QLabel;

//! \class VLabelPair
//! \brief A VHBoxWidget that contains a pair of labels that can be set independently

class VLabelPair : public VHBoxWidget {
    Q_OBJECT

    QLabel *_leftTime;
    QLabel *_rightTime;

public:
    VLabelPair();
    void SetLeftText(const std::string &text);
    void SetRightText(const std::string &text);
};
