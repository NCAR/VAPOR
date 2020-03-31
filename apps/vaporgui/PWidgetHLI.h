#pragma once

//! Creates the framework for creating Params Widgets that use the Params Database high level interface.

#define CreateHLIBase(T, LT)                                                \
    template<class P> class PWidgetHLIBase<P, T> {                          \
    protected:                                                              \
        typedef std::function<T(P *)>       GetterType;                     \
        typedef std::function<void(P *, T)> SetterType;                     \
                                                                            \
    public:                                                                 \
        PWidgetHLIBase(PWidget *w, GetterType getter, SetterType setter)    \
        {                                                                   \
            w->_usingHLI = true;                                            \
            w->_setter##LT = [setter](void *p, T v) { setter((P *)p, v); }; \
            w->_getter##LT = [getter](void *p) { return getter((P *)p); };  \
        }                                                                   \
    };

#define CreateInferredTemplateConstructor(className, T, constModifier) \
    template<class P> className##HLI<P> *new_##className##HLI(const std::string &label, T (P::*getter)() constModifier, void (P::*setter)(T)) { return new className##HLI<P>(label, getter, setter); }

#define CreateHLI(className, T)                                                                                                                      \
    template<class P> class className##HLI final : public className, public PWidgetHLIBase<P, T> {                                                   \
    public:                                                                                                                                          \
        className##HLI(const std::string &label, typename PWidgetHLIBase<P, T>::GetterType getter, typename PWidgetHLIBase<P, T>::SetterType setter) \
        : className("", label), PWidgetHLIBase<P, T>((PWidget *)this, getter, setter)                                                                \
        {                                                                                                                                            \
        }                                                                                                                                            \
    };                                                                                                                                               \
    CreateInferredTemplateConstructor(className, T, );                                                                                               \
    CreateInferredTemplateConstructor(className, T, const);

CreateHLIBase(long, Long);
CreateHLIBase(bool, Long);
CreateHLIBase(int, Long);
CreateHLIBase(double, Double);
CreateHLIBase(float, Double);
CreateHLIBase(std::string, String);

//
// My attempt at implementing this using purely function templates and no defines. Below is pretty close.
// It works but requires too much specilization code so I stopped working on it for now.
//

#ifdef SAVED_FOR_LATER
template<class Base, class Derived> void AssertInheritance() { (void)static_cast<Base *>((Derived *)0); }

template<class PW, typename T, class P, typename... Args> class PWidgetHLI : public PW {
    typedef std::function<T(P *)>       GetterType;
    typedef std::function<void(P *, T)> SetterType;

    GetterType _getter;
    SetterType _setter;

public:
    PWidgetHLI(std::string label, Args... args, GetterType getter, SetterType setter) : PW("", args..., label)
    {
        printf("PWidgetHLI Constructor (%s)\n", label.c_str());
        AssertInheritance<PWidget, PW>();
        AssertInheritance<VAPoR::ParamsBase, P>();

        this->_usingHLI = true;

        this->_getter = getter;
        this->_setter = setter;
    }

    virtual long getParamsLong() const override
    {
        static_assert(std::is_convertible<T, long>(), "");
        //        return this->_getterLong(this->getParams());
        return this->_getter((P *)this->getParams());
    }

    virtual void _setParamsLong(long v) override
    {
        static_assert(std::is_convertible<T, long>(), "");
        //        return this->_setterLong(this->getParams(), v);
        this->_setter((P *)this->getParams(), v);
    }

    //    template <typename=typename std::enable_if<std::is_convertible<T, std::string>::type>>
    virtual std::string getParamsString() const override
    {
        static_assert(std::is_convertible<T, std::string>(), "");
        //        return this->_getterLong(this->getParams());
        return this->_getter((P *)this->getParams());
    }

    virtual void _setParamsString(const std::string &v) override
    {
        static_assert(std::is_convertible<T, std::string>(), "");
        //        return this->_setterLong(this->getParams(), v);
        this->_setter((P *)this->getParams(), v);
    }
};

template<class PW, typename T, class P, typename... Args> class PWidgetHLI2 : public PWidgetHLI<PW, T, P, Args...> {
    using PWidgetHLI<PW, T, P, Args...>::PWidgetHLI;
};

template<class PW, class P, typename... Args> class PWidgetHLI2<PW, std::string, P, Args...> : public PWidgetHLI<PW, std::string, P, Args...> {
    using PWidgetHLI<PW, std::string, P, Args...>::PWidgetHLI;
    virtual std::string getParamsString() const override {}
};

template<class PW, typename T, class P, typename... Args> typename std::enable_if<std::is_same<T, bool>::value, T>::type class PWidgetHLI2 : public PWidgetHLI<PW, T, P, Args...> {
    virtual long getParamsLong() const override {}
};

// template <class PW, class P, typename... Args>
// class PWidgetHLI2<PW, std::string, P, Args...> : public PWidgetHLI<PW, std::string, P, Args...> {

//
//};
#endif
