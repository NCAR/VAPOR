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

void VSection::setExpandedSection()
{
    ExpandSectionButton *expandSectionButton = (ExpandSectionButton *)QTabWidget::cornerWidget();
    if (!expandSectionButton) {
        expandSectionButton = new ExpandSectionButton;
        QTabWidget::setCornerWidget(expandSectionButton, Qt::TopLeftCorner);
    }
    connect(expandSectionButton, &QToolButton::clicked, this, [this](){ emit this->expandButtonClicked(); });
}

std::string VSection::getTitle() const {
    return QTabWidget::tabText(0).toStdString();
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
    stylesheet +=
        R"(
    QTabWidget::left-corner {
    top: 24px;
    left: 3px;
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
    stylesheet +=
        R"(
    QTabWidget::left-corner {
    top: -3px;
    right: 5px;
    }
    )";
#endif

    return QString::fromStdString(stylesheet);
}

VSection::SettingsMenuButton::SettingsMenuButton() : AbstractButton()
{
    setIcon(QIcon(QString::fromStdString(Wasp::GetSharePath("images/gear-dropdown1.png"))));
    configureButton();
}

void VSection::AbstractButton::paintEvent(QPaintEvent *event)
{
    // This function is overridden to prevent Qt from drawing its own dropdown arrow
    QStylePainter p(this);

    QStyleOptionToolButton option;
    initStyleOption(&option);
    option.subControls = QStyle::SC_ToolButton;
    option.features = QStyleOptionToolButton::None;
    p.drawComplexControl(QStyle::CC_ToolButton, option);
}

void VSection::AbstractButton::configureButton() {
    setIconSize(QSize(18, 18));
    setCursor(QCursor(Qt::PointingHandCursor));
    setPopupMode(QToolButton::InstantPopup);

    setStyleSheet("border: none;"
                  "background-color: none;"
                  "padding: 0px;");
}
VSection::ExpandSectionButton::ExpandSectionButton() : AbstractButton()
{
    setIcon(QIcon(QString::fromStdString(Wasp::GetSharePath("images/expandSection.png"))));
    configureButton();
}

