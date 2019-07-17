#ifndef VAPORWIDGETS_H 
#define VAPORWIDGETS_H 

class QWidget;
class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLineEdit;
class QValidator;
class QSpacerItem;
class QHBoxLayout;
class QSpinBox;
class QDoubleSpinBox;

#include <QTabWidget>

//
// ====================================
//
class VaporWidget : public QWidget
{
    Q_OBJECT

public:
    void SetLabelText( const std::string& text );
    void SetLabelText( const QString& text );

protected:
    VaporWidget( 
        QWidget* parent,
        const std::string& labelText
    );
    VaporWidget( 
        QWidget* parent,
        const QString& labelText
    );

    QLabel*      _label;
    QSpacerItem* _spacer;
    QHBoxLayout* _layout;
};

//
// ====================================
//
class VSpinBox : public VaporWidget
{
    Q_OBJECT

public:
    VSpinBox(
        QWidget* parent, 
        const std::string& labelText = "Label",
        int defaultValue = 0 
    );

    void SetMaximum( int maximum );
    void SetMinimum( int minimum );
    void SetValue( int value );
    int GetValue() const;

signals:
    void _valueChanged();

protected:
    QSpinBox* _spinBox;

private slots:
    void _changed();

private:
    int _value;
};

//
// ====================================
//
class VDoubleSpinBox : public VaporWidget
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
    void _valueChanged();

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
class VLineEdit : public VaporWidget
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
    void SetExtents( int min, int max );
    void SetExtents( double min, double max );
    void SetIntType();
    void SetDoubleType();
    void SetValidator( QValidator* v );
    std::string GetEditText() const;

signals:
    void _editingFinished();

protected:
    QLineEdit* _edit;

    // If we assign a validator to the QLineEdit, the QLineEdit will not emit
    // the returnPressed() signal with invalid input.  However we do want this
    // signal to be emitted with invalid input, so we can change it to the
    // previous value.  Therefore, we perform validation within the VLineEdit,
    // not the QLineEdit.
    QValidator* _validator;

private slots:
    void _relaySignal();

private:
    std::string _text;
};

//
// ====================================
//
class VPushButton : public VaporWidget
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
class VComboBox : public VaporWidget
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
class VCheckBox : public VaporWidget
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

#endif // VAPORWIDGETS_H
