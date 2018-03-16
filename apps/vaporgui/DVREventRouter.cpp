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

std::string DVREventRouter::_getDescription() const { return std::string("DVR Renderer"); }

std::string DVREventRouter::_getSmallIconImagePath() const { return std::string("DVR_small.png"); }

std::string DVREventRouter::_getIconImagePath() const { return std::string("DVR.png"); }
