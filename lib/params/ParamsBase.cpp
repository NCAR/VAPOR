//************************************************************************
//									*
//		     Copyright (C)  2008				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									 *
//************************************************************************/
//
//	File:		ParamsBase.cpp
//
//	Author:		John Clyne with modifications by Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		March 2008
//
//	Description:	Implements the ParamsBase class
//		This is  abstract class for classes that use a ParamNode
//		to support get/set functionality, and to support dirty bits
//		It is a base class for Params, MapperFunction and other classes
//		that retain their state in xml nodes
//
#include <string>
#include <cassert>
#include <functional>
#include <vapor/XmlNode.h>
#include <vapor/ParamsBase.h>


using namespace VAPoR;


ParamsBase::ParamsBase(
	StateSave *ssave, const string &classname
) {
	assert(ssave != NULL);

	_ssave = ssave;
	_node = NULL;

	map <string, string> attrs;
	_node = new XmlNode(classname, attrs);
}

ParamsBase::ParamsBase(
	StateSave *ssave, XmlNode *node
) {
	assert(ssave != NULL);
	assert(node != NULL);

	_ssave = ssave;
	_node = node;

}

ParamsBase::ParamsBase(
	const ParamsBase &rhs
) {
	_ssave = rhs._ssave;
	_node = NULL;

	_node = new XmlNode(*(rhs._node));
}

ParamsBase::ParamsBase(StateSave *ssave) {
	assert(ssave != NULL);

	_ssave = ssave;
	_node = NULL;

}

ParamsBase &ParamsBase::operator=( const ParamsBase& rhs ) 
{
	MyBase::operator=(rhs);

	if (_node) {
		_node->SetParent(NULL);
		delete _node;
	}

	_ssave = rhs._ssave;
	_node = NULL;

	_node = new XmlNode(*(rhs._node));

	return(*this);
}



ParamsBase::~ParamsBase() {

	// Only delete the Xml tree if this is the root node
	//
	if (_node && _node->IsRoot()) {
		delete _node;
	}

	_node = NULL;
	_ssave = NULL;

}

void ParamsBase::SetParent(ParamsBase *parent) {

	GetNode()->SetParent(parent ? parent->GetNode() : NULL);
	_ssave->Save(GetNode(), "Set parent node");
}

vector <long> ParamsBase::GetValueLongVec( const string tag) const {

	vector <long> empty;
	if (! _node->HasElementLong(tag)) return(empty);

	return(_node->GetElementLong(tag));
}

vector <long> ParamsBase::GetValueLongVec(
	const string tag, const vector<long>& defaultVal
) const {
	if (! _node->HasElementLong(tag)) return(defaultVal);

	vector <long> v = _node->GetElementLong(tag);
	if (v.size() < defaultVal.size()) {
		for (int i=v.size(); i<defaultVal.size(); i++) {
			v.push_back(defaultVal[i]);
		}
	} 
	else if (v.size() > defaultVal.size()) {
		while(v.size() > defaultVal.size()) {
			v.pop_back();
		}
	}

	return(v);
}

long ParamsBase::GetValueLong(
	const string tag, long defaultVal
) const {

	vector <long> defaultValVec(1, defaultVal);
	vector <long> v = ParamsBase::GetValueLongVec(tag, defaultValVec);

	if (! v.size()) return (defaultVal);

	return(v[0]);
}

vector <double> ParamsBase::GetValueDoubleVec( const string tag) const {

	vector <double> empty;

	assert(_node);

	bool test = _node->HasElementDouble(tag);
	if (!test) return (empty);
	//if (! _node->HasElementDouble(tag)) return(empty);

	return(_node->GetElementDouble(tag));
}

vector <double> ParamsBase::GetValueDoubleVec( const string tag, 
                                               const vector<double>& defaultVal) const 
{
	if (! _node->HasElementDouble(tag)) return(defaultVal);

	vector <double> v = _node->GetElementDouble(tag);
	if (v.size() < defaultVal.size()) {
		for (int i=v.size(); i<defaultVal.size(); i++) {
			v.push_back(defaultVal[i]);
		}
	} 
	else if (v.size() > defaultVal.size()) {
		while(v.size() > defaultVal.size()) {
			v.pop_back();
		}
	}

	return(v);
}

double ParamsBase::GetValueDouble(
	const string tag, double defaultVal
) const {

	vector <double> defaultValVec(1, defaultVal);
	vector <double> v = ParamsBase::GetValueDoubleVec(tag, defaultValVec);

	if (! v.size()) return (defaultVal);

	return(v[0]);
}

vector <string> ParamsBase::GetValueStringVec( const string tag) const {

	vector <string> empty;
	if (! _node->HasElementString(tag)) return(empty);

	vector <string> v;
	_node->GetElementStringVec(tag, v);
	return(v);
}

vector <string> ParamsBase::GetValueStringVec(
	const string tag, const vector<string>& defaultVal
) const {
	if (! _node->HasElementString(tag)) return(defaultVal);

	vector <string> v;
	_node->GetElementStringVec(tag, v);
	if (v.size() < defaultVal.size()) {
		for (int i=v.size(); i<defaultVal.size(); i++) {
			v.push_back(defaultVal[i]);
		}
	} 
	else if (v.size() > defaultVal.size()) {
		while(v.size() > defaultVal.size()) {
			v.pop_back();
		}
	}

	return(v);
}

string ParamsBase::GetValueString(
	const string tag, string defaultVal
) const {

	if (! _node->HasElementString(tag)) return(defaultVal);

	return(_node->GetElementString(tag));
}


void ParamsBase::SetValueLong(
	const string &tag, string description, long value
) {
	vector <long> values(1,value);
	ParamsBase::SetValueLongVec(tag, description, values);
}

void ParamsBase::SetValueLongVec(
	const string &tag, string description, const vector<long> &values
) {
	vector <long> current = GetValueLongVec(tag);
	if (current == values) return;

	_node->SetElementLong(tag, values);

	_ssave->Save(_node, description);
}

void ParamsBase::SetValueDouble(
	const string &tag, string description, double value
) {
	vector <double> values(1,value);
	ParamsBase::SetValueDoubleVec(tag, description, values);
}

void ParamsBase::SetValueDoubleVec(
	const string &tag, string description, const vector<double> &values
) {
	vector <double> current = GetValueDoubleVec(tag);
	if (current == values) return;


	_node->SetElementDouble(tag, values);

	_ssave->Save(_node, description);
}

void ParamsBase::SetValueString(
	const string &tag, string description, const string &value
) {
	vector <string> values(1,value);
	ParamsBase::SetValueStringVec(tag, description, values);
}

void ParamsBase::SetValueStringVec(
	const string &tag, string description, const vector <string> &values
) {
	vector <string> current = GetValueStringVec(tag);
	if (current == values) return;


	_node->SetElementStringVec(tag, values);

	_ssave->Save(_node, description);
}

//////////////////////////////////////////////////////////////////////////
//
// ParamsSeparator Class
//
/////////////////////////////////////////////////////////////////////////

ParamsSeparator::ParamsSeparator(
	StateSave *ssave, const string &name
) : ParamsBase(ssave, name) {

}

ParamsSeparator::ParamsSeparator(
	StateSave *ssave, XmlNode *node
) : ParamsBase(ssave, node) {

}

ParamsSeparator::ParamsSeparator(
	ParamsSeparator *parent, const string &name
) : ParamsBase(parent->_ssave) {

	if (parent->GetNode()->HasChild(name)) {
		_node = parent->GetNode()->GetChild(name);
		assert(_node);
	}
	else {
		_node = parent->GetNode()->NewChild(name);
		parent->_ssave->Save(parent->GetNode(), "New params");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ParamsFactory Class
//
/////////////////////////////////////////////////////////////////////////


ParamsBase *ParamsFactory::CreateInstance(
	string className, ParamsBase::StateSave *ssave, XmlNode *node
) {
    ParamsBase * instance = NULL;

    // find className in the registry and call factory method.
	//
    auto it = m_factoryFunctionRegistry.find(className);
    if(it != m_factoryFunctionRegistry.end())
        instance = it->second(ssave, node);

    if(instance != NULL)
        return instance;
    else
        return NULL;
}

vector <string> ParamsFactory::GetFactoryNames() const {
	vector <string> names;
	map<string, function<ParamsBase * (ParamsBase::StateSave *, XmlNode *)>>::const_iterator itr;

	for (
		itr = m_factoryFunctionRegistry.begin();
		itr!=m_factoryFunctionRegistry.end();
		++itr
	) {
		names.push_back(itr->first);
	}
	return(names);
}

//////////////////////////////////////////////////////////////////////////
//
// ParamsContainer Class
//
/////////////////////////////////////////////////////////////////////////

ParamsContainer::ParamsContainer(
	ParamsBase::StateSave *ssave, const string &name
) {
	assert(ssave != NULL);

	_ssave = ssave;
	_separator = NULL;
	_elements.clear();

	_separator = new ParamsSeparator(ssave, name);

}

ParamsContainer::ParamsContainer(
	ParamsBase::StateSave *ssave, XmlNode *node
) {
	assert(ssave != NULL);
	assert(node != NULL);

	_ssave = ssave;
	_separator = new ParamsSeparator(ssave, node);
	_elements.clear();

	for (int i=0; i<node->GetNumChildren(); i++) {
		XmlNode *eleNameNode = node->GetChild(i);
		if (! eleNameNode->HasChild(0)) continue;	// bad node

		XmlNode *eleNode = eleNameNode->GetChild(0);

		string eleName = eleNameNode->GetTag();
		string classname = eleNode->GetTag();

		ParamsBase *pB = ParamsFactory::Instance()->CreateInstance(
			classname, ssave, eleNode
		);
		if (pB == NULL) {
			SetDiagMsg(
				"ParamsContainer::ParamsContainer() unrecognized class: %s",
				classname.c_str()
			);
			continue;
		}
				
		_elements[eleName] = pB;
	}
}

ParamsContainer::ParamsContainer(
	const ParamsContainer &rhs
) {
	_ssave = rhs._ssave;
	_separator = NULL;
	_elements.clear();

	_ssave->BeginGroup("Params container");

	_separator = new ParamsSeparator(*(rhs._separator));
	_separator->SetParent(NULL);

	vector <string> names = rhs.GetNames();
	for (int i=0; i<names.size(); i++) {
		
		// Make copy of ParamsBase
		//
		ParamsBase *rhspb = rhs.GetParams(names[i]);
		XmlNode *node = new XmlNode(*(rhspb->GetNode()));

		string classname = rhspb->GetName();
		ParamsBase *mypb = ParamsFactory::Instance()->CreateInstance(
			classname, _ssave, node
		);
		mypb->SetParent(_separator);

		_elements[names[i]] = mypb;
	}

	_ssave->EndGroup();
}

ParamsContainer &ParamsContainer::operator=( const ParamsContainer& rhs ) 
{
	assert(_separator);

	vector <string> mynames = GetNames();
	for (int i=0; i<mynames.size(); i++) {
		Remove(mynames[i]);
	}
	_elements.clear();

	_ssave->BeginGroup("Params container");

	_separator = rhs._separator;
	_ssave = rhs._ssave;


	vector <string> names = rhs.GetNames();
	for (int i=0; i<names.size(); i++) {
		XmlNode *eleNameNode = _separator->GetNode()->GetChild(names[i]);
		assert (eleNameNode);
		
		ParamsSeparator mySep(_ssave, eleNameNode);

		XmlNode *eleNode = eleNameNode->GetChild(0);

		string eleName = eleNameNode->GetTag();
		string classname = eleNode->GetTag();

		ParamsBase *mypb = ParamsFactory::Instance()->CreateInstance(
			classname, _ssave, eleNode
		);
		mypb->SetParent(&mySep);

		_elements[names[i]] = mypb;
	}

	_ssave->EndGroup();

	return(*this);
}

ParamsContainer::~ParamsContainer() {

	map <string, ParamsBase *>::iterator itr;
	for (itr = _elements.begin(); itr != _elements.end(); ++itr) {
		if (itr->second) delete itr->second;
	}

	if (_separator) delete _separator;

}

ParamsBase *ParamsContainer::Insert(ParamsBase *pb, string name) {
	assert(pb != NULL);

	map <string, ParamsBase *>::iterator itr = _elements.find(name);
	if (itr != _elements.end()) {
		delete itr->second;
	}

	_ssave->BeginGroup("Params container");

	// Create a separator node
	//
	ParamsSeparator mySep(_ssave, name);
	mySep.SetParent(_separator);

	// Create element name node
	//
	string classname = pb->GetName();
	XmlNode *node = new XmlNode(*(pb->GetNode()));
	ParamsBase *mypb = ParamsFactory::Instance()->CreateInstance(
            classname, _ssave, node
	);
	assert(mypb != NULL);
	mypb->SetParent(&mySep);

	_elements[name] = mypb;

	_ssave->EndGroup();

	return(mypb);

}

ParamsBase *ParamsContainer::Create(string className, string name) {

    map <string, ParamsBase *>::iterator itr = _elements.find(name);
    if (itr != _elements.end()) {
        delete itr->second;
    }

	_ssave->BeginGroup("Params container");

	// Create a separator node
	//
	ParamsSeparator mySep(_ssave, name);
	mySep.SetParent(_separator);

	// Create the desired class
	//
    ParamsBase *mypb = ParamsFactory::Instance()->CreateInstance(
            className, _ssave, NULL
    );
    assert(mypb != NULL);

    mypb->SetParent(&mySep);

	_elements[name] = mypb;

	_ssave->EndGroup();

    return(mypb);
}

void ParamsContainer::Remove(string name) {


	map <string, ParamsBase *>::iterator itr = _elements.find(name);
	if (itr == _elements.end()) return;

	ParamsBase *mypb = itr->second;

	// Set parent to root so  Xml representation will be deleted
	//
	mypb->SetParent(NULL);
	delete mypb;

	_elements.erase(itr);

	_ssave->Save(_separator->GetNode(), "Delete params");

}

ParamsBase *ParamsContainer::GetParams(string name) const {
	map <string, ParamsBase *>::const_iterator itr = _elements.find(name);
	if (itr != _elements.end())  return(itr->second);

	return(NULL);
}

string ParamsContainer::GetParamsName(const ParamsBase *pb) const {
	map <string, ParamsBase *>::const_iterator itr;
	for (itr = _elements.begin(); itr != _elements.end(); ++itr) {
		if (itr->second == pb) return(itr->first);
	}

	return("");
}

vector <string> ParamsContainer::GetNames() const {
	map <string, ParamsBase *>::const_iterator itr; 

	vector <string> names;
	for (itr=_elements.begin(); itr!=_elements.end(); ++itr) {
		names.push_back(itr->first);
	}

	return(names);
}

