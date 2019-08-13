#ifndef VAPORWIDGETS_H 
#define VAPORWIDGETS_H 

class QDir;
class QLabel;
class QWidget;
class QSlider;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QSpacerItem;
class QHBoxLayout;
class QVBoxLayout;
class QDoubleSpinBox;

#include <cmath>
#include <QTabWidget>
#include <QFileDialog>

class VaporWidget : public QWidget
{
    Q_OBJECT

public:
    VaporWidget( QWidget* parent = 0 );
    
    template <class T>
    void Update( T value );

    //template <class T>
    //T GetValue() const;
    virtual int GetValue() {return -1;}

private slots:
    virtual void _validateAndEmit() = 0;

signals:
    void _valueChanged();
};

//
// ====================================
//
class VaporLine : public VaporWidget
{
    Q_OBJECT

public:
    void SetLabelText( const std::string& text );
    void SetLabelText( const QString& text );

    template <class T>
    void Update( T labelText );

    //template <class T>
    //T GetValue() const;
    virtual int GetValue() { return 0; }

protected:
    VaporLine( 
        QWidget* parent,
        const std::string& labelText
    );
    VaporLine( 
        QWidget* parent,
        const QString& labelText
    );

    QLabel*      _label;
    QSpacerItem* _spacer;
    QHBoxLayout* _layout;

protected slots:
    virtual void _validateAndEmit();
};

//
// ====================================
//
class VSpinBox : public VaporLine
{
    Q_OBJECT

public:
    VSpinBox(
        QWidget* parent, 
        const std::string& labelText = "Label",
        int min = 0,
        int max = 100,
        int defaultValue = 0 
    );

    template <class T>
    void Update( T value );
   
    //template <class T> 
    //T GetValue() const;
    virtual int GetValue() { return 1; }

    void SetMaximum( int maximum );
    void SetMinimum( int minimum );

protected:
    QSpinBox* _spinBox;

private:
    int _min;
    int _max;
    int _value;

protected slots:
    virtual void _validateAndEmit();
};



/*
//
// ====================================
//
class VDoubleSpinBox : public VaporLine
{
    Q_OBJECT

public:
    VDoubleSpinBox(
        QWidget* parent, 
        const std::string& labelText = "Label",
        double defaultValue = 0.f
    );

    void SetMaximum( double maximum );
    void SetMinimum( double minimum );
    void SetValue( double value );
    void SetDecimals( int decimals );
    double GetValue() const;

signals:
    void _emitValueChanged();

protected:
    QDoubleSpinBox* _spinBox;

private slots:
    void _changed();

private:
    double _value;
};

//
//
// ====================================
//
class VLineEdit : public VaporLine
{
    Q_OBJECT

public:
    VLineEdit(
        QWidget* parent, 
        const std::string& labelText = "Label",
        const std::string& editText = ""
    );
    ~VLineEdit();

    void SetEditText( const std::string& text );
    void SetEditText( const QString& text );
    std::string GetEditText() const;

signals:
    void _editingFinished();

protected:
    QLineEdit* _edit;

private slots:
    void _relaySignal();

private:
    std::string _text;
};

//
// ====================================
// VSlider augments QSlider by providing a text box displaying the value.
//
// Note: this widget is re-implemented by Sam instead of using the existing
//   QSliderEdit class because:
//   1) this class does NOT use a UI file following the VWidget convension;
//   2) this class does NOT validate the input as QSliderEdit does; and
//   3) this class does NOT use the Combo class that's deprecated.
//
// Note2: QSlider class always uses integer type for recording its positions.
//   Thus this widget uses an internal variable to keep the actual value in float.
// ====================================
//
class VSlider : public VaporLine
{
    Q_OBJECT

public:
    VSlider( QWidget* parent, const std::string& label, float min, float max );
    ~VSlider();

    void  SetRange( float min, float max );
    void  SetCurrentValue( float val );
    float GetCurrentValue() const;

signals:
    // This signal is emitted representing the entire widget
    void  _emitValueChanged();

private slots:
    void  _respondQSliderReleased();    // emit signal
    void  _respondQSliderMoved(int);    // sync qSlider and qLineEdit
    void  _respondQLineEdit();          // emit signal

private:
    float       _min, _max, _currentVal;
    QSlider*    _qslider;
    QLineEdit*  _qedit;
};

//
// ====================================
// VRange combines two VSliders, 
// representing the min and max values of a range.
// ====================================
//
class VRange : public QWidget
{
    Q_OBJECT

public:
    VRange( QWidget* parent, float min, float max,
            const std::string& minLabel = "Min",
            const std::string& maxLabel = "Max"  );
    ~VRange();

    void  SetRange( float min, float max );
    void  SetCurrentValLow(    float );
    void  SetCurrentValHigh(   float );
    void  GetCurrentValRange(  float& low, float& high ) const;

signals:
    void  _rangeChanged();

private slots:
    void  _respondMinSlider();
    void  _respondMaxSlider();

private:
    VSlider        *_minSlider, *_maxSlider;
    QVBoxLayout*    _layout;

    // In case _minSlider is changed, adjust _maxSlider if necessary.
    void  _adjustMaxToMin();    
    // In case _maxSlider is changed, adjust _minSlider if necessary.
    void  _adjustMinToMax();
};

//
// ====================================
// VGeometry combines two or three VRanges, 
// representing a 2D or 3D geometry.
// Note: this class is never supposed to be used beyond 2D and 3D cases.
// ====================================
//
class VGeometry : public QTabWidget
{
    Q_OBJECT

public:
    // Constructor for 2D or 3D geometries. 
    //   Four floating point values imply a 2D geometry, while Six floating point 
    //   values imply a 3D geometry. All other numbers are illegal.
    VGeometry( 
        QWidget* parent, 
        int dim, 
        const std::vector<float>& range,
        const std::string& label = "Geometry"
    );

    ~VGeometry();

    // Adjust the dimension and/or value ranges through this function.
    //   Argument range must contain 4 or 6 values organized in the following order:
    //   xmin, xmax, ymin, ymax, (zmin, zmax).                                    
    void  SetDimAndRange( int dim, const std::vector<float>& range );
    // The number of incoming values MUST match the current dimensionality. 
    //   I.e., 4 values for 2D widgets, and 6 values for 3D widgets.       
    void  SetCurrentValues( const std::vector<float>& vals );
    void  GetCurrentValues( std::vector<float>& vals ) const;

signals:
    void  _geometryChanged();

private slots:
    void  _respondChanges();

private:
    int          _dim;
    VRange      *_xrange, *_yrange, *_zrange;
    //QVBoxLayout* _layout;
    //QWidget*     _pageWidget;
};

//
// ====================================
//
class VPushButton : public VaporLine
{
    Q_OBJECT

public:
    VPushButton(
        QWidget* parent, 
        const std::string& labelText = "Label",
        const std::string& buttonText = "Button"
    );

    void SetButtonText( const std::string& text );
    void SetButtonText( const QString& text );

signals:
    void _pressed();

protected:
    QPushButton* _button;

private slots:
    void _buttonPressed();
};


//
// ====================================
//
class VComboBox : public VaporLine
{
    Q_OBJECT

public:
    VComboBox(
        QWidget* parent,
        const std::string& labelText = "Label"
    );
    int         GetCurrentIndex() const;
    std::string GetCurrentText() const;
    void        AddOption( const std::string& option, int index=0 );
    void        RemoveOption( int index );
    void        SetIndex( int index );
    int         GetNumOfItems() const;

private:
    QComboBox* _combo;

private slots:
    void _userIndexChanged(int index);

signals:
    void _indexChanged(int index);
};


//
// ====================================
//
class VCheckBox : public VaporLine
{
    Q_OBJECT

public:
    VCheckBox(
        QWidget* parent,
        const std::string& labelText = "Label"
    );
    bool GetCheckState() const;
    void SetCheckState( bool checkState );

private:
    QCheckBox* _checkbox;

private slots:
    void _userClickedCheckbox();

signals:
    void _checkboxClicked();
};


//
// ====================================
//
class VFileSelector : public VPushButton
{
    Q_OBJECT

public:
    void SetPath( const std::string& defaultPath);
    void SetPath( const QString& defaultPath);
    void SetFileFilter( const std::string& filter );
    void SetFileFilter( const QString& filter );
    std::string GetPath() const;

protected:
    VFileSelector(
        QWidget* parent,
        const std::string& labelText = "Label",
        const std::string& buttonText = "Select",
        const std::string& filePath = QDir::homePath().toStdString(),
        QFileDialog::FileMode fileMode = QFileDialog::FileMode::ExistingFile
    );
    
    QFileDialog::FileMode _fileMode;
    QFileDialog* _fileDialog;

private slots:
    void _openFileDialog();
    void _setPathFromLineEdit();

signals:
    void _pathChanged();

private:
    QLineEdit*   _lineEdit;
    std::string  _filePath;

    virtual bool _isFileOperable( const std::string& filePath ) const = 0;
};


//
// ====================================
//
class VFileReader : public VFileSelector
{
    Q_OBJECT

public:
    VFileReader(
        QWidget* parent,
        const std::string& labelText = "Label",
        const std::string& buttonText = "Select",
        const std::string& filePath = QDir::homePath().toStdString()
    );

private:
    virtual bool _isFileOperable( const std::string& filePath ) const;
};


//
// ====================================
//
class VFileWriter : public VFileSelector
{
    Q_OBJECT

public:
    VFileWriter(
        QWidget* parent,
        const std::string& labelText = "Label",
        const std::string& buttonText = "Select",
        const std::string& filePath = QDir::homePath().toStdString()
    );

private:
    virtual bool _isFileOperable( const std::string& filePath ) const;
};

//
// ====================================
//
class VTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    VTabWidget(
        QWidget* parent,
        const std::string& firstTabName
    );

    void AddTab(
        const std::string& tabName
    );

    void DeleteTab(
        int index
    );

    void AddWidget( 
        QWidget* widget,
        int index = 0
    );
};
*/
#endif // VAPORWIDGETS_H
