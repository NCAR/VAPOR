#pragma once

#include <QTabWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <string>

#include <QToolButton>

class VSection : public QTabWidget {
    Q_OBJECT
    
public:
    VSection(const std::string &title);
    QVBoxLayout *layout() const;
    void setMenu(QMenu *menu);
    
    
    void setLayout(QLayout *layout) = delete;
    int addTab(QWidget *page, const QString &label) = delete;
    QWidget *widget(int index) const = delete;
    void setCornerWidget(QWidget *widget, Qt::Corner corner) = delete;
    QWidget *cornerWidget() const = delete;
    
private:
    
    QWidget *_tab() const;
    QString _createStylesheet() const;
};





class VSectionSettingsMenuButton : public QToolButton {
    Q_OBJECT
    
public:
    VSectionSettingsMenuButton();
    
protected:
    void paintEvent(QPaintEvent* event);
};
