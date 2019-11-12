#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <string>

#define LEFT_MARGIN 0
#define TOP_MARGIN 0
#define RIGHT_MARGIN 0
#define BOTTOM_MARGIN  0

class VContainer : public QWidget {
    Q_OBJECT

protected:
    VContainer( QWidget* containee ) {
        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(
            LEFT_MARGIN,
            TOP_MARGIN,
            RIGHT_MARGIN,
            BOTTOM_MARGIN
        ); 
        setLayout( layout );
        
        setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
    }

    ~VContainer() {};
};
