#include "VSection.h"
#include <QStylePainter>
#include <QStyleOption>
#include <vapor/ResourcePath.h>

VSection::VSection(const std::string &title)
{
    QTabWidget::addTab(new QWidget, QString::fromStdString(title));
    _tab()->setLayout(new QVBoxLayout);
    layout()->setMargin(12);
    setStyleSheet(_createStylesheet());
}

QVBoxLayout *VSection::layout() const { return (QVBoxLayout *)_tab()->layout(); }

void VSection::setMenu(QMenu *menu)
{
    SettingsMenuButton *menuButton = (SettingsMenuButton *)QTabWidget::cornerWidget();

    if (!menuButton) {
        menuButton = new SettingsMenuButton;
        QTabWidget::setCornerWidget(menuButton);
    }

    menuButton->setMenu(menu);
}

QWidget *VSection::_tab() const { return QTabWidget::widget(0); }

QString VSection::_createStylesheet() const
{
    std::string stylesheet;

#if defined(Darwin)
    stylesheet +=
        R"(
    QTabWidget::right-corner {
    top: 24px;
    right: 3px;
    }
    )";
#else
    stylesheet +=
        R"(
    QTabWidget::right-corner {
    top: -3px;
    right: 5px;
    }
    )";
#endif

    return QString::fromStdString(stylesheet);
}

VSection::SettingsMenuButton::SettingsMenuButton()
{
    setIcon(QIcon(QString::fromStdString(Wasp::GetSharePath("images/gear-dropdown1.png"))));
    setIconSize(QSize(18, 18));
    setCursor(QCursor(Qt::PointingHandCursor));
    setPopupMode(QToolButton::InstantPopup);

    setStyleSheet("border: none;"
                  "background-color: none;"
                  "padding: 0px;");
}

void VSection::SettingsMenuButton::paintEvent(QPaintEvent *event)
{
    // This function is overridden to prevent Qt from drawing its own dropdown arrow
    QStylePainter p(this);

    QStyleOptionToolButton option;
    initStyleOption(&option);
    option.subControls = QStyle::SC_ToolButton;
    option.features = QStyleOptionToolButton::None;
    p.drawComplexControl(QStyle::CC_ToolButton, option);
}
