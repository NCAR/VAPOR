#include <iostream>
#include "VSection.h"
#include <QStylePainter>
#include <QStyleOption>
#include <QWindow>
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

void VSection::setExpandSection(QWidget* expanded)
{
    //_expandedSection = new ExpandedSection(expanded);
    //_expandedSection = new ExpandedSection(new QLabel("hi"));
    //_expandedSection = new QLabel("hi");
    //_expandedSection->setAttribute(Qt::WA_DeleteOnClose);  // Automatically delete when closed
    //connect(_expandedSection, &QWidget::destroyed, this, &VSection::clearExpandedSection);  // Reset pointer when window closes

    _expandedWidget = expanded;

    ExpandSectionButton *expandSectionButton = (ExpandSectionButton *)QTabWidget::cornerWidget();
    if (!expandSectionButton) {
        expandSectionButton = new ExpandSectionButton;
        QTabWidget::setCornerWidget(expandSectionButton, Qt::TopLeftCorner);
    }
    connect(expandSectionButton, &QToolButton::clicked, this, &VSection::showExpandedSection);
    //connect(expandSectionButton, &QToolButton::clicked, this, [this]() {
    //    if (!_expandedSection) {
    //        //newWindow = new NewWindow(new QLabel("Dynamic Widget Example", this));  // Passing dynamic widget
    //        _expandedSection = new ExpandedSection(_expandedWidget);
    //        _expandedSection->setAttribute(Qt::WA_DeleteOnClose);
    //        connect(_expandedSection, &QWidget::destroyed, this, &VSection::clearExpandedSection);
    //        _expandedSection->show();
    //    } else {
    //        _expandedSection->raise();
    //        _expandedSection->activateWindow();
    //    }
    //});
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

VSection::ExpandSectionButton::ExpandSectionButton()
{
    setIcon(QIcon(QString::fromStdString(Wasp::GetSharePath("images/expand2.png"))));
    setIconSize(QSize(18, 18));
    setCursor(QCursor(Qt::PointingHandCursor));
    setPopupMode(QToolButton::InstantPopup);

    setStyleSheet("border: none;"
                  "background-color: none;"
                  "padding: 0px;");
}

void VSection::ExpandSectionButton::paintEvent(QPaintEvent *event)
{
    std::cout << "ESB click" << std::endl;
    // This function is overridden to prevent Qt from drawing its own dropdown arrow
    QStylePainter p(this);

    QStyleOptionToolButton option;
    initStyleOption(&option);
    option.subControls = QStyle::SC_ToolButton;
    option.features = QStyleOptionToolButton::None;
    p.drawComplexControl(QStyle::CC_ToolButton, option);
}

VSection::ExpandedSection::ExpandedSection(QWidget* expanded) : QWidget() {
    setWindowTitle("New Window");
    QVBoxLayout *layout = new QVBoxLayout(this);
    QLabel *label = new QLabel("This is a new window.", this);
    layout->addWidget(label);
    if(expanded != nullptr) {
        std::cout << "adding " << expanded->metaObject()->className() << std::endl;
        layout->addWidget(expanded);
    }
    setLayout(layout);
}

void VSection::showExpandedSection() {
    std::cout << "  showExpandedSection()" << std::endl;
    if (_expandedSection==nullptr) {
        _expandedSection = new ExpandedSection(_expandedWidget);
        //_expandedSection->setAttribute(Qt::WA_DeleteOnClose);  // Automatically delete when closed
        connect(_expandedSection, &QWidget::destroyed, this, &VSection::clearExpandedSection);  // Reset pointer when window closes
        _expandedSection->show();
    } else {
        _expandedSection->raise();  // Bring the window to the front
        _expandedSection->activateWindow();  // Set focus to the window
    }
}

void VSection::clearExpandedSection() {
    //_expandedSection = nullptr;  // Reset the pointer when the window is destroyed
}

