#include <QFileDialog>

#include "VaporWidgets.h"
#include "FileOperationChecker.h"
#include "ErrorReporter.h"

#include <QSlider>
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLineEdit>
#include <QValidator>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QPushButton>

#include <iostream>
#include "vapor/VAssert.h"

VaporWidget::VaporWidget(
        QWidget* parent,
        const std::string& labelText
    ) :
    QWidget( parent )
{
    _layout = new QHBoxLayout(this);
    _layout->setContentsMargins( 10, 0, 10, 0);

    _label = new QLabel(this);
    _spacer = new QSpacerItem(
        10, 
        10, 
        QSizePolicy::MinimumExpanding, 
        QSizePolicy::Minimum
    );

    _layout->addWidget( _label );
    _layout->addItem( _spacer );  // sets _spacer's parent to _layout

    SetLabelText( labelText );
    
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

VaporWidget::VaporWidget(
        QWidget* parent,
        const QString& labelText
    ) : VaporWidget( parent, labelText.toStdString() )
{}

void VaporWidget::SetLabelText( const std::string& text )
{
    _label->setText( QString::fromStdString( text ) );
}

void VaporWidget::SetLabelText( const QString& text )
{
    _label->setText( text );
}

VSpinBoxOld::VSpinBoxOld(
        QWidget *parent,
        const std::string& labelText,
        int defaultValue
    ) :
    VaporWidget(parent, labelText), 
    _value( defaultValue )
{
    _spinBox = new QSpinBox( this );
    _layout->addWidget( _spinBox );

    SetValue( defaultValue );

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( _changed() ) );
}

void VSpinBoxOld::_changed(){
    double newValue = _spinBox->value();
    if ( newValue != _value ) {
        _value = newValue;
        emit _valueChanged();
    }
}

void VSpinBoxOld::SetMaximum( int maximum ) {
    _spinBox->setMaximum( maximum );
}

void VSpinBoxOld::SetMinimum( int minimum ) {
    _spinBox->setMinimum( minimum );
}

void VSpinBoxOld::SetValue( int value ) {
    _spinBox->setValue( value );
}

int VSpinBoxOld::GetValue() const {
    return _value;
}

VDoubleSpinBoxOld::VDoubleSpinBoxOld(
        QWidget *parent,
        const std::string& labelText,
        double defaultValue
    ) :
    VaporWidget(parent, labelText),
    _value( defaultValue )
{
    _spinBox = new QDoubleSpinBox( this );
    _layout->addWidget( _spinBox );

    SetValue( defaultValue );

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( _changed() ) );
}

void VDoubleSpinBoxOld::_changed() {
    double newValue = _spinBox->value();
    if ( newValue != _value ) {
        _value = newValue;
        emit _valueChanged();
    }
}

void VDoubleSpinBoxOld::SetMaximum( double maximum ) {
    _spinBox->setMaximum( maximum );
}

void VDoubleSpinBoxOld::SetMinimum( double minimum ) {
    _spinBox->setMinimum( minimum );
}

void VDoubleSpinBoxOld::SetValue( double value ) {
    _spinBox->setValue( value );
}

void VDoubleSpinBoxOld::SetDecimals( int decimals ) {
    _spinBox->setDecimals( decimals );
}

double VDoubleSpinBoxOld::GetValue() const {
    return _value;
}

//
// ====================================
//
VRange::VRange( QWidget* parent, float min, float max, const std::string& minLabel, 
                const std::string& maxLabel ) 
      : QWidget( parent )
{
    _layout    = new QVBoxLayout(this);

    _minSlider = new VSliderOld( this, minLabel, min, max );
    _maxSlider = new VSliderOld( this, maxLabel, min, max );
    connect( _minSlider, SIGNAL( _valueChanged() ), this, SLOT( _respondMinSlider() ) );
    connect( _maxSlider, SIGNAL( _valueChanged() ), this, SLOT( _respondMaxSlider() ) );

    _layout->addWidget( _minSlider );
    _layout->addWidget( _maxSlider );
}

VRange::~VRange() {}

void
VRange::SetRange( float min, float max )
{
    VAssert( max >= min );
    _minSlider->SetRange( min, max );
    _maxSlider->SetRange( min, max );
}

void
VRange::SetCurrentValLow( float low )
{
    /* _minSlider will only respond if low is within a valid range. */
    _minSlider->SetCurrentValue( low );
    _adjustMaxToMin();    
}

void
VRange::SetCurrentValHigh( float high )
{
    /* _maxSlider will only respond if high is within a valid range. */
    _maxSlider->SetCurrentValue( high );
    _adjustMinToMax();    
}

void
VRange::GetCurrentValRange( float& low, float& high ) const
{
    low  = _minSlider->GetCurrentValue();
    high = _maxSlider->GetCurrentValue();
}

void
VRange::_adjustMaxToMin()
{
    float low  = _minSlider->GetCurrentValue();
    float high = _maxSlider->GetCurrentValue();
    if( low > high )
        _maxSlider->SetCurrentValue( low );
}

void
VRange::_adjustMinToMax()
{
    float high = _maxSlider->GetCurrentValue();
    float min  = _minSlider->GetCurrentValue();
    if( high < min  )
        _minSlider->SetCurrentValue( high );
}

void
VRange::_respondMinSlider()
{
    _adjustMaxToMin();
    emit _rangeChanged();
}

void
VRange::_respondMaxSlider()
{
    _adjustMinToMax();
    emit _rangeChanged();
}

//
// ====================================
//
VSliderOld::VSliderOld( QWidget* parent, const std::string& label, float min, float max )
       : VaporWidget( parent, label )
{
    _min = min;
    _max = max;
    VAssert( _max > _min );
    _currentVal = (_min + _max) / 2.0f;
    
    _qslider = new QSlider( this );
    _qslider->setOrientation( Qt::Horizontal );
    /* QSlider will always have its range in integers from 0 to 100. */
    _qslider->setMinimum( 0 );
    _qslider->setMaximum( 100 );
    connect( _qslider, SIGNAL( sliderReleased() ), this, SLOT( _respondQSliderReleased() ) );
    connect( _qslider, SIGNAL( sliderMoved(int) ), this, SLOT( _respondQSliderMoved(int) ) );
    _layout->addWidget( _qslider );
        
    _qedit = new QLineEdit( this );
    connect( _qedit, SIGNAL( editingFinished() ), this, SLOT( _respondQLineEdit() ) );
    _layout->addWidget( _qedit );

    /* update widget display */
    float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
    _qslider->setValue( std::lround( percent ) );
    _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
}

VSliderOld::~VSliderOld() {}

void
VSliderOld::SetRange( float min, float max )
{
    VAssert( min < max );
    _min = min;
    _max = max;

    /* keep the old _currentVal if it's still within the range.
       Otherwise, re-assign the max or min value to _currentVal */
    if( _currentVal < min )
    {
        _currentVal = min;
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    }
    else if( _currentVal > max )
    {
        _currentVal = max;
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    }

    /* update the slider position based on new range. */
    float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
    _qslider->setValue( std::lround( percent ) );
}

void
VSliderOld::SetCurrentValue( float val )
{
    /* Only respond if val is within range */
    if( val >= _min && val <= _max )
    {
        _currentVal   = val;
        float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
        _qslider->setValue( std::lround( percent ) );
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    }
}

float
VSliderOld::GetCurrentValue() const
{
    return _currentVal;
}

void
VSliderOld::_respondQSliderReleased()
{
    /* QSlider is always giving a valid value, so no need to validate range */
    int newvalI    = _qslider->value();
    float percent  = (float)newvalI / 100.0f;
    _currentVal = _min + percent * (_max - _min);
    _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    
    emit _valueChanged();
}

void
VSliderOld::_respondQSliderMoved( int newPos )
{
    /* QSlider is always at a valid position, so no need to validate range */
    float percent   = (float)newPos / 100.0f;
    float tmpVal    = _min + percent * (_max - _min);
    _qedit->setText( QString::number( tmpVal, 'f', 3 ) );
}

void
VSliderOld::_respondQLineEdit()
{
    std::string newtext = _qedit->text().toStdString();
    float   newval;

    try
    {
        newval = std::stof( newtext );
    }
    catch ( const std::invalid_argument& e )
    {
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
        return;
    }

    /* Now validate the input is within range */
    if( newval < _min || newval > _max )
    {
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
        return;
    }

    /* Now update _currentVal, _qslider, and emit signal */
    _currentVal   = newval;
    float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
    _qslider->setValue( std::lround( percent ) );

    emit _valueChanged();
}

//
// ====================================
//
VIntSliderOld::VIntSliderOld( QWidget* parent, const std::string& label, int min, int max )
          : VaporWidget( parent, label )
{
    VAssert( min <= max );
    
    _qslider = new QSlider( this );
    _qslider->setOrientation( Qt::Horizontal );
    /* QSlider will have its range in integers from min to max. */
    _qslider->setMinimum( min );
    _qslider->setMaximum( max );
    connect( _qslider, SIGNAL( sliderReleased() ), this, SLOT( _respondQSliderReleased() ) );
    connect( _qslider, SIGNAL( sliderMoved(int) ), this, SLOT( _respondQSliderMoved(int) ) );
    _layout->addWidget( _qslider );
        
    _qedit = new QLineEdit( this );
    connect( _qedit, SIGNAL( editingFinished() ), this, SLOT( _respondQLineEdit() ) );
    _layout->addWidget( _qedit );

    /* update widget display, displaying the middle between min and max */
    int mid = (min + max) / 2;
    _qslider->setValue( mid );
    _qedit->setText( QString::number( mid ) );
}

VIntSliderOld::~VIntSliderOld() {}

void
VIntSliderOld::SetRange( int newMin, int newMax )
{
    VAssert( newMin <= newMax );

    // Directly give newMin and newMax to the slider.
    // It depends on QT what value to hold after these setters.
    _qslider->setMinimum( newMin );
    _qslider->setMaximum( newMax );

    // Query the current value in _qslider and assign to _qedit
    _qedit->setText( QString::number( _qslider->value() ) );
}

void
VIntSliderOld::SetCurrentValue( int val )
{
    /* Only respond if val is within range */
    if( val >= _qslider->minimum() && val <= _qslider->maximum() )
    {
        _qslider->setValue( val );
        _qedit->setText( QString::number( val ) );
    }
}

int
VIntSliderOld::GetCurrentValue() const
{
    return _qslider->value();
}

void
VIntSliderOld::_respondQSliderReleased()
{
    /* QSlider is always giving a valid value, so no need to validate range */
    int newval     = _qslider->value();
    _qedit->setText( QString::number( newval ) );
    
    emit _valueChanged( newval );
}

void
VIntSliderOld::_respondQSliderMoved( int newPos )
{
    /* QSlider is always at a valid position, so no need to validate range */
    _qedit->setText( QString::number( newPos ) );
}

void
VIntSliderOld::_respondQLineEdit()
{
    std::string newtext = _qedit->text().toStdString();
    int  newval;

    try
    {
        newval = std::stoi( newtext );
    }
    catch ( const std::invalid_argument& e )
    {
        _qedit->setText( QString::number( _qslider->value() ));
        return;
    }

    /* Now validate the input is within range */
    if( newval < _qslider->minimum() || newval > _qslider->maximum() )
    {
        _qedit->setText( QString::number( _qslider->value() ));
        return;
    }

    /* Now update _qslider, and emit signal */
    _qslider->setValue( newval );

    emit _valueChanged( newval );
}


//
// ====================================
//

VGeometry::VGeometry( QWidget* parent, int dim, const std::vector<float>& range )
         : QWidget( parent )
{
    VAssert( dim == 2 || dim == 3 );
    VAssert( range.size() == dim * 2 );
    for( int i = 0; i < dim; i++ )
        VAssert( range[ i*2 ] < range[ i*2+1 ] );

    _dim = dim;
    _xrange = new VRange( this, range[0], range[1], "XMin", "XMax" );
    _yrange = new VRange( this, range[2], range[3], "YMin", "YMax" );
    if( _dim == 3 )
        _zrange = new VRange( this, range[4], range[5], "ZMin", "ZMax" );
    else    // Create anyway. Will be hidden though.
    {
        _zrange = new VRange( this, 0.0f, 100.0f, "ZMin", "ZMax" );
        _zrange->hide(); 
    }

    connect( _xrange, SIGNAL( _rangeChanged() ), this, SLOT( _respondChanges() ) );
    connect( _yrange, SIGNAL( _rangeChanged() ), this, SLOT( _respondChanges() ) );
    connect( _zrange, SIGNAL( _rangeChanged() ), this, SLOT( _respondChanges() ) );

    _layout = new QVBoxLayout(this);
    _layout->addWidget( _xrange );
    _layout->addWidget( _yrange );
    _layout->addWidget( _zrange );
}

VGeometry::~VGeometry() {}

void
VGeometry::SetDimAndRange( int dim, const std::vector<float>& range )
{
    VAssert( dim == 2 || dim == 3 );
    VAssert( range.size() == dim * 2 );
    for( int i = 0; i < dim; i++ )
        VAssert( range[ i*2 ] < range[ i*2+1 ] );

    /* Adjust the appearance if necessary */
    if( _dim == 2 && dim == 3 )
        _zrange->show();
    else if( _dim == 3 && dim == 2 )
        _zrange->hide();
    _dim = dim;

    _xrange->SetRange( range[0], range[1] );
    _yrange->SetRange( range[2], range[3] );
    if( _dim == 3 )
        _zrange->SetRange( range[4], range[5] );
}

void
VGeometry::SetCurrentValues( const std::vector<float>& vals )
{
    VAssert( vals.size() == _dim * 2 );

    /* VRange widgets will adjust itself if new values violate
       range constraints, i.e., low > high.                   */
    /* VRange widgets will only respond to values within their ranges */
    _xrange->SetCurrentValLow(  vals[0] );
    _xrange->SetCurrentValHigh( vals[1] );
    _yrange->SetCurrentValLow(  vals[2] );
    _yrange->SetCurrentValHigh( vals[3] );
    if( _dim == 3 )
    {
        _zrange->SetCurrentValLow(  vals[4] );
        _zrange->SetCurrentValHigh( vals[5] );
    }
}

void
VGeometry::GetCurrentValues( std::vector<float>& vals ) const
{
    vals.resize( _dim * 2, 0.0f );
    _xrange->GetCurrentValRange( vals[0], vals[1] );
    _yrange->GetCurrentValRange( vals[2], vals[3] );
    if( _dim == 3 )
        _zrange->GetCurrentValRange( vals[4], vals[5] );
}

void
VGeometry::_respondChanges()
{
    emit _geometryChanged();
}

//
// ====================================
//
VLineEditOld::VLineEditOld(
        QWidget *parent,
        const std::string& labelText,
        const std::string& editText
    ) :
    VaporWidget(parent, labelText)
{
    _text = editText;

    _edit = new QLineEdit( this );
    _layout->addWidget( _edit );

    SetEditText( QString::fromStdString( editText ) );

    connect( _edit, SIGNAL( editingFinished() ),
        this, SLOT( _relaySignal() ) );
}

VLineEditOld::~VLineEditOld() { }

void VLineEditOld::SetEditText( const std::string& text )
{
    SetEditText(QString::fromStdString( text ) );
}

void VLineEditOld::SetEditText( const QString& text )
{
    _edit->setText( text );
    _text = _edit->text().toStdString();
}

std::string VLineEditOld::GetEditText() const {
    return _text;
}

void VLineEditOld::_relaySignal() {
    QString text = _edit->text();
    _edit->setText( text );
    _text = text.toStdString();    

    emit _editingFinished();
}

//
// ====================================
//

VPushButtonOld::VPushButtonOld(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText
    ) :
    VaporWidget(parent, labelText)
{
    _button = new QPushButton( this );
    _layout->addWidget( _button );

    SetButtonText( QString::fromStdString( buttonText ) );

    connect( _button, SIGNAL( pressed() ),
        this, SLOT( _buttonPressed() ) );
}

void VPushButtonOld::SetButtonText( const std::string& text )
{
    SetButtonText(QString::fromStdString( text ) );
}

void VPushButtonOld::SetButtonText( const QString& text )
{
    _button->setText( text );
}

void VPushButtonOld::_buttonPressed() {
    emit _pressed();
}

VComboBoxOld::VComboBoxOld(
        QWidget *parent,
        const std::string& labelText
    ) :
    VaporWidget(parent, labelText)
{
    _combo = new QComboBox(this);
    _layout->addWidget( _combo );

    connect( _combo, SIGNAL( currentIndexChanged(int) ),
        this, SLOT( _userIndexChanged(int) ) );
}

void VComboBoxOld::_userIndexChanged(int index) {
    emit _indexChanged(index);
}

int VComboBoxOld::GetNumOfItems() const
{
    return _combo->count();
}

int VComboBoxOld::GetCurrentIndex() const {
    return _combo->currentIndex();
}

std::string VComboBoxOld::GetCurrentText() const {
    return _combo->currentText().toStdString();
}

std::string VComboBoxOld::GetItemText( int index) const {
    return _combo->itemText(index).toStdString();
}

void VComboBoxOld::AddOption( const std::string& option, int index) {
    _combo->insertItem( index, QString::fromStdString(option) );
}

void VComboBoxOld::RemoveOption( int index=0 ) {
    _combo->removeItem( index ) ;
}

void VComboBoxOld::SetIndex( int index ) {
    _combo->setCurrentIndex( index );
}

VCheckBoxOld::VCheckBoxOld(
        QWidget *parent,
        const std::string& labelText
    ) :
    VaporWidget(parent, labelText)
{
    _checkbox = new QCheckBox( "", this );
    _layout->addWidget( _checkbox );

    _layout->setContentsMargins( 10, 0, 16, 0);

    connect( _checkbox, SIGNAL( stateChanged(int) ),
        this, SLOT( _userClickedCheckbox() ) );
}

bool VCheckBoxOld::GetCheckState() const {
    if ( _checkbox->checkState() == Qt::Checked)
        return true;
    else
        return false;
}

void VCheckBoxOld::SetCheckState( bool checkState ) {
    if ( checkState )
        _checkbox->setCheckState( Qt::Checked );
    else
        _checkbox->setCheckState( Qt::Unchecked );
}

void VCheckBoxOld::_userClickedCheckbox() {
    emit _checkboxClicked();
}

VFileSelectorOld::VFileSelectorOld(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText,
        const std::string& filePath,
        QFileDialog::FileMode fileMode
    ) :
    VPushButtonOld(parent, labelText, buttonText),
    _filePath( filePath )
{
    _lineEdit = new QLineEdit(this);
    _layout->addWidget( _lineEdit );

    QString defaultPath = QString::fromStdString( GetPath() );
    if ( _filePath.empty() )
        defaultPath = QDir::homePath();

    _fileDialog = new QFileDialog(
        this,
        QString::fromStdString( labelText ),
        defaultPath
    );
    
    _fileMode = fileMode;
    _fileDialog->setFileMode( _fileMode );
    
    _lineEdit->setText( QString::fromStdString( filePath ) );
    
    connect( _button, SIGNAL( pressed() ),
        this, SLOT( _openFileDialog() ) );
    connect( _lineEdit, SIGNAL( returnPressed() ),
        this, SLOT( _setPathFromLineEdit() ) );
}

std::string VFileSelectorOld::GetPath() const {
    return _filePath;
}

void VFileSelectorOld::SetPath( const QString& path) {
    SetPath( path.toStdString() );
}

void VFileSelectorOld::SetPath( const std::string& path ) {
    if ( path.empty() )
        return;

    if ( !_isFileOperable( path ) ) {
        MSG_ERR(
            FileOperationChecker::GetLastErrorMessage().toStdString()
        );
        _lineEdit->setText( QString::fromStdString( _filePath ) );
        return;
    }
    _filePath = path;
    _lineEdit->setText( QString::fromStdString(path) );
}

void VFileSelectorOld::SetFileFilter( const QString& filter ) {
    _fileDialog->setNameFilter( filter );
}

void VFileSelectorOld::SetFileFilter( const std::string& filter) {
    _fileDialog->setNameFilter( QString::fromStdString(filter) );
}

void VFileSelectorOld::_openFileDialog() {

    if (_fileDialog->exec() != QDialog::Accepted) {
        _button->setDown(false);
        return;
    }

    QStringList files = _fileDialog->selectedFiles();
    if (files.size() != 1) {
        _button->setDown(false);
        return;
    }

    QString filePath = files[0];

    SetPath( filePath.toStdString() );
    _button->setDown(false);

    emit _pathChanged();
}

void VFileSelectorOld::_setPathFromLineEdit() {
    QString filePath = _lineEdit->text();
    SetPath( filePath.toStdString() );
    emit _pathChanged();
}

VFileReaderOld::VFileReaderOld(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText,
        const std::string& filePath
    ) : 
    VFileSelectorOld(
        parent,
        labelText,
        buttonText,
        filePath,
        QFileDialog::FileMode::ExistingFile
    ) 
{}

bool VFileReaderOld::_isFileOperable( const std::string& filePath ) const {
    bool operable = false;
    if ( _fileMode == QFileDialog::FileMode::ExistingFile ) {
        operable = FileOperationChecker::FileGoodToRead( 
            QString::fromStdString( filePath ) );
    }
    if ( _fileMode == QFileDialog::FileMode::Directory ) {
        operable = FileOperationChecker::DirectoryGoodToRead(
            QString::fromStdString( filePath ) );
    }

    return operable;
}

VFileWriterOld::VFileWriterOld(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText,
        const std::string& filePath
    ) : 
    VFileSelectorOld(
        parent,
        labelText,
        buttonText,
        filePath
    ) 
{
    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptSave;
    _fileDialog->setAcceptMode( acceptMode );
    _fileMode = QFileDialog::AnyFile;
    _fileDialog->setFileMode( _fileMode );
}

bool VFileWriterOld::_isFileOperable( const std::string& filePath ) const {
    bool operable = false;
    QString qFilePath = QString::fromStdString( filePath );
    operable = FileOperationChecker::FileGoodToWrite( qFilePath );
    return operable;
}
