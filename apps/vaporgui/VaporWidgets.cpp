#include "VaporWidgets.h"
#include "FileOperationChecker.h"
#include "ErrorReporter.h"

#include <QLabel>
#include <QSlider>
#include <QWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QValidator>
#include <QPushButton>
#include <QFileDialog>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <iostream>
#include "vapor/VAssert.h"

VaporWidget::VaporWidget(
    QWidget* parent
    ) : 
    QWidget( parent )
{}

template <class T>
void VaporWidget::Update( T value ) {}

//template <class T>
//T GetValue() { return nullptr; }

int VaporWidget::GetValue() { return -1; }

VaporLine::VaporLine(
    QWidget* parent,
    const std::string& labelText
    ) :
    VaporWidget( parent )
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

VaporLine::VaporLine(
        QWidget* parent,
        const QString& labelText
    ) : VaporLine( parent, labelText.toStdString() )
{}

template<>
void VaporLine::Update<const std::string&>( const std::string& labelText ) {
    SetLabelText( labelText );
}

//template<>
//std::string VaporLine::GetValue()< std::string > const {
//    return _label->text().toStdString();
//}

void VaporLine::_validateAndEmit() {};

void VaporLine::SetLabelText( const std::string& text )
{
    _label->setText( QString::fromStdString( text ) );
}

void VaporLine::SetLabelText( const QString& text )
{
    _label->setText( text );
}

VSpinBox::VSpinBox(
        QWidget *parent,
        const std::string& labelText,
        int min,
        int max,
        int defaultValue
    ) :
    VaporLine(parent, labelText), 
    _min( min ),
    _max( max ),
    _value( defaultValue )
{
    _spinBox = new QSpinBox( this );
    SetMinimum( min );
    SetMaximum( max );

    _spinBox->setValue( _value );
    
    _layout->addWidget( _spinBox );

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( _validate() ) );
}

template <>
void VSpinBox::Update<int>( int value ) {
     if ( value != _value &&
          value <= _max   &&
          value >= _min   ) 
    {
        _value = value;
        _spinBox->setValue( value );
    }
}

//template<>
//int VSpinBox::GetValue()< int > const {
//    return _value;
//}

void VSpinBox::_validateAndEmit() {
    double newValue = _spinBox->value();
   
     if ( newValue != _value &&
          newValue <= _max   &&
          newValue >= _min   ) 
    {
        _value = newValue;
        emit _valueChanged();
    }
}

void VSpinBox::SetMaximum( int maximum ) {
    if ( maximum >= _min ) {
        _max = maximum;
        _spinBox->setMaximum( maximum );
    }
}

void VSpinBox::SetMinimum( int minimum ) {
    if ( minimum <= _max ) {
        _min = minimum;
        _spinBox->setMinimum( minimum );
    }
}

/*
VDoubleSpinBox::VDoubleSpinBox(
        QWidget *parent,
        const std::string& labelText,
        double defaultValue
    ) :
    VaporLine(parent, labelText),
    _value( defaultValue )
{
    _spinBox = new QDoubleSpinBox( this );
    _layout->addWidget( _spinBox );

    SetValue( defaultValue );

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( _changed() ) );
}

void VDoubleSpinBox::_changed() {
    double newValue = _spinBox->value();
    if ( newValue != _value ) {
        _value = newValue;
        emit _emitValueChanged();
    }
}

void VDoubleSpinBox::SetMaximum( double maximum ) {
    _spinBox->setMaximum( maximum );
}

void VDoubleSpinBox::SetMinimum( double minimum ) {
    _spinBox->setMinimum( minimum );
}

void VDoubleSpinBox::SetValue( double value ) {
    _spinBox->setValue( value );
}

void VDoubleSpinBox::SetDecimals( int decimals ) {
    _spinBox->setDecimals( decimals );
}

double VDoubleSpinBox::GetValue() const {
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

    _minSlider = new VSlider( this, minLabel, min, max );
    _maxSlider = new VSlider( this, maxLabel, min, max );
    connect( _minSlider, SIGNAL( _emitValueChanged() ), this, SLOT( _respondMinSlider() ) );
    connect( _maxSlider, SIGNAL( _emitValueChanged() ), this, SLOT( _respondMaxSlider() ) );

    _layout->addWidget( _minSlider );
    _layout->addWidget( _maxSlider );
}

VRange::~VRange() {}

void
VRange::SetRange( float min, float max )
{
    VAssert( max > min );
    _minSlider->SetRange( min, max );
    _maxSlider->SetRange( min, max );
}

void
VRange::SetCurrentValLow( float low )
{
    // _minSlider will only respond if low is within a valid range.
    _minSlider->SetCurrentValue( low );
    _adjustMaxToMin();    
}

void
VRange::SetCurrentValHigh( float high )
{
    // _maxSlider will only respond if high is within a valid range.
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
VSlider::VSlider( QWidget* parent, const std::string& label, float min, float max )
       : VaporLine( parent, label )
{
    _min = min;
    _max = max;
    VAssert( _max > _min );
    _currentVal = (_min + _max) / 2.0f;
    
    _qslider = new QSlider( this );
    _qslider->setOrientation( Qt::Horizontal );
    // QSlider will always have its range in integers from 0 to 100.
    _qslider->setMinimum( 0 );
    _qslider->setMaximum( 100 );
    connect( _qslider, SIGNAL( sliderReleased() ), this, SLOT( _respondQSliderReleased() ) );
    connect( _qslider, SIGNAL( sliderMoved(int) ), this, SLOT( _respondQSliderMoved(int) ) );
    _layout->addWidget( _qslider );
        
    _qedit = new QLineEdit( this );
    connect( _qedit, SIGNAL( editingFinished() ), this, SLOT( _respondQLineEdit() ) );
    _layout->addWidget( _qedit );

    // update widget display
    float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
    _qslider->setValue( std::lround( percent ) );
    _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
}

VSlider::~VSlider() {}

void
VSlider::SetRange( float min, float max )
{
    VAssert( min < max );
    _min = min;
    _max = max;

    // keep the old _currentVal if it's still within the range.
    //   Otherwise, re-assign the middle point to _currentVal
    if( _currentVal < min ||  _currentVal > max )
    {
        _currentVal   = (min + max) / 2.0f;
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    }

    // update the slider position based on new range.
    float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
    _qslider->setValue( std::lround( percent ) );
}

void
VSlider::SetCurrentValue( float val )
{
    // Only respond if val is within range
    if( val >= _min && val <= _max )
    {
        _currentVal   = val;
        float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
        _qslider->setValue( std::lround( percent ) );
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    }
}

float
VSlider::GetCurrentValue() const
{
    return _currentVal;
}

void
VSlider::_respondQSliderReleased()
{
    // QSlider is always giving a valid value, so no need to validate range
    int newvalI    = _qslider->value();
    float percent  = (float)newvalI / 100.0f;
    _currentVal = _min + percent * (_max - _min);
    _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
    
    emit _emitValueChanged();
}

void
VSlider::_respondQSliderMoved( int newPos )
{
    // QSlider is always at a valid position, so no need to validate range
    float percent   = (float)newPos / 100.0f;
    float tmpVal    = _min + percent * (_max - _min);
    _qedit->setText( QString::number( tmpVal, 'f', 3 ) );
}

void
VSlider::_respondQLineEdit()
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

    // Now validate the input is within range
    if( newval < _min || newval > _max )
    {
        _qedit->setText( QString::number( _currentVal, 'f', 3 ) );
        return;
    }

    // Now update _currentVal, _qslider, and emit signal
    _currentVal   = newval;
    float percent = (_currentVal - _min) / (_max - _min) * 100.0f;
    _qslider->setValue( std::lround( percent ) );

    emit _emitValueChanged();
}

//
// ====================================
//

VGeometry::VGeometry( 
        QWidget* parent, 
        int dim, 
        const std::vector<float>& range,
        const std::string& label
) : QTabWidget( parent )
{
    VAssert( dim == 2 || dim == 3 );
    VAssert( range.size() == dim * 2 );
    for( int i = 0; i < dim; i++ )
        VAssert( range[ i ] < range[ i+3 ] );

    QWidget* pageWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();
    pageWidget->setLayout( layout );

    _dim = dim;
    _xrange = new VRange( pageWidget, range[0], range[3], "XMin", "XMax" );
    _yrange = new VRange( pageWidget, range[1], range[4], "YMin", "YMax" );
    if( _dim == 3 )
        _zrange = new VRange( pageWidget, range[2], range[5], "ZMin", "ZMax" );
    else    // Create anyway. Will be hidden though.
    {
        _zrange = new VRange( pageWidget, 0.0f, 100.0f, "ZMin", "ZMax" );
        _zrange->hide();
    }

    connect( _xrange, SIGNAL( _rangeChanged() ), this, SLOT( _respondChanges() ) );
    connect( _yrange, SIGNAL( _rangeChanged() ), this, SLOT( _respondChanges() ) );
    connect( _zrange, SIGNAL( _rangeChanged() ), this, SLOT( _respondChanges() ) );

    //_layout = new QVBoxLayout(this);
    layout->addWidget( _xrange );
    layout->addWidget( _yrange );
    layout->addWidget( _zrange );
    addTab( pageWidget, QString::fromStdString(label) );
}

VGeometry::~VGeometry() {}

void
VGeometry::SetDimAndRange( int dim, const std::vector<float>& range )
{
    VAssert( dim == 2 || dim == 3 );
    VAssert( range.size() == dim * 2 );
    for( int i = 0; i < dim; i++ )
        VAssert( range[ i ] < range[ i+3 ] );

    // Adjust the appearance if necessary
    if( _dim == 2 && dim == 3 )
        _zrange->show();
    else if( _dim == 3 && dim == 2 )
        _zrange->hide();
    _dim = dim;

    _xrange->SetRange( range[0], range[3] );
    _yrange->SetRange( range[1], range[4] );
    if( _dim == 3 )
        _zrange->SetRange( range[2], range[5] );
}

void
VGeometry::SetCurrentValues( const std::vector<float>& vals )
{
    VAssert( vals.size() == _dim * 2 );
    for( int i = 0; i < _dim; i++ )
        VAssert( vals[ i ] < vals[ i+3 ] );

    // VRange widgets will only respond to values within their ranges
    _xrange->SetCurrentValLow(  vals[0] );
    _xrange->SetCurrentValHigh( vals[3] );
    _yrange->SetCurrentValLow(  vals[1] );
    _yrange->SetCurrentValHigh( vals[4] );
    if( _dim == 3 )
    {
        _zrange->SetCurrentValLow(  vals[2] );
        _zrange->SetCurrentValHigh( vals[5] );
    }
}

void
VGeometry::GetCurrentValues( std::vector<float>& vals ) const
{
    vals.resize( _dim * 2, 0.0f );
    _xrange->GetCurrentValRange( vals[0], vals[3] );
    _yrange->GetCurrentValRange( vals[1], vals[4] );
    if( _dim == 3 )
        _zrange->GetCurrentValRange( vals[2], vals[5] );
}

void
VGeometry::_respondChanges()
{
    emit _geometryChanged();
}

//
// ====================================
//
VLineEdit::VLineEdit(
        QWidget *parent,
        const std::string& labelText,
        const std::string& editText
    ) :
    VaporLine(parent, labelText)
{
    _text = editText;

    _edit = new QLineEdit( this );
    _layout->addWidget( _edit );

    SetEditText( QString::fromStdString( editText ) );

    connect( _edit, SIGNAL( editingFinished() ),
        this, SLOT( _relaySignal() ) );
}

VLineEdit::~VLineEdit() { }

void VLineEdit::SetEditText( const std::string& text )
{
    SetEditText(QString::fromStdString( text ) );
}

void VLineEdit::SetEditText( const QString& text )
{
    _edit->setText( text );
    _text = _edit->text().toStdString();
}

std::string VLineEdit::GetEditText() const {
    return _text;
}

void VLineEdit::_relaySignal() {
    QString text = _edit->text();
    _edit->setText( text );
    _text = text.toStdString();    

    emit _editingFinished();
}

//
// ====================================
//

VPushButton::VPushButton(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText
    ) :
    VaporLine(parent, labelText)
{
    _button = new QPushButton( this );
    _layout->addWidget( _button );

    SetButtonText( QString::fromStdString( buttonText ) );

    connect( _button, SIGNAL( pressed() ),
        this, SLOT( _buttonPressed() ) );
}

void VPushButton::SetButtonText( const std::string& text )
{
    SetButtonText(QString::fromStdString( text ) );
}

void VPushButton::SetButtonText( const QString& text )
{
    _button->setText( text );
}

void VPushButton::_buttonPressed() {
    emit _pressed();
}

VComboBox::VComboBox(
        QWidget *parent,
        const std::string& labelText
    ) :
    VaporLine(parent, labelText)
{
    _combo = new QComboBox(this);
    _layout->addWidget( _combo );

    connect( _combo, SIGNAL( currentIndexChanged(int) ),
        this, SLOT( _userIndexChanged(int) ) );
}

void VComboBox::_userIndexChanged(int index) {
    emit _indexChanged(index);
}

int VComboBox::GetNumOfItems() const
{
    return _combo->count();
}

int VComboBox::GetCurrentIndex() const {
    return _combo->currentIndex();
}

std::string VComboBox::GetCurrentText() const {
    return _combo->currentText().toStdString();
}

void VComboBox::AddOption( const std::string& option, int index) {
    _combo->insertItem( index, QString::fromStdString(option) );
}

void VComboBox::RemoveOption( int index=0 ) {
    _combo->removeItem( index ) ;
}

void VComboBox::SetIndex( int index ) {
    _combo->setCurrentIndex( index );
}

VCheckBox::VCheckBox(
        QWidget *parent,
        const std::string& labelText
    ) :
    VaporLine(parent, labelText)
{
    _checkbox = new QCheckBox( "", this );
    _layout->addWidget( _checkbox );

    _layout->setContentsMargins( 10, 0, 16, 0);

    connect( _checkbox, SIGNAL( stateChanged(int) ),
        this, SLOT( _userClickedCheckbox() ) );
}

bool VCheckBox::GetCheckState() const {
    if ( _checkbox->checkState() == Qt::Checked)
        return true;
    else
        return false;
}

void VCheckBox::SetCheckState( bool checkState ) {
    if ( checkState )
        _checkbox->setCheckState( Qt::Checked );
    else
        _checkbox->setCheckState( Qt::Unchecked );
}

void VCheckBox::_userClickedCheckbox() {
    emit _checkboxClicked();
}

VFileSelector::VFileSelector(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText,
        const std::string& filePath,
        QFileDialog::FileMode fileMode
    ) :
    VPushButton(parent, labelText, buttonText),
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

std::string VFileSelector::GetPath() const {
    return _filePath;
}

void VFileSelector::SetPath( const QString& path) {
    SetPath( path.toStdString() );
}

void VFileSelector::SetPath( const std::string& path ) {
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

void VFileSelector::SetFileFilter( const QString& filter ) {
    _fileDialog->setNameFilter( filter );
}

void VFileSelector::SetFileFilter( const std::string& filter) {
    _fileDialog->setNameFilter( QString::fromStdString(filter) );
}

void VFileSelector::_openFileDialog() {

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

void VFileSelector::_setPathFromLineEdit() {
    QString filePath = _lineEdit->text();
    SetPath( filePath.toStdString() );
    emit _pathChanged();
}

VFileReader::VFileReader(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText,
        const std::string& filePath
    ) : 
    VFileSelector(
        parent,
        labelText,
        buttonText,
        filePath,
        QFileDialog::FileMode::ExistingFile
    ) 
{}

bool VFileReader::_isFileOperable( const std::string& filePath ) const {
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

VFileWriter::VFileWriter(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText,
        const std::string& filePath
    ) : 
    VFileSelector(
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

bool VFileWriter::_isFileOperable( const std::string& filePath ) const {
    bool operable = false;
    QString qFilePath = QString::fromStdString( filePath );
    operable = FileOperationChecker::FileGoodToWrite( qFilePath );
    return operable;
}

VTabWidget::VTabWidget(
    QWidget* parent,
    const std::string& firstTabName
) : QTabWidget( parent ) 
{
    AddTab( firstTabName );
}

void VTabWidget::AddTab(
    const std::string& tabName
) {
    QWidget* container = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins( 0, 0, 0, 0 );
    container->setLayout(layout);
    
    addTab( container, QString::fromStdString(tabName) );
}

void VTabWidget::DeleteTab(
    int index
) {
    removeTab( index );
}

void VTabWidget::AddWidget(
    QWidget* inputWidget,
    int index
) {
    QWidget* target = widget(index);
    target->layout()->addWidget( inputWidget );
}*/
