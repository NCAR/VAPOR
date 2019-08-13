#include "ParamsWidgets.h"
#include "VaporWidgets.h"

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
{}

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

    connect( _vaporWidget, SIGNAL( _emitValueChanged( int value ) ),
        this, SLOT( _updateParams( int value ) ) );
}

void PSpinBox::Update( VAPoR::ParamsBase* params ) {
    VAssert( params != nullptr );
    _params = params;

    int value = (int)_params->GetValueDouble( _tag, 0 );
    //_vaporWidget->Update<int>( value );
}

void PSpinBox::_updateParams() {
    int value = _vaporWidget->GetValue();
    cout << "SpinBox Value " << value << endl;
    //_params->SetValueDouble( _tag, _description, value );
}
