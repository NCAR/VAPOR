//************************************************************************
//									*
//		     Copyright (C)  2008				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		ParamsBase.h
//
//	Author:		John Clyne, modified by Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2008
//
//	Description:
//		Defines the ParamsBase class
//		This is an abstract class for classes that rely on
//		accessing an XML node for get/set
//

#ifndef ParamsBase_H
#define ParamsBase_H

#include <cassert>
#include <functional>
#include <vapor/MyBase.h>
#include <vapor/XmlNode.h>

namespace VAPoR {

//
//! \class ParamsBase
//! \brief Nodes with state in Xml tree representation
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This is pure abstract base class that may be derived for
//! to maintain parameter state information in a manner that
//! supports session save/restore and undo/redo operations. All
//! session state is maintained in an XML tree that may be
//! written to a file and subsequently used to reinitialize ParamsBase
//! class objects.
//!
//!
class PARAMS_API ParamsBase : public Wasp::MyBase {

  public:
    //! \class StateSave
    //! \brief State capture class
    //!
    //! A class for capturing state changes. A pointer to an
    //! instance of this class is passed to the ParamsBase constructor.
    //! Any changes to the ParamsBase are recorded to StateSave by
    //! calling StateSave::Save() with the effected node and
    //! a description of the change. It is expected that users of ParamsBase
    //! will re-implement StateSave to suit their own needs
    //
    class StateSave {
      public:
        //! Capture current state
        //!
        //! If the value of GetSaveState() is true this method is called prior
        //! to making any changes to the internal state
        //!
        virtual void Reinit(const XmlNode *rootNode) {}
        virtual void Save(const XmlNode *node, string description) {}
        virtual void BeginGroup(string description) {}
        virtual void EndGroup() {}
    };

    // NO DEFAULT CONSTRUCTOR
    //
    //ParamsBase();

    //! Create a ParamsBase object from scratch
    //!
    //! \param[in] ssave StateSave class object that will be used to record
    //! state changes made to this class.
    //!
    //! \param[in] classname The string identifier associated with a
    //! derived class that will be used to create new instances of that
    //! class via the ParamsFactory factory object.
    //!
    ParamsBase(
        StateSave *ssave, const string &classname);

    //! Create a ParamsBase object from an existing XmlNode tree
    //!
    //! This method will construct a ParamsBase object using an
    //! existing XML tree
    //
    ParamsBase(
        StateSave *ssave, XmlNode *node);

    //! Copy constructor.
    ParamsBase(const ParamsBase &rhs);

    ParamsBase &operator=(const ParamsBase &rhs);

    //! Equivalence operator
    //
    bool operator==(const ParamsBase &rhs) const {
        return (
            _ssave == rhs._ssave &&
            _node == rhs._node);
    }

    bool operator!=(const ParamsBase &rhs) const {
        return (!(*this == rhs));
    };

    //! Destroy object
    //!
    //! Destroys all resources except possibly the XmlNode and
    //! its children associated
    //! with this object. If this objects node is a root node (i.e. has
    //! no parent) the node is freed. Otherwise it is not.
    //!
    virtual ~ParamsBase();

    //! Set parent
    //!
    //! This method sets the parent of the ParamsBase class to
    //! \p parent, modifying both this class instance and the parent
    //!
    //! \param[in] parent A pointer to a parent ParamsBase class . If
    //! NULL the class will become parentless
    //
    void SetParent(ParamsBase *parent);

    XmlNode *GetNode() const { return _node; }

    virtual vector<long> GetValueLongVec(const string tag) const;

    virtual vector<long> GetValueLongVec(
        const string tag, const vector<long> &defaultVal) const;

    virtual long GetValueLong(
        const string tag, long defaultVal) const;

    virtual vector<double> GetValueDoubleVec(const string tag) const;

    virtual vector<double> GetValueDoubleVec(
        const string tag, const vector<double> &defaultVal) const;

    virtual double GetValueDouble(
        const string tag, double defaultVal) const;

    virtual vector<string> GetValueStringVec(const string tag) const;

    virtual vector<string> GetValueStringVec(
        const string tag, const vector<string> &defaultVal) const;

    virtual string GetValueString(
        const string tag, string defaultVal) const;

    virtual void SetValueLongVec(
        const string &tag, string description, const vector<long> &values);

    virtual void SetValueLong(
        const string &tag, string description, long value);

    virtual void SetValueDoubleVec(
        const string &tag, string description, const vector<double> &values);

    virtual void SetValueDouble(
        const string &tag, string description, double value);

    virtual void SetValueStringVec(
        const string &tag, string description, const vector<string> &values);

    virtual void SetValueString(
        const string &tag, string description, const string &value);

    //!
    //! Method for obtaining the name and/or tag associated with the instance
    //!
    string GetName() const {
        assert(_node);
        return (_node->Tag());
    }

  protected:
    ParamsBase(StateSave *ssave);

    //! Delete the named branch.
    //!
    //! This method deletes the named child, and all decendents, of the current
    //! destroying it's contents in the process. The
    //! named node must be a child of the current node. If the named node
    //! does not exist the result is a no-op.
    //!
    //! \param[in] name The name of the branch
    //
    void Remove(const string &name);

    //! Return the attributes associated with the current branch
    //!
    //! \retval map attribute mapping
    //
    const map<string, string> &GetAttributes();

    //! Remove (undefine) all parameters
    //!
    //! This method deletes any and all paramters contained in the base
    //! class as well as deleting any tree branches.
    //
    void Clear();

  protected:
    StateSave *_ssave;
    XmlNode *_node;
};

//////////////////////////////////////////////////////////////////////////
//
// ParamsSeparator Class
//
/////////////////////////////////////////////////////////////////////////

class PARAMS_API ParamsSeparator : public ParamsBase {
  public:
    ParamsSeparator(
        StateSave *ssave, const string &name);

    ParamsSeparator(
        StateSave *ssave, XmlNode *node);

    ParamsSeparator(
        ParamsSeparator *parent, const string &name);

    bool HasChild(const string &name) {
        return (GetNode()->HasChild(name));
    }
};

//////////////////////////////////////////////////////////////////////////
//
// ParamsFactory Class
//
/////////////////////////////////////////////////////////////////////////

class PARAMS_API ParamsFactory {
  public:
    static ParamsFactory *Instance() {
        static ParamsFactory instance;
        return &instance;
    }

    void RegisterFactoryFunction(
        string name,
        function<ParamsBase *(ParamsBase::StateSave *, XmlNode *)> classFactoryFunction) {

        // register the class factory function
        m_factoryFunctionRegistry[name] = classFactoryFunction;
    }

    ParamsBase *(CreateInstance(string classType, ParamsBase::StateSave *, XmlNode *));

    vector<string> GetFactoryNames() const;

  private:
    map<string, function<ParamsBase *(ParamsBase::StateSave *, XmlNode *)>>
        m_factoryFunctionRegistry;

    ParamsFactory() {}
    ParamsFactory(const ParamsFactory &) {}
    ParamsFactory &operator=(const ParamsFactory &) { return *this; }
};

//////////////////////////////////////////////////////////////////////////
//
// ParamsRegistrar Class
//
// Register ParamsBase derived class with:
//
//	static ParamsRegistrar<ParamsClass> registrar("myclassname");
//
// where 'ParamsClass' is a class derived from 'ParamsBase', and
// "myclassname" is the name of the class
//
/////////////////////////////////////////////////////////////////////////

template <class T>
class ParamsRegistrar {
  public:
    ParamsRegistrar(string classType) {

        // register the class factory function
        //
        ParamsFactory::Instance()->RegisterFactoryFunction(
            classType, [](ParamsBase::StateSave *ssave, XmlNode *node) -> ParamsBase * {
                if (node)
                    return new T(ssave, node);
                else
                    return new T(ssave);
            });
    }
};

//////////////////////////////////////////////////////////////////////////
//
// ParamsContainer Class
//
/////////////////////////////////////////////////////////////////////////

//
// The ParamsContainer class constructs an XML tree as depicted below,
// where 'Container Name' is the root of XML tree, and is the name
// passed into the constructor as 'myname'; 'Class Name' is the name of
// the derived ParamsBase class used to construct new instances of
// the derived class; and 'ele name x' is the unique name of the element
// contained in the container.
//
//
//            |----------------|
//            | Container Name |
//            |----------------|
//                    |
//                   \|/
//            |----------------|
//            |   Class Name   |
//            |----------------|
//                    |         \
//                   \|/         \
//            |----------------|  \ |----------------|
//            |   ele name 1   |....|   ele name n   |
//            |----------------|    |----------------|
//
class PARAMS_API ParamsContainer : public Wasp::MyBase {
  public:
    ParamsContainer(
        ParamsBase::StateSave *ssave, const string &myname);

    ParamsContainer(
        ParamsBase::StateSave *ssave, XmlNode *node);

    //! Copy constructor.
    ParamsContainer(const ParamsContainer &rhs);

    ParamsContainer &operator=(const ParamsContainer &rhs);

    //! Destroy object
    //!
    //! Destroys all resources except possibly the XmlNode associated
    //! with this object. If this object's node is a root node (i.e. has
    //! no parent) the node is freed. Otherwise it is not
    //!
    ~ParamsContainer();

    //! Set parent
    //!
    //! This method sets the parent of the ParamsBase class to
    //! \p parent, modifying both this class instance and the parent
    //!
    //! \param[in] parent A pointer to a parent ParamsBase class . If
    //! NULL the class will become parentless
    //
    void SetParent(ParamsBase *parent) {
        GetNode()->SetParent(parent->GetNode());
    }

    ParamsBase *Insert(ParamsBase *pb, string name);

    ParamsBase *Create(string classType, string name);

    void Remove(string name);

    void Remove(const ParamsBase *pb) {
        Remove(GetParamsName(pb));
    }

    ParamsBase *GetParams(string name) const;

    string GetParamsName(const ParamsBase *pb) const;

    vector<string> GetNames() const;

    size_t Size() const {
        return (_elements.size());
    }

    XmlNode *GetNode() const { return _separator->GetNode(); }

  private:
    ParamsBase::StateSave *_ssave;
    //XmlNode *_node;
    ParamsSeparator *_separator;
    map<string, ParamsBase *> _elements;
};

}; //End namespace VAPoR

#endif //ParamsBase_H
