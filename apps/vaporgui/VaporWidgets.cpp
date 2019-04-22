#include <QFileDialog>

#include "VaporWidgets.h"
#include "FileOperationChecker.h"
#include "ErrorReporter.h"

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

#include <iostream>
#include <cassert>

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

VSpinBox::VSpinBox(
        QWidget *parent,
        const std::string& labelText,
        int defaultValue
    ) :
    VaporWidget(parent, labelText)
{
    _spinBox = new QSpinBox( this );
    _layout->addWidget( _spinBox );

    SetLabelText( QString::fromStdString( labelText ) );
    SetValue( defaultValue );

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( _changed() ) );
}

void VSpinBox::_changed(){
    double newValue = _spinBox->value();
    if ( newValue != _value ) {
        _value = newValue;
        emit _valueChanged();
    }
}

void VSpinBox::SetMaximum( int maximum ) {
    _spinBox->setMaximum( maximum );
}

void VSpinBox::SetMinimum( int minimum ) {
    _spinBox->setMinimum( minimum );
}

void VSpinBox::SetValue( int value ) {
    _spinBox->setValue( value );
}

int VSpinBox::GetValue() const {
    return _value;
}

VDoubleSpinBox::VDoubleSpinBox(
        QWidget *parent,
        const std::string& labelText,
        double defaultValue
    ) :
    VaporWidget(parent, labelText)
{
    _spinBox = new QDoubleSpinBox( this );
    _layout->addWidget( _spinBox );

    SetLabelText( QString::fromStdString( labelText ) );
    SetValue( defaultValue );

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( _changed() ) );
}

void VDoubleSpinBox::_changed() {
    double newValue = _spinBox->value();
    if ( newValue != _value ) {
        _value = newValue;
        emit _valueChanged();
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

VLineEdit::VLineEdit(
        QWidget *parent,
        const std::string& labelText,
        const std::string& editText
    ) :
    VaporWidget(parent, labelText)
{
    _text = editText;

    _edit = new QLineEdit( this );
    _layout->addWidget( _edit );

    SetLabelText( QString::fromStdString( labelText ) );
    SetEditText( QString::fromStdString( editText ) );

    connect( _edit, SIGNAL( returnPressed() ),
        this, SLOT( _returnPressed() ) );
}

void VLineEdit::SetValidator( const QValidator* v ) {
    _edit->setValidator( v );
}

void VLineEdit::SetEditText( const std::string& text )
{
    SetEditText(QString::fromStdString( text ) );
}

void VLineEdit::SetEditText( const QString& text )
{
    _edit->setText( text );
}

std::string VLineEdit::GetEditText() const {
    std::string text = _edit->text().toStdString();
    return text;
}

void VLineEdit::_returnPressed() {
    const QValidator* validator = _edit->validator();
    if ( validator != nullptr ) {
        QString text = _edit->text();
        int i=0;
        const QValidator::State state = validator->validate( text, i );
        if ( state != QValidator::Acceptable )
            _edit->setText( QString::fromStdString( _text ) );
        else
            _text = _edit->text().toStdString();
    }

    std::cout << _text << std::endl;

    emit _editingFinished();
}

VPushButton::VPushButton(
        QWidget *parent,
        const std::string& labelText,
        const std::string& buttonText
    ) :
    VaporWidget(parent, labelText)
{
    _button = new QPushButton( this );
    _layout->addWidget( _button );

    SetLabelText( QString::fromStdString( labelText ) );
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
    VaporWidget(parent, labelText)
{
    _combo = new QComboBox(this);
    _layout->addWidget( _combo );

    connect( _combo, SIGNAL( currentIndexChanged(int) ),
        this, SLOT( _userIndexChanged(int) ) );
}

void VComboBox::_userIndexChanged(int index) {
    emit _indexChanged(index);
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
    VaporWidget(parent, labelText)
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
        const std::string& filePath,
        QFileDialog::FileMode fileMode
    ) :
    VPushButton(parent, labelText, "Select")
{
    _fileMode = fileMode;

    _lineEdit = new QLineEdit(this);
    _layout->addWidget( _lineEdit );
    
    SetLabelText( labelText );
    _filePath = filePath;
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

void VFileSelector::_openFileDialog() {
    QString title = "Select file containing seed points";
    QFileDialog fileDialog(
        this,
        title,
        QString::fromStdString( GetPath() )
    );

    QFileDialog::AcceptMode acceptMode = QFileDialog::AcceptOpen;
    fileDialog.setAcceptMode( acceptMode );

    fileDialog.setFileMode( _fileMode );

    if (fileDialog.exec() != QDialog::Accepted) {
        _button->setDown(false);
        return;
    }

    QStringList files = fileDialog.selectedFiles();
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
        const std::string& filePath
    ) : 
    VFileSelector(
        parent,
        labelText,
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
        const std::string& filePath
    ) : 
    VFileSelector(
        parent,
        labelText,
        filePath
    ) 
{}

bool VFileWriter::_isFileOperable( const std::string& filePath ) const {
    bool operable = false;
    if ( _fileMode == QFileDialog::FileMode::ExistingFile ) {
        operable = FileOperationChecker::FileGoodToWrite(
            QString::fromStdString( filePath ) );
    }

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
}
