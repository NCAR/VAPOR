#pragma once

#include <QWidget>

class VFrame : public QFrame {
    Q_OBJECT
    
public:
    VFrame() {
        setLayout( new QVBoxLayout );
        setContentsMargins( 0, 0, 0, 0 );
        setFrameStyle( QFrame::NoFrame );
    }
};
