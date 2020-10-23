#include "PShowIf.h"
#include "PGroup.h"
#include <vapor/ParamsBase.h>
#include <vapor/VAssert.h>

PShowIf::PShowIf(std::string tag) : PWidgetWrapper(tag, _group = new PGroup) {}

PShowIf *PShowIf::Equals(long l)
{
    _test = std::unique_ptr<Test>(new TestLongEquals(getTag(), l));
    return this;
}

PShowIf *PShowIf::Equals(std::string s)
{
    _test = std::unique_ptr<Test>(new TestStringEquals(getTag(), s));
    return this;
}

PShowIf *PShowIf::Not()
{
    _negate = !_negate;
    return this;
}

PShowIf *PShowIf::Then(PWidget *p)
{
    _hasThen = true;
    _group->Add(new Helper(this, p));
    return this;
}

PShowIf *PShowIf::Else(PWidget *p)
{
    _hasElse = true;
    _group->Add(new Helper(this, p, true));
    return this;
}

PShowIf *PShowIf::Then(const PGroup::List &list) { return Then(new PGroup(list)); }

PShowIf *PShowIf::Else(const PGroup::List &list) { return Else(new PGroup(list)); }

bool PShowIf::isShown() const
{
    bool result = evaluate();
    return (result && _hasThen) || (!result && _hasElse);
}

bool PShowIf::evaluate() const
{
    VAssert(_test);
    return _test->Evaluate(getParams()) != _negate;
}

PShowIf::Helper::Helper(PShowIf *parent, PWidget *widget, bool negate) : PWidgetWrapper(widget), _parent(parent), _negate(negate) {}

bool PShowIf::Helper::isShown() const { return _parent->evaluate() != _negate; }

bool PShowIf::TestLongEquals::Evaluate(VAPoR::ParamsBase *params) const { return params->GetValueLong(_tag, 0) == _val; }

bool PShowIf::TestStringEquals::Evaluate(VAPoR::ParamsBase *params) const { return params->GetValueString(_tag, "") == _val; }
