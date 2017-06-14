#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#ifndef RANGECONTROLLER_H
#define RANGECONTROLLER_H

#include <qdialog.h>
#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <cassert>
#include <vector>

using namespace std;

class Observer;
class Range;

//! \class Observee
//! \brief A base class for inheriting traits for updating observers
//! \author Scott Pearse
//! \version $Revision$
//! \date $Date$

//! Observee class is an object that holds a common state for
//! several other Observer objects.  Whenever the observee gets
//! changed, it sends a signal to its observers to update their
//! state.
class Observee {
  public:
    virtual ~Observee(){};

    //! The Observee will need to add observers
    //! as they are instantiated, so that they can
    //! be notified when the Observee's state is changed
    //! \param[in] Observer object to register so that it
    //! can be updated when state changes are made
    bool addObserver(Observer *observer);
    bool removeObserver(Observer *observer);

    // When one of our observer widgets gets changed,
    // it will call its Notify() function which triggers
    // an update to the Range/Observee.
    bool notifyObservers();

  protected:
    //! Protect the constructor.  This should only be
    //! inherited.
    //Observee() {};
    Observee(){};

  private:
    // A list of observer objects to be updated upon state change.
    vector<Observer *> _observers;
};

//! \class Observer
//! \brief A base class for inheriting traits needed by observers objects
//! to update their values upon changes to Observee objects
//! \author Scott Pearse
//! \version $Revision$
//! \date $Date$

//! Each instance of the Observer class represents a widget
//! that needs to display the current state of their Observee.
//! When the Observee gets changed, it will call each Observer's
//! notify() function so that they are update to the correct state.
//!
class Observer {

  public:
    virtual ~Observer(){};
    //! Each inheritor of the Observer class will need to implement
    //! its own notify() method.  Observees will call notify on each
    //! of their registered observers when their state has been changed.
    //! It is the responsibility of the notify() function to update
    //! Observer values accordingly.
    virtual void notify() = 0;

  protected:
    //! Protect the constructor.  This should only be
    //! inherited.
    Observer(){};
};

//! \class Controller
//! \brief A base class for inheriting functions that change Observees
//! \author Scott Pearse
//! \version $Revision$
//! \date $Date$

//! Widgets that can modify observees also inherit from the
//! Controller class.  Not all Observers are Controllers.  For example,
//! a QLabel has no way of receiving user input or modifying an Observee.
//!
class Controller : public QObject {

    Q_OBJECT

  public:
    //! Constructor for the Controller class.
    //! \param[in] range - A Range object that will be modified by the controller.
    //! \param[in] type - A specifyer that tells whether the controller operates on the
    //! minimum or maximum value of the range.
    Controller(Range *range,
               int type = 0);
    ~Controller(){};

  public slots:
    //! updateValue() must be implemented by all derivations of Controller
    //! to specify how the targetted Range will have new values applied.
    virtual void updateValue() = 0;

  protected:
    Range *_range;
    double _min;
    double _max;
    double _val;
    int _type;
};

//! \class Range
//! \brief A class for specifying the extents of a 1D numeric range
//! \author Scott Pearse
//! \version $Revision$
//! \date $Date$
//! The Range class is a container that holds information
//! on the extents along a single axis within the domain.
//! It inherits from the Observee class, as it is being
//! observed by a multitude of Observer widgets that need
//! to update when extents are changed.
//!
class Range : public Observee {
  public:
    //! Range constructor
    //! param[in] min The minimum value of the Range domain
    //! param[in] max The maximum value of the Range domain
    //!
    Range(double min, double max);
    ~Range(){};

    //! Set/Get Domain extents functions
    //! These specify the extents of the entire 1D domain within
    //! the Range.
    //
    void setDomainMin(double v);

    //! \copydoc setDomainMin()
    //
    void setDomainMax(double v);

    //! \copydoc setDomainMin()
    //
    double getDomainMin() const { return _domainMin; }

    //! \copydoc setDomainMin()
    //
    double getDomainMax() const { return _domainMax; }

    //! Set/Get User extents functions
    //! These specify what extents the user has chosen.  These
    //! will typically be called by Controller->updateValue().
    //
    void setUserMin(double v);

    //! \copydoc setUserMin()
    //
    void setUserMax(double v);

    //! \copydoc setUserMin()
    //
    double getUserMin() const { return _userMin; }

    //! \copydoc setUserMin()
    //
    double getUserMax() const { return _userMax; }

    //! Set/Get constant functions are used when single-point
    //! selection information is needed.  If the user is selecting
    //! a single point instead of a range, setConst(true) should be called.
    //
    bool getConst() const { return _constant; }

    //! \copydoc getConst()
    //
    void setConst(bool c);

  private:
    double _domainMin;
    double _domainMax;
    double _userMin;
    double _userMax;
    bool _constant;
};

//! Observer/Controller for a Min/Max slider.  Type 0
//! MinMaxSliders control the minimum, and type 1 sliders
//! control their Range's maximum.
//!
class MinMaxSlider : public Controller, public Observer {

    Q_OBJECT

  public:
    //! Constructor recieves its associated Range, QWidget,
    //! and its type.
    //! \param[in] - Range object for the widget to control
    //! \param[in] - Qt widget to control the Range
    //! \param[in] - Type specifiyer to distinguish between a min/max controller
    //! type 0 = min value controller
    //! type 1 = max value controller
    //
    MinMaxSlider(Range *range,
                 QSlider *slider,
                 int type = 0);
    ~MinMaxSlider(){};
    void notify();

  public slots:
    //! \copydoc Controller::updateValue()
    //
    void updateValue();

  private:
    int _position;

  protected:
    QSlider *_slider;
    int _increments;
};

//! Observer/Controller for a time slider.  This differs
//! from a Min/Max slider in that it increments in integer
//! values, not decimal.  Type 0 TimeSliders the minimum,
//! and type 1 sliders control their Range's maximum.
//!
class TimeSlider : public MinMaxSlider {
    Q_OBJECT

  public:
    //! \copydoc MinMaxSlider::MinMaxSlider()
    //
    TimeSlider(Range *range,
               QSlider *slider,
               int type = 0);
    ~TimeSlider(){};
    void setConst(bool c);
};

//! RangeLabels are a base class for Min/Max labels and
//! Center/Size labels.  They update whenever their Range's
//! domain extents are changed.
//!
class RangeLabel : public QObject, public Observer {
    Q_OBJECT

  public:
    //! \copydoc MinMaxSlider::MinMaxSlider()
    //
    RangeLabel(Range *range,
               QLabel *label,
               int type = 0);
    ~RangeLabel(){};
    void setVal(double val) { _val = val; }
    double getVal() { return _val; }
    virtual void notify() = 0;

  protected:
    Range *_range;
    double _val;
    QLabel *_label;
    int _type;
};

//! MinMaxLabels are a specialization of the RangeLabel.
//! Type 0 MinMaxLabels display their Range's domain min,
//! while type 1 display their Range's max.
//!
class MinMaxLabel : public RangeLabel {
    Q_OBJECT

  public:
    MinMaxLabel(Range *range,
                QLabel *label,
                int type = 0);
    ~MinMaxLabel(){};
    void notify();

  private:
    Range *_range;
    QLabel *_label;
    int _type;
};

//! SizeLabels are a specialization of the RangeLabel.
//! Type 0 SizeLabels display the size's minimum (0),
//! while type 1 display the size's max.
//!
class SizeLabel : public RangeLabel {
    Q_OBJECT

  public:
    SizeLabel(Range *range,
              QLabel *label,
              int type = 0);
    ~SizeLabel(){};
    void notify();

  private:
    Range *_range;
    QLabel *_label;
    int _type;
};

//! MinMaxLineEdits provide a textual input for the user to
//! apply minima or maxima to a given range.  Type 0 controlls
//! minima, and type 1 controls maxima.
//!
class MinMaxLineEdit : public Controller, public Observer {

    Q_OBJECT

  public:
    //! \copydoc MinMaxSlider::MinMaxSlider()
    //
    MinMaxLineEdit(Range *range,
                   QLineEdit *lineEdit,
                   int type);
    ~MinMaxLineEdit(){};
    void notify();

  public slots:
    //! \copydoc Controller::updateValue()
    //
    void updateValue();

  private:
    QLineEdit *_lineEdit;
};

//! SinglePointSliders control both the _userMin and _userMax
//! values of their Range simultaneously.  For this reason, they
//! have no type, unlike Center/Size and Min/Max sliders.
//!
class SinglePointSlider : public Controller, public Observer {

    Q_OBJECT

  public:
    //! Constructor recieves its associated Range, QWidget,
    //! and its default position.
    //! \param[in] - Range object for the widget to control
    //! \param[in] - Qt widget to control the Min/Max
    //! \param[in] - Default position of the slider
    //
    SinglePointSlider(Range *range,
                      QSlider *slider,
                      int defaultPos = 2);
    ~SinglePointSlider(){};
    void notify();

  public slots:
    //! \copydoc Controller::updateValue()
    //
    void updateValue();

  private:
    QSlider *_slider;
    int _position;
    int _increments;
};

//! Like the SinglePointSlider, SinglePointLineEdits control
//! both the _userMin and _userMax values of their given Range.
//! they have no type.
//!
class SinglePointLineEdit : public Controller, public Observer {

    Q_OBJECT

  public:
    //! Constructor recieves its associated Range, QWidget,
    //! and its default position.
    //! \param[in] - Range object for the widget to control
    //! \param[in] - Qt widget to control the Min/Max
    //! \param[in] - Default value of the LineEdit
    //
    SinglePointLineEdit(Range *range,
                        QLineEdit *lineEdit,
                        double defaultVal);
    ~SinglePointLineEdit(){};
    void notify();

  public slots:
    //! \copydoc Controller::updateValue()
    //
    void updateValue();

  private:
    QLineEdit *_lineEdit;
};

//! CenterSizeSliders of type 0 control where the center of
//! the selected user extents resides.  Type 1 controls the size
//! of the extents around the center.  Instead of holding state for
//! the center and size that the user has selected, this class
//! calculates a _userMin and _userMax value to get/set upon its
//! given range.
//!
class CenterSizeSlider : public Controller, public Observer {

    Q_OBJECT

  public:
    //! Constructor recieves its associated Range, QWidget,
    //! and its type.
    //! \param[in] - Range object for the widget to control
    //! \param[in] - Qt widget to control the Range
    //! \param[in] - Type specifiyer to distinguish between a center/size controller
    //! type 0 = center value controller
    //! type 1 = size value controller
    //
    CenterSizeSlider(Range *range,
                     QSlider *slider,
                     int type);
    ~CenterSizeSlider(){};
    void notify();

  public slots:
    //! \copydoc Controller::updateValue()
    //
    void updateValue();

  private:
    QSlider *_slider;
    int _increments;
    double _binSize;
};

//! \copydoc CenterSizeSlider
//
class CenterSizeLineEdit : public Controller, public Observer {

    Q_OBJECT

  public:
    //! Constructor recieves its associated Range, QWidget,
    //! and its type.
    //! \param[in] - Range object for the widget to control
    //! \param[in] - Qt widget to control the Range
    //! \param[in] - Type specifiyer to distinguish between a center/size controller
    //! type 0 = center value controller
    //! type 1 = size value controller
    //
    CenterSizeLineEdit(Range *range,
                       QLineEdit *lineEdit,
                       int type);
    ~CenterSizeLineEdit(){};
    void notify();

  public slots:
    //! \copydoc Controller::updateValue()
    //
    void updateValue();

  private:
    QLineEdit *_lineEdit;
};

//! MinMaxTableCells provide the user another way to edit their
//! selected minima and maxima through a display table.  Type 0
//! controls the minimum, and type 1 controls the maximum.
//!
class MinMaxTableCell : public Controller, public Observer {
    Q_OBJECT

  public:
    //! Constructor recieves its associated Range, QWidget,
    //! type, table row, and table column.
    //! \param[in] - Range object for the widget to control
    //! \param[in] - Qt widget to control the Range
    //! \param[in] - Type specifiyer to distinguish between a min/max controller
    //! type 0 = min value controller
    //! type 1 = max value controller
    //! \param[in] - row designating the row in the cell's table
    //! \param[in] - col designating the column in the cell's table
    //
    MinMaxTableCell(Range *range,
                    QTableWidget *table,
                    int type, int row, int col);
    ~MinMaxTableCell(){};
    void notify();

  public slots:
    //! Filler function since MinMaxTableCell inherits from the Controller
    //! class, which defines updateValue() as pure virtual.  The updateValue
    //! for a table cell will need to know the cell row and column, which is
    //! implemented in updateValue(int row, int col).
    //
    void updateValue(){};

    //! \copydoc Controller::updateValue()
    //
    void updateValue(int row, int col);

  private:
    QTableWidget *_table;
    QTableWidgetItem *_cell;
    int _row;
    int _col;
};

#endif
