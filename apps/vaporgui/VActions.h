#pragma once

#include <string>

#include <QWidgetAction>

class VIntSpinBox;
class VCheckBox;
class VStringLineEdit;
class VIntLineEdit;
class VDoubleLineEdit;

class VSpinBoxAction : public QWidgetAction {
    Q_OBJECT

public:
    VSpinBoxAction (const std::string& title, int value);

    int GetValue() const;

    void SetValue( int value );

private:
    VIntSpinBox* _spinBox;

private slots:
    void _spinBoxChanged( int value );

signals:
    void editingFinished( int );
};

class VCheckBoxAction : public QWidgetAction {
    Q_OBJECT

public:
    VCheckBoxAction (const std::string& title, bool value);

    bool GetValue() const;
    
    void SetValue( bool value );

private:
    VCheckBox* _checkBox;

private slots:
    void _checkBoxChanged( bool value );

signals:
    void clicked( bool );
};


class VStringLineEditAction : public QWidgetAction {
    Q_OBJECT

public:
    VStringLineEditAction( const std::string& title, std::string value );

    void SetValue( const std::string& value );

private:
    VStringLineEdit* _lineEdit;

private slots:
    void _lineEditChanged( int value );

signals:
    void ValueChanged( int );
};

class VIntLineEditAction : public QWidgetAction {
    Q_OBJECT

public:
    VIntLineEditAction( const std::string& title, int value );

    void SetValue( int value );

private:
    VIntLineEdit* _lineEdit;

private slots:
    void _lineEditChanged( int value );

signals:
    void ValueChanged( int );

};

class VDoubleLineEditAction : public QWidgetAction {
    Q_OBJECT

public:
    VDoubleLineEditAction( const std::string& title, double value );
    
    void SetValue( double value );

private:
    VDoubleLineEdit* _lineEdit;

private slots:
    void _lineEditChanged( double value );

signals:
    void ValueChanged( double );
};
