#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#include <vapor/glutil.h>
#include <vapor/ImageParams.h>
#include <qlineedit.h>
#include <QFileDialog>
#include <ImageEventRouter.h>

using namespace VAPoR;

ImageEventRouter::ImageEventRouter(QWidget *parent, ControlExec *ce)
    : QTabWidget(parent),
      RenderEventRouter(ce, ImageParams::GetClassType()) {
}

ImageEventRouter::~ImageEventRouter() {
}

void ImageEventRouter::GetWebHelp(vector<pair<string, string>> &help) const {
    help.clear();
}

void ImageEventRouter::_updateTab() {
}
