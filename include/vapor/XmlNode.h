//
//      $Id$
//

#ifndef _XmlNode_h_
#define _XmlNode_h_

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <stack>
#include <vapor/MyBase.h>
#ifdef WIN32
#pragma warning(disable : 4251)
#endif

namespace VAPoR {

//
//! \class XmlNode
//! \brief An Xml tree
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This class manages an XML tree. Each node in the tree
//! coresponds to an XML "parent" element. The concept
//! of "parent" element is a creation of this class,
//! and should be confused with any notions of parent
//! in more commonly used XML jargon. A parent element
//! is simply one possessing child XML elements
//! Non-parent elements - those elements that do not have
//! children elements -
//! may be thought of as data elements. Typically
//! non-parent elements possess XML character data. Parent
//! elements may or may not have XML character data
//! themselves.
//!
//! The XmlNode class is derived from the MyBase base
//! class. Hence all of the methods make use of MyBase's
//! error reporting capability.
//! Any method with a return type of int returns a 0 on
//! success, and and a negative int on failure, unless otherwise
//! docummented. Similary, methods returning pointers return non-NULL on
//! success and NULL on failure. Failure in either case triggers
//! logging of an error message with MyBase::SetErrMsg()
//!
class PARAMS_API XmlNode : public Wasp::MyBase {
  public:
    //! Constructor for the XmlNode class.
    //!
    //! Create's a new Xml node
    //!
    //! \param[in] tag Name of Xml node
    //! \param[in] attrs A list of Xml attribute names and values for this node
    //! \param[in] numChildrenHint Reserve space for the indicated number of
    //! children. Children must be created with
    //! the NewChild() method
    //!
    XmlNode(
        const string &tag, const map<string, string> &attrs,
        size_t numChildrenHint = 0);
    XmlNode(
        const string &tag,
        size_t numChildrenHint = 0);
    XmlNode();

    virtual XmlNode *Construct(
        const string &tag, const map<string, string> &attrs,
        size_t numChildrenHint = 0) { return (new XmlNode(tag, attrs, numChildrenHint)); }

    //! Copy constructor for the XmlNode class.
    //!
    //! Create's a new XmlNode node from an existing one. The new
    //! node will be parentless.
    //!
    //! \param[in] node XmlNode instance from which to construct a copy
    //!
    XmlNode(const XmlNode &node);

    virtual XmlNode *Clone() { return new XmlNode(*this); };

    //! Destructor
    //!
    //! Recursively delete node and chilren, but only if this is the root
    //! node. Otherwise this method is a no-op
    //!
    //! \sa SetParent()
    //
    virtual ~XmlNode();

    //! Set or get that node's tag (name)
    //!
    //! \retval tag A reference to the node's tag
    //
    string &Tag() { return (_tag); }

    string GetTag() const { return (_tag); }

    void SetTag(string tag) { _tag = tag; }

    //! Set or get that node's attributes
    //!
    //! \retval attrs A reference to the node's attributes
    //
    map<string, string> &Attrs() { return (_attrmap); }

    // These methods set or get XML character data, possibly formatting
    // the data in the process. The paramter 'tag' identifies the XML
    // element tag associated with the character data. The
    // parameter, 'values', contains the character data itself. The Long and
    // Double versions of these methods convert a between character streams
    // and vectors of longs or doubles as appropriate.
    //

    //! Set an Xml element of type long
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the array of longs
    //! specified by \p values
    //!
    //! \param[in] tags Sequence of names of elements as a path to the desired node
    //! \param[in] values Vector of longs to be converted to character data
    //!
    //! \retval status Returns 0 if successful
    //
    virtual void SetElementLong(
        const vector<string> &tags, const vector<long> &values);
    virtual void SetElementLong(const vector<string> &tags, long value) {
        vector<long> values(1, value);
        XmlNode::SetElementLong(tags, values);
    }

    //! Set an Xml element of type long
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the array of longs
    //! specified by \p values
    //!
    //! \param[in] tag Name of the element to define/set
    //! \param[in] values Vector of longs to be converted to character data
    //!
    //! \retval status Returns 0 if successful
    //
    virtual void SetElementLong(
        const string &tag, const vector<long> &values);
    virtual void SetElementLong(const string &tag, long value) {
        vector<long> values(1, value);
        XmlNode::SetElementLong(tag, values);
    }

    //! Get an Xml element's data of type long
    //!
    //! Return the character data associated with the Xml elemented
    //! named by \p tag for this node. The data is interpreted and
    //! returned as a vector of longs. If the element does not exist
    //! an empty vector is returned.
    //!
    //! \param[in] tag Name of element
    //! \retval vector Vector of longs associated with the named elemented
    //!
    virtual const vector<long> &GetElementLong(const string &tag) const;

    //! Return true if the named element of type long exists
    //!
    //! \param[in] tag Name of element
    //! \retval value at element
    //!
    virtual bool HasElementLong(const string &tag) const;

    //! Set an Xml element of type double
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the array of doubles
    //! specified by \p values
    //!
    //! \param[in] tag Name of the element to define/set
    //! \param[in] values Vector of doubles to be converted to character data
    //!
    //! \retval status 0 if successful
    //
    virtual void SetElementDouble(
        const string &tag, const vector<double> &values);
    virtual void SetElementDouble(const string &tag, double value) {
        vector<double> values(1, value);
        XmlNode::SetElementDouble(tag, values);
    }

    //! Set an Xml element of type double, using a sequence of tags
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the array of doubles
    //! specified by \p values
    //!
    //! \param[in] tags vector of tags to the specified element
    //! \param[in] values Vector of doubles to be converted to character data
    //!
    //! \retval status 0 if successful
    //
    virtual void SetElementDouble(
        const vector<string> &tags, const vector<double> &values);
    virtual void SetElementDouble(const vector<string> &tags, double value) {
        vector<double> values(1, value);
        XmlNode::SetElementDouble(tags, values);
    }

    //! Get an Xml element's data of type double
    //!
    //! Return the character data associated with the Xml elemented
    //! named by \p tag for this node. The data is interpreted and
    //! returned as a vector of doubles. If the element does not exist
    //! an empty vector is returned.
    //!
    //! \param[in] tag Name of element
    //! \retval vector Vector of doubles associated with the named elemented
    //!
    virtual const vector<double> &GetElementDouble(const string &tag) const;

    //! Return true if the named element of type double exists
    //!
    //! \param[in] tag Name of element
    //! \retval bool
    //!
    virtual bool HasElementDouble(const string &tag) const;

    //! Set an Xml element of type string
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the string
    //! specified by \p values
    //!
    //! \param[in] tag Name of the element to define/set
    //! \param[in] values string to be converted to character data
    //!
    //! \retval status Returns a non-negative value on success
    //! \retval status Returns 0 if successful
    //
    virtual void SetElementString(const string &tag, const string &values);

    //! Get an Xml element's data of type string
    //!
    //! Return the character data associated with the Xml elemented
    //! named by \p tag for this node. The data is interpreted and
    //! returned as a string. If the element does not exist
    //! an empty vector is returned.
    //!
    //! \param[in] tag Name of element
    //! \retval string The string associated with the named element
    //!
    virtual const string &GetElementString(const string &tag) const;

    //! Set an Xml element of type string
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the array of strings
    //! specified by \p values. The array of strings is first
    //! translated to a single string of space-separated words (contiguous
    //! characters)
    //!
    //! \param[in] tag Name of the element to define/set
    //! \param[in] values Vector of strings to be converted to a
    //! space-separated list of characters
    //!
    //! \retval status Returns 0 if successful
    //
    virtual void SetElementStringVec(
        const string &tag, const vector<string> &values);
    //! Set an Xml element of type string
    //!
    //! This method defines and sets an Xml element. The Xml character
    //! data to be associated with this element is the array of strings
    //! specified by \p values. The array of strings is first
    //! translated to a single string of space-separated words (contiguous
    //! characters)
    //!
    //! \param[in] tagpath sequence of tags leading from this to element
    //! \param[in] values Vector of strings to be converted to a
    //! space-separated list of characters
    //!
    //! \retval status Returns 0 if successful
    //
    virtual void SetElementStringVec(
        const vector<string> &tagpath, const vector<string> &values);

    //! Get an Xml element's data of type string
    //!
    //! Return the character data associated with the Xml elemented
    //! named by \p tag for this node. The data is interpreted as
    //! a space-separated list of words (contiguous characters). The
    //! string vector returned is generated by treating white
    //! space as delimeters between vector elements.
    //! If the element does not exist
    //! an empty vector is returned
    //!
    //! \param[in] tag Name of element
    //! \param[out] vec Vector of strings associated with the named element
    //!
    virtual void GetElementStringVec(const string &tag, vector<string> &vec) const;

    //! Return true if the named element of type string exists
    //!
    //! \param[in] tag Name of element
    //! \retval bool
    //!
    virtual bool HasElementString(const string &tag) const;

    //! Return the number of children nodes this node has
    //!
    //! \retval n The number of direct children this node has
    //!
    //! \sa NewChild()
    //

    //! Add an existing node as a child of the current node.
    //!
    //! The new child node will be cloned from \p child and
    //! appended to the array of child nodes.
    //!
    //! \param[in] child is the XmlNode object to be added as a child
    //! \retval newchild Returns a pointer to the newly created child. This
    //! is a no-fail method (NULL is never returned).
    //
    XmlNode *AddChild(
        const XmlNode *child);
    XmlNode *AddChild(
        const XmlNode &child);

    virtual int GetNumChildren() const { return (int)(_children.size()); };

    //! Create a new child of this node
    //!
    //! Create a new child node, named \p tag. The new child node will be
    //! appended to the array of child nodes. The \p numChildrenHint
    //! parameter is a hint specifying how many children the new child
    //! itself may have.
    //!
    //! \param[in] tag Name to give the new child node
    //! \param[in] attrs A list of Xml attribute names and values for this node
    //! \param[in] numChildrenHint Reserve space for future children of this node
    //! \retval child Returns the newly created child, or NULL if the child
    //! could not be created
    //
    virtual XmlNode *NewChild(
        const string &tag, const map<string, string> &attrs,
        size_t numChildrenHint = 0);

    virtual XmlNode *NewChild(
        const string &tag,
        size_t numChildrenHint = 0) {
        map<string, string> attrs;
        return (NewChild(tag, attrs, numChildrenHint));
    }

    //! Delete the indicated child node.
    //!
    //! Delete the indicated child node, decrementing the total number
    //! of children by one. Return an error if the child does not
    //! exist (i.e. if index >= GetNumChildren())
    //!
    //! \param[in] index Index of the child. The first child is zero
    //! \retval status Returns a non-negative value on success
    //! \sa GetNumChildren()
    virtual int DeleteChild(size_t index);
    virtual int DeleteChild(const string &tag);

    //!
    //! Recursively delete all descendants of a node.
    //!
    //
    virtual void DeleteAll();

#ifdef DEAD
    //! Replace the indicated child node with specified new child node
    //!
    //! If indicated child does not exist, return -1, otherwise
    //! return the index of the replaced child.
    //!
    //! \param[in] childNode Pointer to existing child node
    //! \param[in] newChild Pointer to replacement child node
    //! \retval status Returns non-negative child index on success

    virtual int ReplaceChild(XmlNode *childNode, XmlNode *newChild);
#endif

    //! Return the indicated child node.
    //!
    //! Return the ith child of this node. The first child node is index=0,
    //! the last is index=GetNumChildren()-1. Return NULL if the child
    //! does not exist.
    //!
    //! \param[in] index Index of the child. The first child is zero
    //! \retval child Returns the indicated child, or NULL if the child
    //! could does not exist. No error is generated on failure.
    //! \sa GetNumChildren()
    //
    virtual XmlNode *GetChild(size_t index) const;

    //! Return true if the indicated child node exists
    //!
    //! \param[in] index Index of the child. The first child is zero
    //! \retval bool
    //!
    virtual bool HasChild(size_t index) const;

    //! Return the indicated child node.
    //!
    //! Return the indicated tagged child node. Return NULL if the child
    //! does not exist.
    //! \param[in] tag Name of the child node to return
    //! \retval child Returns the indicated child, or NULL if the child
    //! could does not exist. No error is generated on failure.
    //
    virtual XmlNode *GetChild(const string &tag) const;

    //! Return true if the indicated child node exists
    //!
    //! \param[in] tag Name of the child node
    //! \retval bool
    //!
    virtual bool HasChild(const string &tag) const;

    //! Return a pointer to a node with the indicated path if it exists
    //!
    //! This method returns a pointer to the node with the path indicated
    //! by the path vector \p path. If \p absolute is true the search
    //! begins from the root of the root of the tree. If \p absolute
    //! is false, the search begins with the children of this node.
    //!
    //! \p retval The indicated node is returned on success. If the path
    //! is not valid NULL is returned, but no error is reported
    //!
    virtual XmlNode *GetNode(std::vector<string> path, bool absolute) const;

    //! Return a pointer to a node with the indicated path if it exists
    //!
    //! The path argument \p path is parsed into a vector of strings, with
    //! the '/' character used as a delimiter. The resulting path vector
    //! is passed to GetNode(std::vector <string> path, bool absolute).
    //! If the first character in \p path is '/' then an absolute path
    //! is assumed.
    //!
    //! \sa GetNode(std::vector <string> path, bool absolute)
    //
    virtual XmlNode *GetNode(string path) const;

    //! Return the node's parent
    //!
    //! This method returns a pointer to the parent node, or NULL if this
    //! node is the root of the tree.
    //!
    //! \retval node Pointer to parent node or NULL if no parent exists
    //
    virtual XmlNode *GetParent() const { return (_parent); }

    //! Set a node's parent
    //!
    //! Set the parent node to \p parent, adding this node to the
    //! children of \p parent, and removing it from the children of the
    //! node's current parent. If \p parent already has a child with the
    //! same name as this node that child is deleted.
    //!
    //! \p parent may be NULL in which case the node becomes a root node
    //!
    //! \sa IsRoot();
    //
    void SetParent(XmlNode *parent);

    //! Write the XML tree, rooted at this node, to a file in XML format
    //
    friend ostream &operator<<(ostream &s, const XmlNode &node);

    //! Assignment operator
    //
    XmlNode &operator=(const XmlNode &rhs);

    //! Equivalence operator
    //
    bool operator==(const XmlNode &rhs) const;
    bool operator!=(const XmlNode &rhs) const {
        return (!(*this == rhs));
    };

    //! Return boolean indicating if this node is the root of the tree
    //!
    //! This method returns true if the node is the root if the tree. I.e.
    //! true is returned if the node has no parent. Otherwise false is returned.
    //
    virtual bool IsRoot() const {
        return (GetParent() == NULL);
    }

    //! Get the path from the root to this node as a vector of tags
    //!
    //! This method returns an ordered vector of strings containing all
    //! of the XmlNode tags from the root to this node. The first string
    //! in the vector is the root node tag.
    //
    virtual std::vector<string> GetPathVec() const;

    //! Get the path from the root to this node as a delimeter separated string
    //!
    //! This method constructs a delimeter separated path from the string
    //! vector returned  by GetPathVec(). The delimiter is a '/'
    //
    virtual string GetPath() const;

    //! Get the root node of the tree
    //!
    //! Walks up the tree starting from this node and
    //! returns the root node of the tree
    //!
    virtual XmlNode *GetRoot() const;

    static const std::vector<XmlNode *> &GetAllocatedNodes() {
        return (_allocatedNodes);
    }

    //Following is a substitute for exporting the "<<" operator in windows.
    //I don't know how to export an operator<< !
    static ostream &streamOut(ostream &os, const XmlNode &node);

  private:
    static vector<long> _emptyLongVec; // empty elements
    static vector<double> _emptyDoubleVec;
    static vector<string> _emptyStringVec;
    static string _emptyString;

    static std::vector<XmlNode *> _allocatedNodes;

    map<string, vector<long>> _longmap;     // node's long data
    map<string, vector<double>> _doublemap; // node's double data
    map<string, string> _stringmap;         // node's string data
    map<string, string> _attrmap;           // node's attributes

    vector<XmlNode *> _children; // node's children
    string _tag;                 // node's tag name

    size_t _asciiLimit; // length limit beyond which element data are encoded
    XmlNode *_parent;   // Node's parent
};
//ostream& VAPoR::operator<< (ostream& os, const XmlNode& node);

class PARAMS_API XmlParser : public Wasp::MyBase {
  public:
    XmlParser();
    ~XmlParser() {}

    int LoadFromFile(XmlNode *root, string path);

  private:
    enum type {
        UNKNOWN,
        PARENT,
        LONG_DATA,
        DOUBLE_DATA,
        STRING_DATA
    };

    XmlNode *_root;
    type _nodeType;
    std::stack<XmlNode *> _nodeStack;
    string _stringData;

    void _startElementHandler(string tag, map<string, string> &myattrs);
    void _endElementHandler(string tag);
    void _charDataHandler(string s);

    bool _isParentElement(string tag, map<string, string> myattrs) const;
    bool _isDataElement(
        string tag, map<string, string> myattrs, type &dtype) const;

    friend void _StartElementHandler(
        void *userData, const char *tag, const char **attrs);

    friend void _EndElementHandler(void *userData, const char *tag);

    friend void _CharDataHandler(void *userData, const char *s, int len);
};

}; // namespace VAPoR

#endif //	_XmlNode_h_
