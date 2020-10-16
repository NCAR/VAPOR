#pragma once

#include "PWidgetWrapper.h"
#include "PGroup.h"

//! \class Pif
//! \brief Shows and hides PWidgets based on a logic test.
//! \author Stas Jaroszynski
//!
//! For example:
//! (new Pif("param_tag"))->Not()->Equals("Hi")->Then(
//!     PWidgets here will only show if param_tag != "Hi"
//! )->Else(
//!     PWidgets here will only show if param_tag == "Hi"
//! );

class Pif : public PWidgetWrapper {
    PGroup *_group;
    bool _negate = false;
    bool _hasThen = false;
    bool _hasElse = false;
public:
    Pif (std::string tag);
    Pif *Equals(long l);
    Pif *Equals(std::string s);
//    Pif *Or(Pif *);
    Pif *Not();
    Pif *Then(PWidget *p);
    Pif *Else(PWidget *p);
    Pif *Then(const PGroup::List &list);
    Pif *Else(const PGroup::List &list);
    
protected:
    bool isShown() const override;
    
private:
    bool evaluate() const;
    
    class Helper : public PWidgetWrapper {
        const Pif *_parent;
        const bool _negate;
    public:
        Helper(Pif *parent, PWidget *widget, bool negate=false);
    protected:
        bool isShown() const override;
    };
    
    struct Test {
        const std::string _tag;
        Test(std::string tag) : _tag(tag) {}
        virtual bool Evaluate(VAPoR::ParamsBase *params) const = 0;
        virtual ~Test() {}
    };
    
    struct TestLongEquals : public Test {
        const long _val;
        TestLongEquals(std::string tag, long val) : Test(tag), _val(val) {}
        bool Evaluate(VAPoR::ParamsBase *params) const override;
    };
    
    struct TestStringEquals : public Test {
        const std::string _val;
        TestStringEquals(std::string tag, std::string val) : Test(tag), _val(val) {}
        bool Evaluate(VAPoR::ParamsBase *params) const override;
    };
    
    std::unique_ptr<Test> _test;
};
