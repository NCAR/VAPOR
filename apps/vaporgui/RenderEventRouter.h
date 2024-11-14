#pragma once

#include "VaporFwd.h"
#include "Updatable.h"
#include <common.h>
#include <functional>
#include <QObject>

using std::vector;
using std::string;

using std::function;

class RenderEventRouter : public Updatable {
public:
    RenderEventRouter(ControlExec *ce) : _controlExec(ce) {}
    virtual ~RenderEventRouter() {}
    void SetActive(string instName) { _instName = instName; }

    virtual string GetType() const = 0;
    virtual bool Supports2DVariables() const = 0;
    virtual bool Supports3DVariables() const = 0;
    virtual bool SupportsParticleVariables() const { return false; }

    VAPoR::RenderParams *GetActiveParams() const;
    VAPoR::DataMgr *GetActiveDataMgr() const;
    string GetDescription() const { return _getDescription(); }
    string GetSmallIconImagePath() const;
    string GetIconImagePath() const;

protected:
    ControlExec *_controlExec;

    virtual string _getDescription() const = 0;
    virtual string _getSmallIconImagePath() const = 0;
    virtual string _getIconImagePath() const = 0;

private:
    string _instName;
};


//////////////////////////////////////////////////////////////////////////
//
// RenderEventRouterFactory Class
//
/////////////////////////////////////////////////////////////////////////

class RenderEventRouterFactory {
public:
    static RenderEventRouterFactory *Instance()
    {
        static RenderEventRouterFactory instance;
        return &instance;
    }

    void RegisterFactoryFunction(string name, function<RenderEventRouter *(QWidget *, VAPoR::ControlExec *)> classFactoryFunction)
    {
        _factoryFunctionRegistry[name] = classFactoryFunction;
    }

    RenderEventRouter *CreateInstance(string classType, QWidget *, VAPoR::ControlExec *);
    vector<string> GetFactoryNames() const;

private:
    std::map<string, function<RenderEventRouter *(QWidget *, VAPoR::ControlExec *)>> _factoryFunctionRegistry;
};

//////////////////////////////////////////////////////////////////////////
//
// Register RenderEventRouter derived class with:
//
//	static RenderEventRouterRegistrar<RERClass> registrar("myclassname");
//
// where 'RERClass' is a class derived from 'RenderEventRouter', and
// "myclassname" is the name of the class
//
/////////////////////////////////////////////////////////////////////////

template<class T> class RenderEventRouterRegistrar {
public:
    RenderEventRouterRegistrar(string classType)
    {
        RenderEventRouterFactory::Instance()->RegisterFactoryFunction(classType, [](QWidget *parent, VAPoR::ControlExec *ce) -> RenderEventRouter * { return new T(parent, ce); });
    }
};

