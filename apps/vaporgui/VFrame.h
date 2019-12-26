#pragma once

#include <QWidget>
#include <QFrame>


//! \class VFrame
//!
//! A simple class that sets a layout, spacing, and margin
//! policy onto a QFrame for consistency throughout the application.
class VFrame : public QFrame {
    Q_OBJECT
    
public:
    VFrame(); 

    void addWidget( QWidget* widget);
};
