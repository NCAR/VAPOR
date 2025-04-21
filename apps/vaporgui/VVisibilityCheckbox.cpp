#include "VVisibilityCheckbox.h"
#include <vapor/ResourcePath.h>
#include <QVariant>
#include "mac_helpers.h"

VVisibilityCheckbox::VVisibilityCheckbox()
{
    _isWhite = true;
    setBlack();
}

void VVisibilityCheckbox::paintEvent(QPaintEvent *e)
{
#ifdef __APPLE__
    if (MacIsDarkMode())
        setWhite();
    else
        setBlack();
#endif
    
    VCheckBox::paintEvent(e);
}

void VVisibilityCheckbox::setColor(std::string color)
{
    string eye = Wasp::GetSharePath("images/eye");
    if (eye.empty())
        return;
    
    setStyleSheet(QString(
R"VV(
QCheckBox::indicator:checked
{
image: url(%1);
}
QCheckBox::indicator:unchecked
{
image: url(%2);
}
)VV")
    .arg(QString::fromStdString(eye + "/eye_on_" + color + ".png"))
    .arg(QString::fromStdString(eye + "/eye_off_" + color + ".png"))
);
}

void VVisibilityCheckbox::setBlack()
{
    if (_isWhite) {
        _isWhite = false;
        setColor("black");
    }
}

void VVisibilityCheckbox::setWhite()
{
    if (!_isWhite) {
        _isWhite = true;
        setColor("white");
    }
}
