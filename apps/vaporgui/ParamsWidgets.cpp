#include "ParamsWidgets.h"
#include "VaporWidgets.h"

#include <QVBoxLayout>

ParamsWidget::ParamsWidget( 
    QWidget* parent,
    const std::string& tag,
    const std::string& description
) : 
    QWidget( parent ),
    _params( nullptr ),
    _vaporWidget( nullptr ),
    _tag( tag ),
    _description( description )
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

void ParamsWidget::GetValue( int& value ) const {
    value = -1;
}

void ParamsWidget::GetValue( double& value ) const {
    value = -1.f;
}

void ParamsWidget::GetValue( std::string& value ) const {
    value = "";
}

void ParamsWidget::GetValue( std::vector<double>& value ) const {
    std::vector<double> rvalue = {};
    value = rvalue;
}

PSpinBox::PSpinBox(
    QWidget* parent,
    const std::string& tag,
    const std::string& description,
    const std::string& label,
    int min,
    int max,
    int val
) :
    ParamsWidget(
        parent,
        tag,
        description
    )
{
    _vaporWidget = new VSpinBox( parent, label, min, max, val );
    layout()->addWidget( _vaporWidget );

    connect( _vaporWidget, SIGNAL( _valueChanged() ),
        this, SLOT( _updateParams() ) );
}

void PSpinBox::Update( VAPoR::ParamsBase* params ) {
    VAssert( params != nullptr );
    _params = params;

    int value;
    _vaporWidget->GetValue( value );
    
    value = (int)_params->GetValueDouble( _tag, 0 );
    _vaporWidget->Update( value );
}

void PSpinBox::GetValue( int& value ) const {
    _vaporWidget->GetValue( value );
}

void PSpinBox::_updateParams() {
    int value;
    _vaporWidget->GetValue( value );
    _params->SetValueDouble( _tag, _description, value );
    emit _valueChanged();
}

PSlider::PSlider(
    QWidget* parent,
    const std::string& tag,
    const std::string& description,
    const std::string& label,
    double min,
    double max,
    double val
) :
    ParamsWidget(
        parent,
        tag,
        description
    )
{
    _vaporWidget = new VSlider( parent, label, min, max, val );
    layout()->addWidget( _vaporWidget );

    connect( _vaporWidget, SIGNAL( _valueChanged() ),
        this, SLOT( _updateParams() ) );
}

void PSlider::Update( VAPoR::ParamsBase* params ) {
    VAssert( params != nullptr );
    _params = params;

    double value = _params->GetValueDouble( _tag, 0.f );
    _vaporWidget->Update( value );
}

void PSlider::GetValue( double &value ) const {
    _vaporWidget->GetValue( value );
}

void PSlider::_updateParams() {
    double value;
    _vaporWidget->GetValue( value );
    _params->SetValueDouble( _tag, _description, value );
    emit _valueChanged();
}

PRange::PRange(
    QWidget* parent,
    const std::string& tag,
    const std::string& description,
    double min,
    double max,
    const std::string& minLabel,
    const std::string& maxLabel
) :
    ParamsWidget(
        parent,
        tag,
        description
    )
{ 
    _vaporWidget = new VRange( parent, min, max, minLabel, maxLabel );
    layout()->addWidget( _vaporWidget );

    connect( _vaporWidget, SIGNAL( _valueChanged() ),
        this, SLOT( _updateParams() ) ) ;
}

void PRange::Update( VAPoR::ParamsBase* params ) {
    VAssert( params != nullptr );
    _params = params;

    std::vector<double> value = _params->GetValueDoubleVec( _tag );
    _vaporWidget->Update( value );
}

void PRange::GetValue( std::vector<double>& value ) const {
    _vaporWidget->GetValue( value );
}

void PRange::_updateParams() {
    std::vector<double> value;
    _vaporWidget->GetValue( value );
    _params->SetValueDoubleVec( _tag, _description, value );
    emit _valueChanged();
}

PGeometry::PGeometry(
    QWidget* parent,
    const std::string& tag,
    const std::string& description,
    std::vector<double>& range,
    std::vector<std::string>& labels
) :
    ParamsWidget(
        parent,
        tag,
        description
    )
{
    VAssert( range.size() == labels.size() );

    _vaporWidget = new VGeometry( parent, range, labels );
    layout()->addWidget( _vaporWidget );

    connect( _vaporWidget, SIGNAL( _valueChanged() ),
        this, SLOT( _updateParams() ) );
}

void PGeometry::Update( VAPoR::ParamsBase* params ) {
    VAssert( params != nullptr );
    _params = params;

    std::vector<double> values = _params->GetValueDoubleVec( _tag );
    _vaporWidget->Update( values );
}

void PGeometry::GetValue( std::vector<double>& values ) const {
    _vaporWidget->GetValue( values );
}

void PGeometry::_updateParams() {
    std::vector<double> values;
    _vaporWidget->GetValue( values );
    _params->SetValueDoubleVec( _tag, _description, values );
    emit _valueChanged();
}
