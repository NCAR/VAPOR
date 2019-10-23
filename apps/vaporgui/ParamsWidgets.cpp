#include "ParamsWidgets.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QEvent>
#include <QColorDialog>
#include "QColorWidget.h"

using namespace VAPoR;

ParamsWidget::ParamsWidget(const std::string &tag, const std::string &label)
{
    assert(!tag.empty());
    _tag = tag;

    if (label.empty())
        _label = tag;
    else
        _label = label;

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    this->setLayout(layout);

    QLabel *labelWidget = new QLabel(_label.c_str());
    _spacer = new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    layout->addWidget(labelWidget);
    layout->addItem(_spacer);
}

ParamsWidgetCheckbox::ParamsWidgetCheckbox(const std::string &tag, const std::string &labelText) : ParamsWidget(tag, labelText)
{
    _checkBox = new QCheckBox();
    connect(_checkBox, SIGNAL(clicked(bool)), this, SLOT(checkbox_clicked(bool)));

    layout()->addWidget(_checkBox);
}

void ParamsWidgetCheckbox::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _checkBox->setChecked(p->GetValueLong(_tag, false));
}

void ParamsWidgetCheckbox::checkbox_clicked(bool checked)
{
    if (_params) _params->SetValueLong(_tag, _tag, checked);
}

ParamsWidgetNumber::ParamsWidgetNumber(const std::string &tag, const std::string &labelText) : ParamsWidget(tag, labelText)
{
    _lineEdit = new QLineEdit();
    _lineEdit->setValidator(new QIntValidator);
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChangedSlot()));

    layout()->addWidget(_lineEdit);
}

void ParamsWidgetNumber::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _lineEdit->setText(QString::number(p->GetValueLong(_tag, false)));
}

ParamsWidgetNumber *ParamsWidgetNumber::SetRange(int min, int max)
{
    const QValidator *toDelete = _lineEdit->validator();
    _lineEdit->setValidator(new QIntValidator(min, max));
    if (toDelete) delete toDelete;
    return this;
}

void ParamsWidgetNumber::valueChangedSlot()
{
    if (_params) _params->SetValueLong(_tag, _tag, _lineEdit->text().toInt());
}

ParamsWidgetFloat::ParamsWidgetFloat(const std::string &tag, const std::string &labelText) : ParamsWidget(tag, labelText)
{
    _lineEdit = new QLineEdit();
    _lineEdit->setValidator(new QDoubleValidator);
    connect(_lineEdit, SIGNAL(editingFinished()), this, SLOT(valueChangedSlot()));

    layout()->addWidget(_lineEdit);
}

void ParamsWidgetFloat::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _lineEdit->setText(QString::number(p->GetValueDouble(_tag, false)));
}

ParamsWidgetFloat *ParamsWidgetFloat::SetRange(float min, float max)
{
    const QValidator *toDelete = _lineEdit->validator();
    _lineEdit->setValidator(new QDoubleValidator(min, max, 2));
    if (toDelete) delete toDelete;
    return this;
}

void ParamsWidgetFloat::valueChangedSlot()
{
    if (_params) _params->SetValueDouble(_tag, _tag, _lineEdit->text().toDouble());
}

ParamsWidgetDropdown::ParamsWidgetDropdown(const std::string &tag, const std::vector<std::string> &items, const std::vector<int> &itemValues, const std::string &labelText)
: ParamsWidget(tag, labelText)
{
    _box = new QComboBox();
    connect(_box, SIGNAL(currentIndexChanged(int)), this, SLOT(indexChangedSlot(int)));

    _box->blockSignals(true);
    for (string item : items) _box->addItem(QString::fromStdString(item));
    _box->blockSignals(false);

    layout()->addWidget(_box);

    if (!itemValues.empty()) {
        assert(itemValues.size() == items.size());
        _itemValues = itemValues;
    }
}

void ParamsWidgetDropdown::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _box->blockSignals(true);
    _box->setCurrentIndex(getIndexForValue(p->GetValueLong(_tag, 0)));
    _box->blockSignals(false);
}

void ParamsWidgetDropdown::indexChangedSlot(int index)
{
    if (_params) _params->SetValueLong(_tag, _tag, getValueForIndex(index));
}

int ParamsWidgetDropdown::getValueForIndex(int index) const
{
    if (_itemValues.empty()) return index;
    return _itemValues[index];
}

int ParamsWidgetDropdown::getIndexForValue(int value) const
{
    if (_itemValues.empty()) return value;

    const int N = _itemValues.size();
    for (int i = 0; i < N; i++)
        if (_itemValues[i] == value) return i;

    return -1;
}

ParamsWidgetColor::ParamsWidgetColor(const std::string &tag, const std::string &label) : ParamsWidget(tag, label)
{
    _color = new QColorWidget;
    connect(_color, SIGNAL(colorChanged(QColor)), this, SLOT(colorChanged(QColor)));
    layout()->addWidget(_color);
}

void ParamsWidgetColor::Update(VAPoR::ParamsBase *p)
{
    _params = p;

    QColor color = VectorToQColor(p->GetValueDoubleVec(_tag));
    _color->setColor(color);
}

void ParamsWidgetColor::colorChanged(QColor color)
{
    if (_params) _params->SetValueDoubleVec(_tag, _tag, QColorToVector(color));
}

QColor ParamsWidgetColor::VectorToQColor(const std::vector<double> &v)
{
    if (v.size() != 3) return QColor::fromRgb(0, 0, 0);
    return QColor::fromRgb(v[0] * 255, v[1] * 255, v[2] * 255);
}

std::vector<double> ParamsWidgetColor::QColorToVector(const QColor &c)
{
    vector<double> v(3, 0);
    v[0] = c.redF();
    v[1] = c.greenF();
    v[2] = c.blueF();
    return v;
}

#include <QFileDialog>
#include <vapor/FileUtils.h>

ParamsWidgetFile::ParamsWidgetFile(const std::string &tag, const std::string &label) : ParamsWidget(tag, label)
{
#warning _spacer is a hack. Will be refactored
    _spacer->changeSize(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);

    _button = new QPushButton;
    _button->setText("Select");
    connect(_button, SIGNAL(clicked()), this, SLOT(_buttonClicked()));

    _pathTexbox = new QLineEdit;
    _pathTexbox->setReadOnly(true);

    layout()->addWidget(_pathTexbox);
    layout()->addWidget(_button);
}

void ParamsWidgetFile::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    _pathTexbox->setText(QString::fromStdString(p->GetValueString(_tag, "<empty>")));
}

ParamsWidgetFile *ParamsWidgetFile::SetFileTypeFilter(const std::string &filter)
{
    _fileTypeFilter = QString::fromStdString(filter);
    return this;
}

void ParamsWidgetFile::_buttonClicked()
{
    if (!_params) return;

    string defaultPath;
    string selectedFile = _params->GetValueString(_tag, "");

    if (Wasp::FileUtils::Exists(selectedFile))
        defaultPath = Wasp::FileUtils::Dirname(selectedFile);
    else
        defaultPath = Wasp::FileUtils::HomeDir();

    QString qSelectedPath = QFileDialog::getOpenFileName(nullptr, "Select a file", QString::fromStdString(defaultPath), _fileTypeFilter);
    if (qSelectedPath.isNull()) return;

    _params->SetValueString(_tag, _tag, qSelectedPath.toStdString());
}

/*
ParamsWidgetTextLabel::ParamsWidgetTextLabel(const std::string &tag, const std::string &label)
: ParamsWidget(tag, label)
{
    _textLabel = new QLabel;
    layout()->addWidget(_textLabel);
    this->setSizePolicy(QSizePolicy::Maximum, this->sizePolicy().verticalPolicy());
}

void ParamsWidgetTextLabel::Update(VAPoR::ParamsBase *p)
{
    _params = p;
    QString text = QString::fromStdString(p->GetValueString(_tag, "<empty>"));
    QFontMetrics metrics(_textLabel->font());
    QString elidedText = metrics.elidedText(text, Qt::ElideRight, _textLabel->maximumWidth());
    _textLabel->setText(elidedText);
}
*/

ParamsWidgetGroup::ParamsWidgetGroup(const std::string &title) : QGroupBox(title.c_str()) { setLayout(new QVBoxLayout); }

void ParamsWidgetGroup::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::FontChange && !fontUpdated) {
        fontUpdated = true;
        this->setStyleSheet("font-size: " + QString::number(font().pointSize()) + "pt");
    }
}

ParamsWidgetTabGroup::ParamsWidgetTabGroup(const std::string &title)
{
    addTab(new QWidget(this), QString::fromStdString(title));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSpacing(0);
    _tab()->setLayout(layout);
}

QWidget *ParamsWidgetTabGroup::_tab() const { return this->widget(0); }

void ParamsWidgetTabGroup::Update(VAPoR::ParamsBase *p)
{
    for (ParamsWidget *w : _widgets) w->Update(p);
}

void ParamsWidgetTabGroup::Add(ParamsWidget *widget)
{
    _tab()->layout()->addWidget(widget);
    _widgets.push_back(widget);
}
