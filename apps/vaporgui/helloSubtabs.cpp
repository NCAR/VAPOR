#include <vapor/HelloParams.h>
#include "helloSubtabs.h"
#include "Flags.h"

using namespace VAPoR;

HelloVariablesSubtab::HelloVariablesSubtab(QWidget *parent) : QWidget(parent), Ui_HelloVariablesGUI()
{
    setupUi(this);

    _variablesWidget->Reinit((VariableFlags)(SCALAR | HEIGHT), (DimFlags)(THREED));
}
