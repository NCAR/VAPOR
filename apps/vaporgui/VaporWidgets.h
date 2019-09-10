#ifndef VAPORWIDGETS_H
#define VAPORWIDGETS_H

// Some classes cannot be forward declared due to nested name specifiers,
// such as QFileDialog::FileMode
#include <QDir>
#include <QFileDialog>
#include <QPushButton>

class QWidget;
class QLabel;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QDoubleValidator;
class QSpacerItem;
class QHBoxLayout;

class VaporWidget : public QWidget {
    Q_OBJECT

public:
    VaporWidget(QWidget *parent, const std::string &labelText);
    VaporWidget(QWidget *parent, const QString &labelText);
    void SetLabelText(const std::string &text);
    void SetLabelText(const QString &text);

protected:
    QLabel *     _label;
    QSpacerItem *_spacer;
    QHBoxLayout *_layout;
};

class VPushButton : public VaporWidget {
    Q_OBJECT

public:
    VPushButton(QWidget *parent, const std::string &labelText = "Label", const std::string &buttonText = "Button");

    void SetButtonText(const std::string &text);
    void SetButtonText(const QString &text);

signals:
    void _pressed();

protected:
    QPushButton *_button;

private slots:
    void _buttonPressed();
};

class VComboBox : public VaporWidget {
    Q_OBJECT

public:
    VComboBox(QWidget *parent, const std::string &labelText = "Label");
    int         GetCurrentIndex() const;
    std::string GetCurrentText() const;
    void        AddOption(const std::string &option, int index = 0);
    void        RemoveOption(int index);

private:
    QComboBox *_combo;

private slots:
    void _userIndexChanged(int index);

signals:
    void _indexChanged(int index);
};

class VCheckBox : public VaporWidget {
    Q_OBJECT

public:
    VCheckBox(QWidget *parent, const std::string &labelText = "Label");
    bool GetCheckState() const;

private:
    QCheckBox *_checkbox;

private slots:
    void _userClickedCheckbox();

signals:
    void _checkboxClicked();
};

class VFileSelector : public VPushButton {
    Q_OBJECT

public:
    void        SetPath(const std::string &defaultPath);
    void        SetPath(const QString &defaultPath);
    std::string GetPath() const;

protected:
    VFileSelector(QWidget *parent, const std::string &labelText = "Label", const std::string &filePath = QDir::homePath().toStdString(),
                  QFileDialog::FileMode fileMode = QFileDialog::FileMode::ExistingFile);

    QFileDialog::FileMode _fileMode;

private slots:
    void _openFileDialog();
    void _setPathFromLineEdit();

signals:
    void _pathChanged();

private:
    QLineEdit * _lineEdit;
    std::string _filePath;

    virtual bool _isFileOperable(const std::string &filePath) const = 0;
};

class VFileReader : public VFileSelector {
    Q_OBJECT

public:
    VFileReader(QWidget *parent, const std::string &labelText = "Label", const std::string &filePath = QDir::homePath().toStdString(),
                QFileDialog::FileMode fileMode = QFileDialog::FileMode::ExistingFile);

private:
    virtual bool _isFileOperable(const std::string &filePath) const;
};

class VFileWriter : public VFileSelector {
    Q_OBJECT

public:
    VFileWriter(QWidget *parent, const std::string &labelText = "Label", const std::string &filePath = QDir::homePath().toStdString());

private:
    virtual bool _isFileOperable(const std::string &filePath) const;
};

class VPushButtonWithDoubleClick : public QPushButton {
    Q_OBJECT
    using QPushButton::QPushButton;
    void mouseDoubleClickEvent(QMouseEvent *e) { emit doubleClicked(); }

signals:
    void doubleClicked();
};

#endif    // VAPORWIDGETS_H
