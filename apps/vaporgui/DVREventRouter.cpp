#include "DVREventRouter.h"
#include "ui_DVREventRouter.h"

//
// Register class with object factory!!!
//
static RenderEventRouterRegistrar<DVREventRouter> registrar(DVREventRouter::GetClassType());

DVREventRouter::DVREventRouter(QWidget *parent, VAPoR::ControlExec *ce) : QWidget(parent), ui(new Ui::DVREventRouter), RenderEventRouter(ce, DVREventRouter::GetClassType()) { ui->setupUi(this); }

DVREventRouter::~DVREventRouter() { delete ui; }

void DVREventRouter::_updateTab() {}

void DVREventRouter::GetWebHelp(vector<pair<string, string>> &help) const { help.clear(); }
