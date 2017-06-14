//************************************************************************
//									*
//		     Copyright (C)  2013				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		test_vizwin.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		January 2016
//
//

#ifndef TEST_VIZWIN_H
#define TEST_VIZWIN_H

#include <vapor/glutil.h>
#include <QGLWidget>
#include <QWheelEvent>

class QCloseEvent;
class QRect;
class QMouseEvent;
class QFocusEvent;

namespace VAPoR {
class Viewpoint;
class ControlExec;
//! \class Test_VizWin
//! \ingroup Public_GUI
//! \brief A QGLWidget that supports display based on GL methods invoked in a Visualizer
//! \author Alan Norton
//! \version 3.0
//! \date October 2013
//!
//!	The VizWin class is a QGLWidget that supports the rendering by the VAPOR Visualizer class.
//! The standard rendering methods (resize, initialize, paint) are passed to the Visualizer.
//! In addition this is the class that responds to mouse events, resulting in scene navigation
//! or manipulator changes.
//!
class Test_VizWin : public QGLWidget {
    Q_OBJECT

  public:
    //! Force the window to update, even if nothing has changed.
    void reallyUpdate();

    Test_VizWin(QWidget *parent, ControlExec *, std::string winName);

    virtual ~Test_VizWin() {}
#ifndef DOXYGEN_SKIP_THIS

  private:
    bool m_initialized;
    void closeEvent(QCloseEvent *event);
    virtual QSize minimumSizeHint() const { return QSize(400, 400); }

    virtual void resizeGL(int width, int height);
    virtual void initializeGL();
    void paintGL();

    std::string _windowName;
    ControlExec *_controlExec;

#endif //DOXYGEN_SKIP_THIS
};
}; // namespace VAPoR

#endif // TEST_VIZWIN_H
