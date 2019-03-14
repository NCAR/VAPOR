#ifndef VAPORWIDGETS_H
#define VAPORWIDGETS_H

class QWidget;
class QLabel;
class QComboBox;
class QCheckBox;
class QPushButton;
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

class VPushButton : public VaporWidget    // QWidget, public Ui_VPushButton
{
    Q_OBJECT

public:
    VPushButton(QWidget *parent, std::string labelText = "Label", std::string buttonText = "Button");

    void SetButtonText(const std::string &text);
    void SetButtonText(const QString &text);

signals:
    void _pressed();

protected:
    QPushButton *_button;

private slots:
    void _buttonPressed();
};

class VComboBox : public VaporWidget    // public QWidget, public Ui_VComboBox
{
    Q_OBJECT

public:
    VComboBox(QWidget *parent, std::string labelText = "Label");
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

class VCheckBox : public VaporWidget    // QWidget, public Ui_VCheckBox
{
    Q_OBJECT

public:
    VCheckBox(QWidget *parent, std::string labelText = "Label");
    bool GetCheckState() const;

private:
    QCheckBox *_checkbox;

private slots:
    void _userClickedCheckbox();

signals:
    void _checkboxClicked();
};

class VPathSelector : public VPushButton    // QWidget, public Ui_VPathSelector
{
    Q_OBJECT

    enum Flags { SINGLE_FILE = (1u << 0), PATH = (1u << 2) };

public:
    VPathSelector(QWidget *parent, Flags flags, const std::string &labelText = "Label", const std::string &filePath = QDir::homePath().toStdString() QFileMode fileMode = QFileDialog::ExistingFile;);
    void        SetPath(const std::string &defaultPath);
    std::string GetPath() const;

private slots:
    void _openFileDialog();
    void _setFilePath();

signals:
    void _pathChanged();

private:
    QLineEdit * _lineEdit;
    std::string _filePath;
    QFileMode   _fileMode;
};

#endif    // VAPORWIDGETS_H
