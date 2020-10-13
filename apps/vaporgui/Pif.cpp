#include "Pif.h"
#include "PGroup.h"
#include <vapor/ParamsBase.h>
#include <vapor/VAssert.h>

Pif::Pif(std::string tag)
: PWidgetWrapper(tag, _group = new PGroup) {}

Pif *Pif::Equals(long l)
{
    _test = std::unique_ptr<Test>(new TestLongEquals(getTag(), l));
    return this;
}

Pif *Pif::Equals(std::string s)
{
    _test = std::unique_ptr<Test>(new TestStringEquals(getTag(), s));
    return this;
}

Pif *Pif::Not()
{
    _negate = !_negate;
    return this;
}

Pif *Pif::Then(PWidget *p)
{
    _hasThen = true;
    _group->Add(new Helper(this, p));
    return this;
}

Pif *Pif::Else(PWidget *p)
{
    _hasElse = true;
    _group->Add(new Helper(this, p, true));
    return this;
}

Pif *Pif::Then(const PGroup::List &list)
{
    return Then(new PGroup(list));
}

Pif *Pif::Else(const PGroup::List &list)
{
    return Else(new PGroup(list));
}

bool Pif::isShown() const
{
    bool result = evaluate();
    return (result && _hasThen) || (!result && _hasElse);
}

bool Pif::evaluate() const
{
    VAssert(_test);
    return _test->Evaluate(getParams()) != _negate;
}

Pif::Helper::Helper(Pif *parent, PWidget *widget, bool negate)
: PWidgetWrapper(widget), _parent(parent), _negate(negate) {}

bool Pif::Helper::isShown() const
{
    return _parent->evaluate() != _negate;
}

bool Pif::TestLongEquals::Evaluate(VAPoR::ParamsBase *params) const
{
    return params->GetValueLong(_tag, 0) == _val;
}

bool Pif::TestStringEquals::Evaluate(VAPoR::ParamsBase *params) const
{
    return params->GetValueString(_tag, "") == _val;
}
