//
//      $Id$
//
//************************************************************************
//								 *
//		     Copyright (C)  2004			*
//     University Corporation for Atmospheric Research		*
//		     All Rights Reserved			*
//								*
//************************************************************************/
//
//	File:		XmlNode.cpp
//
//	Author:		John Clyne
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		Thu Sep 30 14:06:17 MDT 2004
//
//	Description:
//
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <expat.h>
#include <vapor/XmlNode.h>

using namespace VAPoR;
using namespace Wasp;

// Enable memory checking
//
#define MEMCHECK

namespace VAPoR {
vector<double>         XmlNode::_emptyDoubleVec;
vector<long>           XmlNode::_emptyLongVec;
vector<string>         XmlNode::_emptyStringVec;
string                 XmlNode::_emptyString;
std::vector<XmlNode *> XmlNode::_allocatedNodes;
};    // namespace VAPoR

namespace {

string       TypeAttr = "Type";
const string LongType = "Long";
const string DoubleType = "Double";
const string StringType = "String";
};    // namespace

XmlNode::XmlNode(const string &tag, const map<string, string> &attrs, size_t numChildrenHint)
{
    _longmap.clear();
    _doublemap.clear();
    _stringmap.clear();
    _attrmap.clear();
    _children.clear();
    _tag.clear();
    _asciiLimit = 1024;
    _parent = NULL;

    _tag = tag;
    _attrmap = attrs;

    if (numChildrenHint) _children.reserve(numChildrenHint);

#ifdef MEMCHECK
    _allocatedNodes.push_back(this);
#endif
}

XmlNode::XmlNode(const string &tag, size_t numChildrenHint)
{
    _longmap.clear();
    _doublemap.clear();
    _stringmap.clear();
    _attrmap.clear();
    _children.clear();
    _tag.clear();
    _asciiLimit = 1024;
    _parent = NULL;

    _tag = tag;

    if (numChildrenHint) _children.reserve(numChildrenHint);

#ifdef MEMCHECK
    _allocatedNodes.push_back(this);
#endif
}

XmlNode::XmlNode()
{
    _longmap.clear();
    _doublemap.clear();
    _stringmap.clear();
    _attrmap.clear();
    _children.clear();
    _tag.clear();
    _asciiLimit = 1024;
    _parent = NULL;

#ifdef MEMCHECK
    _allocatedNodes.push_back(this);
#endif
}

XmlNode::XmlNode(const XmlNode &rhs)
: _longmap(rhs._longmap), _doublemap(rhs._doublemap), _stringmap(rhs._stringmap), _attrmap(rhs._attrmap), _children(rhs._children), _tag(rhs._tag), _asciiLimit(rhs._asciiLimit),
  _parent(NULL)    // Set parent to NULL
{
    _children.clear();
    for (int i = 0; i < rhs._children.size(); i++) { AddChild(rhs._children[i]); }

#ifdef MEMCHECK
    _allocatedNodes.push_back(this);
#endif
}

XmlNode &XmlNode::operator=(const XmlNode &rhs)
{
    DeleteAll();
    MyBase::operator=(rhs);

    _children.clear();
    _longmap = rhs._longmap;
    _doublemap = rhs._doublemap;
    _stringmap = rhs._stringmap;
    _attrmap = rhs._attrmap;
    _children = rhs._children;
    _tag = rhs._tag;
    _asciiLimit = rhs._asciiLimit;
    _parent = NULL;    // Set parent to NULL

    _children.clear();
    for (int i = 0; i < rhs._children.size(); i++) { AddChild(rhs._children[i]); }

    return (*this);
}

bool XmlNode::operator==(const XmlNode &rhs) const
{
    if (_longmap != rhs._longmap) return (false);
    if (_doublemap != rhs._doublemap) return (false);
    if (_stringmap != rhs._stringmap) return (false);
    if (_attrmap != rhs._attrmap) return (false);
    if (_tag != rhs._tag) return (false);

    if (_children.size() != rhs._children.size()) return (false);
    for (int i = 0; i < _children.size(); i++) {
        // Recusrively call '==' operator
        //
        if (!(*(_children[i]) == *(rhs._children[i]))) return (false);
    }

    // Don't check parent!!!
    //
    // if (_parent != rhs._parent) return(false);

    return (true);
}

XmlNode::~XmlNode()
{
    DeleteAll();

#ifdef MEMCHECK
    std::vector<XmlNode *>::iterator itr;
    for (itr = _allocatedNodes.begin(); itr != _allocatedNodes.end(); ++itr) {
        if (*itr == this) {
            _allocatedNodes.erase(itr);
            break;
        }
    }
#endif
}

void XmlNode::SetElementLong(const string &tag, const vector<long> &values) { _longmap[tag] = values; }

void XmlNode::SetElementLong(const vector<string> &tags, const vector<long> &values)
{
    // Iterate through tags, finding associated node
    XmlNode *currNode = this;
    for (int i = 0; i < tags.size() - 1; i++) {
        XmlNode *child = currNode->GetChild(tags[i]);
        if (!child) { child = currNode->NewChild(tags[i]); }
        currNode = child;
    }

    string tag = tags[tags.size() - 1];
    currNode->_longmap[tag] = values;
}

void XmlNode::SetElementDouble(const vector<string> &tags, const vector<double> &values)
{
    // Iterate through tags, finding associated node
    XmlNode *currNode = this;
    for (int i = 0; i < tags.size() - 1; i++) {
        XmlNode *child = currNode->GetChild(tags[i]);
        if (!child) { child = currNode->NewChild(tags[i]); }
        currNode = child;
    }
    string tag = tags[tags.size() - 1];
    currNode->_doublemap[tag] = values;
}

const vector<long> &XmlNode::GetElementLong(const string &tag) const
{
    map<string, vector<long>>::const_iterator p = _longmap.find(tag);

    // see if entry for this key (tag) already exists
    //
    if (p == _longmap.end()) { return (_emptyLongVec); }

    return (p->second);
}

bool XmlNode::HasElementLong(const string &tag) const
{
    map<string, vector<long>>::const_iterator p = _longmap.find(tag);
    return (p != _longmap.end());
}

void XmlNode::SetElementDouble(const string &tag, const vector<double> &values) { _doublemap[tag] = values; }

const vector<double> &XmlNode::GetElementDouble(const string &tag) const
{
    map<string, vector<double>>::const_iterator p = _doublemap.find(tag);

    // see if entry for this key (tag) already exists
    //
    if (p == _doublemap.end()) { return (_emptyDoubleVec); }

    return (p->second);
}

bool XmlNode::HasElementDouble(const string &tag) const
{
    if (_doublemap.empty()) return false;
    map<string, vector<double>>::const_iterator p = _doublemap.find(tag);
    return (p != _doublemap.end());
}

void XmlNode::SetElementString(const string &tag, const string &str) { _stringmap[tag] = str; }

void XmlNode::SetElementStringVec(const string &tag, const vector<string> &strvec)
{
    string s;
    for (int i = 0; i < strvec.size(); i++) {
        s.append(strvec[i]);
        if (i < strvec.size() - 1) s.append(" ");
    }

    return (XmlNode::SetElementString(tag, s));
}

void XmlNode::SetElementStringVec(const vector<string> &tags, const vector<string> &strvec)
{
    // Iterate through tags, finding associated node
    XmlNode *currNode = this;
    for (int i = 0; i < tags.size() - 1; i++) {
        XmlNode *child = currNode->GetChild(tags[i]);
        if (!child) { child = currNode->NewChild(tags[i]); }
        currNode = child;
    }
    string tag = tags[tags.size() - 1];

    string s;
    for (int i = 0; i < strvec.size(); i++) {
        s.append(strvec[i]);
        if (i < strvec.size() - 1) s.append(" ");
    }
    return (currNode->SetElementString(tag, s));
}

const string &XmlNode::GetElementString(const string &tag) const
{
    map<string, string>::const_iterator p = _stringmap.find(tag);

    // see if entry for this key (tag) already exists
    //
    if (p == _stringmap.end()) { return (_emptyString); }

    return (p->second);
}

void XmlNode::GetElementStringVec(const string &tag, vector<string> &vec) const
{
    string s = XmlNode::GetElementString(tag);

    StrToWordVec(s, vec);
}

bool XmlNode::HasElementString(const string &tag) const
{
    map<string, string>::const_iterator p = _stringmap.find(tag);
    return (p != _stringmap.end());
}

XmlNode *XmlNode::NewChild(const string &tag, const map<string, string> &attrs, size_t numChildrenHint)
{
    // Delete duplicates
    //
    if (HasChild(tag)) { DeleteChild(tag); }

    XmlNode *mychild = Construct(tag, attrs, numChildrenHint);

    mychild->_parent = this;

    _children.push_back(mychild);
    return (mychild);
}

XmlNode *XmlNode::AddChild(const XmlNode *child) { return (AddChild(*child)); }

XmlNode *XmlNode::AddChild(const XmlNode &child)
{
    XmlNode *mychild = new XmlNode(child);

    // Delete duplicates
    //
    if (HasChild(mychild->Tag())) { DeleteChild(mychild->Tag()); }

    mychild->_parent = this;

    _children.push_back(mychild);
    return (mychild);
}

#ifdef DEAD
int XmlNode::ReplaceChild(XmlNode *prevChildNode, XmlNode *newChildNode)
{
    for (int index = 0; index < _children.size(); index++) {
        XmlNode *node = _children[index];
        if (node == prevChildNode) {
            delete node;
            _children[index] = new XmlNode(*newChildNode);
            newChildNode->_parent = this;

            return index;
        }
    }
    SetErrMsg("Node not found");
    return (-1);
}
#endif

int XmlNode::DeleteChild(size_t index)
{
    if (index >= _children.size()) {
        SetErrMsg("Invalid child id : %d", index);
        return (-1);
    }

    XmlNode *node = _children[index];
    assert(node);

    // Remove from parent's list of children and
    // recursively delete this node's children, if any
    //
    node->SetParent(NULL);
    delete node;

    return (0);
}

int XmlNode::DeleteChild(const string &tag)
{
    XmlNode *child;

    for (size_t i = 0; i < _children.size(); i++) {
        child = GetChild(i);
        assert(child);

        if (StrCmpNoCase(child->_tag, tag) == 0) { return (XmlNode::DeleteChild(i)); }
    }

    SetErrMsg("Invalid child name (does not exist) : %s", tag.c_str());
    return (-1);
}

XmlNode *XmlNode::GetChild(size_t index) const
{
    if (index >= _children.size()) {
        SetErrMsg("Invalid child id : %d", index);
        return (NULL);
    }

    return (_children[index]);
}

bool XmlNode::HasChild(size_t index) const { return (index < _children.size()); }

XmlNode *XmlNode::GetChild(const string &tag) const
{
    XmlNode *child;

    for (size_t i = 0; i < _children.size(); i++) {
        if (!(child = GetChild(i))) return (NULL);

        if (StrCmpNoCase(child->_tag, tag) == 0) return (child);
    }

    return (NULL);
}

bool XmlNode::HasChild(const string &tag) const
{
    XmlNode *child = NULL;

    for (size_t i = 0; i < _children.size(); i++) {
        child = GetChild(i);
        assert(child != NULL);

        if (StrCmpNoCase(child->_tag, tag) == 0) return (true);
    }
    return (false);
}

XmlNode *XmlNode::GetNode(std::vector<string> path, bool absolute) const
{
    if (path.empty()) return (NULL);

    const XmlNode *node = this;

    // If absolute path find the root of the tree
    //
    if (absolute) {
        while (node->GetParent() != NULL) { node = node->GetParent(); }
        if (node->GetTag() != path[0]) return (NULL);
        path.erase(path.begin());
    }

    // Now walk the path down to the last node in the path
    //
    for (int i = 0; i < path.size(); i++) {
        if (!node->HasChild(path[i])) return (NULL);

        node = node->GetChild(path[i]);
    }

    return ((XmlNode *)node);
}

XmlNode *XmlNode::GetNode(string path) const
{
    if (path.empty()) return (NULL);

    vector<string> pathvec;
    Wasp::SplitString(path, '/', pathvec);

    // Absolute or relative path?
    //
    if (path[0] == '/') {
        return (GetNode(pathvec, true));
    } else {
        return (GetNode(pathvec, false));
    }
}

void XmlNode::SetParent(XmlNode *parent)
{
    // No-op case
    //
    if (parent == _parent) return;

    // Delete duplicates on new parent
    //
    if (parent && parent->HasChild(Tag())) { parent->DeleteChild(Tag()); }

    // Remove from current parent's list of children
    //
    if (_parent) {
        vector<XmlNode *>::iterator itr = _parent->_children.begin();
        for (; itr != _parent->_children.end(); ++itr) {
            XmlNode *node = *itr;
            if (node->Tag() == Tag()) {
                _parent->_children.erase(itr);
                break;
            }
        }
    }

    // If new parent is not NULL
    //
    if (parent) { parent->_children.push_back(this); }

    _parent = parent;
}

// Recursively delete all descendants of this node
//
void XmlNode::DeleteAll()
{
    for (int i = 0; i < (int)_children.size(); i++) {
        if (_children[i]) {
            XmlNode *node = _children[i];
            assert(node);

            delete node;
        }
    }
    _children.clear();
}

vector<string> XmlNode::GetPathVec() const
{
    vector<string> path;
    path.push_back(GetTag());

    const XmlNode *child = this;
    const XmlNode *parent = NULL;
    while ((parent = child->GetParent()) != NULL) {
        path.push_back(parent->GetTag());
        child = parent;
    }

    // Reverse so root is first
    //
    reverse(path.begin(), path.end());

    return (path);
}

string XmlNode::GetPath() const
{
    vector<string> tags = GetPathVec();
    string         path;

    for (int i = 0; i < tags.size(); i++) {
        path += "/";
        path += tags[i];
    }
    return (path);
}

XmlNode *XmlNode::GetRoot() const
{
    const XmlNode *node = this;    // Why must node be const?
    while (node->GetParent()) { node = node->GetParent(); }
    return ((XmlNode *)node);
}

ostream &XmlNode::streamOut(ostream &os, const XmlNode &node)
{
    os << node;
    return os;
}

namespace VAPoR {
std::ostream &operator<<(ostream &os, const VAPoR::XmlNode &node)
{
    map<string, vector<long>>::const_iterator   plong;
    map<string, vector<double>>::const_iterator pdouble;
    map<string, string>::const_iterator         pstring;
    map<string, string>::const_iterator         pattr;

    int i;

    //	os.setf(ios_base::scientific, ios_base::floatfield);
    os.precision(16);

    plong = node._longmap.begin();
    pdouble = node._doublemap.begin();
    pstring = node._stringmap.begin();
    pattr = node._attrmap.begin();

    os << "<" << node._tag;

    for (; pattr != node._attrmap.end(); pattr++) {
        const string &name = pattr->first;
        const string &value = pattr->second;

        os << " " << name << "=\"" << value << "\" ";
    }
    os << ">" << endl;

    for (; plong != node._longmap.end(); plong++) {
        const string &tag = plong->first;

        const vector<long> &v = plong->second;

        os << "<" << tag << " Type=\"Long\">" << endl << "  ";

        for (i = 0; i < (int)v.size(); i++) { os << v[i] << " "; }
        os << endl;

        os << "</" << tag << ">" << endl;
    }

    for (; pdouble != node._doublemap.end(); pdouble++) {
        const string &tag = pdouble->first;

        os << "<" << tag << " Type=\"Double\">" << endl << "  ";

        const vector<double> &v = pdouble->second;

        for (i = 0; i < (int)v.size(); i++) { os << v[i] << " "; }
        os << endl;

        os << "</" << tag << ">" << endl;
    }

    for (; pstring != node._stringmap.end(); pstring++) {
        const string &tag = pstring->first;

        os << "<" << tag << " Type=\"String\">" << endl << "  ";

        const string &v = pstring->second;

        os << v;

        os << endl;

        os << "</" << tag << ">" << endl;
    }

    if (node._children.size()) {
        for (int i = 0; i < (int)node._children.size(); i++) {
            XmlNode *childptr;

            childptr = node._children[i];
            os << *childptr;
        }
    }

    os << "</" << node._tag << ">" << endl;

    return (os);
}

void _StartElementHandler(void *userData, const char *tag, const char **attrs)
{
    XmlParser *parser = (XmlParser *)userData;
    string     mytag(tag);

    map<string, string> myattrs;
    while (*attrs) {
        string key = *attrs;
        attrs++;
        assert(*attrs);
        string value = *attrs;
        attrs++;
        myattrs[key] = value;
    }
    parser->_startElementHandler(mytag, myattrs);
}

void _EndElementHandler(void *userData, const char *tag)
{
    XmlParser *parser = (XmlParser *)userData;
    string     mytag(tag);

    parser->_endElementHandler(mytag);
}

void _CharDataHandler(void *userData, const char *s, int len)
{
    XmlParser *parser = (XmlParser *)userData;
    string     mys(s, len);

    parser->_charDataHandler(mys);
}

};    // namespace VAPoR

XmlParser::XmlParser()
{
    _root = NULL;
    _nodeType = UNKNOWN;
    //_nodeStack.clear(); // No stack clear! Sigh
    _stringData.clear();
}

int XmlParser::LoadFromFile(XmlNode *node, string path)
{
    assert(node != NULL);

    _root = node;
    _nodeType = UNKNOWN;
    _stringData.clear();

    //_nodeStack.clear();
    while (_nodeStack.size()) _nodeStack.pop();

    // Delete all current children
    //
    _root->DeleteAll();

    ifstream in(path);
    if (!in) {
        SetErrMsg("Failed to open file %s : %M", path.c_str());
        return (-1);
    }

    XML_Parser expatParser = XML_ParserCreate(NULL);
    assert(expatParser != NULL);

    XML_SetElementHandler(expatParser, _StartElementHandler, _EndElementHandler);
    XML_SetCharacterDataHandler(expatParser, _CharDataHandler);

    XML_SetUserData(expatParser, (void *)this);

    // Parse the file until we run out of elements or a parsing error occurs
    //
    char line[1024];
    while (in.good()) {
        int rc;
        in.read(line, sizeof(line) - 1);
        if ((rc = in.gcount()) > 0) {
            if (XML_Parse(expatParser, line, rc, 0) == XML_STATUS_ERROR) {
                SetErrMsg("Error parsing xml file at line %d : %s", XML_GetCurrentLineNumber(expatParser), XML_ErrorString(XML_GetErrorCode(expatParser)));
                XML_ParserFree(expatParser);
                return (-1);
            }
        }
    }
    XML_ParserFree(expatParser);

    if (in.bad() || !_nodeStack.empty()) {
        SetErrMsg("IO Error reading XML file");
        return (-1);
    }
    in.close();

    return (0);
}

void XmlParser::_startElementHandler(string tag, map<string, string> &myattrs)
{
    _nodeType = UNKNOWN;
    _stringData.clear();

    type dtype;
    if (_isParentElement(tag, myattrs)) {
        _nodeType = PARENT;
        XmlNode *parent = NULL;
        if (_nodeStack.empty()) {
            parent = _root;
            parent->Tag() = tag;
            parent->Attrs() = myattrs;
            _nodeStack.push(parent);
        } else {
            XmlNode child(tag, myattrs);
            parent = _nodeStack.top();
            XmlNode *childptr = parent->AddChild(&child);
            _nodeStack.push(childptr);
        }
    } else if (_isDataElement(tag, myattrs, dtype)) {
        assert(!_nodeStack.empty());
        _nodeType = dtype;
    }
    // cout << "_startElementHandler() tag, type " << tag << " " << _nodeType << endl;
}

void XmlParser::_endElementHandler(string tag)
{
    assert(!_nodeStack.empty());

    // cout << "_endElementHandler() tag, type /" << tag << " " << _nodeType << endl;

    // remove leading and trailing white space from the XML char data
    //
    StrRmWhiteSpace(_stringData);

    XmlNode *node = _nodeStack.top();

    switch (_nodeType) {
    case PARENT:
        assert(tag == node->Tag());
        _nodeStack.pop();
        break;

    case LONG_DATA: {
        istringstream ist(_stringData);
        long          v;
        vector<long>  values;
        while (ist >> v) values.push_back(v);

        node->SetElementLong(tag, values);
        break;
    }
    case DOUBLE_DATA: {
        istringstream  ist(_stringData);
        double         v;
        vector<double> values;
        while (ist >> v) values.push_back(v);

        node->SetElementDouble(tag, values);
        break;
    }
    case STRING_DATA: node->SetElementString(tag, _stringData); break;
    default: assert(0);
    }
    _nodeType = PARENT;
}

void XmlParser::_charDataHandler(string s)
{
    if (!(_nodeType == LONG_DATA || _nodeType == DOUBLE_DATA || _nodeType == STRING_DATA)) return;

    // cout << "_CharDataHandler() : " << "\""<<s<<"\"" << endl;

    _stringData.append(s);
}

bool XmlParser::_isParentElement(string tag, map<string, string> myattrs) const
{
    type dtype;

    if (_isDataElement(tag, myattrs, dtype)) return (false);

    return (true);
}

bool XmlParser::_isDataElement(string tag, map<string, string> myattrs, type &dtype) const
{
    dtype = UNKNOWN;

    if (myattrs.size() != 1) return false;

    map<string, string>::const_iterator itr = myattrs.begin();
    if (StrCmpNoCase(itr->first, TypeAttr) != 0) return false;

    if (StrCmpNoCase(itr->second, LongType) == 0) {
        dtype = LONG_DATA;
        return true;
    } else if (StrCmpNoCase(itr->second, DoubleType) == 0) {
        dtype = DOUBLE_DATA;
        return true;
    } else if (StrCmpNoCase(itr->second, StringType) == 0) {
        dtype = STRING_DATA;
        return true;
    }

    return (false);
}
