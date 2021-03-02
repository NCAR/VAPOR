#include "PCornerSelector.h"
#include "VCheckBox.h"
#include <vapor/ColorbarPbase.h>
#include <QFrame>
#include <QGridLayout>


using VAPoR::ColorbarPbase;


class PCornerSelector::Check : public VCheckBox {
public:
    float x, y;
    Check(float x, float y) : x(x), y(y) { setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum); }
};


template<QFrame::Shape T> class Line : public QFrame {
public:
    Line()
    {
        setFrameShape(T);
        setFrameShadow(QFrame::Sunken);
    }
};
typedef Line<QFrame::HLine> HLine;
typedef Line<QFrame::VLine> VLine;


PCornerSelector::PCornerSelector(std::string tag, std::string title) : PLineItem(tag, title, _w = new QWidget)
{
    QGridLayout *l = new QGridLayout;
    //        l->setSpacing(0); This breaks the layout whereas setting H and V manually works... Thanks Qt.
    l->setVerticalSpacing(0);
    l->setHorizontalSpacing(0);
    _w->setLayout(l);

    auto add = [&](int x, int y, QWidget *w) {
        l->addWidget(w, y, x);
        w->setContentsMargins(0, 0, 0, 0);
        if (auto c = dynamic_cast<Check *>(w))
            _checks.push_back(c);
        else if (dynamic_cast<VLine *>(w))
            l->setAlignment(w, Qt::AlignHCenter);
    };

    add(0, 0, new Check(0, 1));
    add(1, 0, new HLine);
    add(2, 0, new Check(0.5, 1));
    add(3, 0, new HLine);
    add(4, 0, new Check(1, 1));

    add(0, 1, new VLine);
    add(4, 1, new VLine);

    add(0, 2, new Check(0, 0.5));
    add(4, 2, new Check(1, 0.5));

    add(0, 3, new VLine);
    add(4, 3, new VLine);

    add(0, 4, new Check(0, 0));
    add(1, 4, new HLine);
    add(2, 4, new Check(0.5, 0));
    add(3, 4, new HLine);
    add(4, 4, new Check(1, 0));

    l->setRowMinimumHeight(1, l->itemAtPosition(0, 0)->minimumSize().height() / 1.618);
    l->setRowMinimumHeight(3, l->itemAtPosition(0, 0)->minimumSize().height() / 1.618);

    for (auto c : _checks) connect(c, &VCheckBox::ValueChanged, this, &PCornerSelector::checked);
}


void PCornerSelector::updateGUI() const
{
    auto d = getValue();
    if (d.size() != 2) return;
    float x = d[0], y = d[1];

    for (auto c : _checks) {
        float cx = c->x - (c->x - 0.5) * 2 * padding;
        float cy = c->y - (c->y - 0.5) * 2 * padding;
        c->SetValue(equalsf(cx, x) && equalsf(cy, y));
    }
}


void PCornerSelector::checked(bool on)
{
    Check *c = dynamic_cast<Check *>(sender());
    assert(c);
    if (!c) return;

    if (!on) c->SetValue(true);

    float cx = c->x - (c->x - 0.5) * 2 * padding;
    float cy = c->y - (c->y - 0.5) * 2 * padding;
    setValue({cx, cy});
}


std::vector<double> PCornerSelector::getValue() const { return getParams()->GetValueDoubleVec(getTag()); }


void PCornerSelector::setValue(const std::vector<double> &v) { getParams()->SetValueDoubleVec(getTag(), "", v); }


PColorbarCornerSelector::PColorbarCornerSelector() : PCornerSelector("", "Position") {}

std::vector<double> PColorbarCornerSelector::getValue() const { return getParams<ColorbarPbase>()->GetCornerPosition(); }

void PColorbarCornerSelector::setValue(const std::vector<double> &v) { getParams<ColorbarPbase>()->SetCornerPosition(v); }
