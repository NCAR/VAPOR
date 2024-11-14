#pragma once


class QEnableableI {
public:
//    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual ~QEnableableI() {}
};


template <class T>
class QEnableable : public QEnableableI {
public:
    QEnableable(T *o) : _o(o) {}
//    bool isEnabled() const override { return _o->isEnabled(); }
    void setEnabled(bool enabled) override { _o->setEnabled(enabled); }
private:
    T* const _o;
};
