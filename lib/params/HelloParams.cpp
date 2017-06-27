
#include <string>
#include <vapor/HelloParams.h>


using namespace Wasp;
using namespace VAPoR;

//Provide values for all the tags
const string HelloParams::m_lineThicknessTag = "LineThickness";
const string HelloParams::m_point1Tag = "Point1";
const string HelloParams::m_point2Tag = "Point2";

//
// Register class with object factory!!!
//
static RenParamsRegistrar<HelloParams> registrar(HelloParams::GetClassType());



HelloParams::HelloParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave
) : RenderParams(dataMgr, ssave, HelloParams::GetClassType()) {
	
	//restart provides default values even with no data mgr
	_init();
}

HelloParams::HelloParams(
	DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node
) : RenderParams(dataMgr, ssave, node) {

	// If node isn't tagged correctly we correct the tag and reinitialize
	// from scratch;
	//
	if (node->GetTag() != HelloParams::GetClassType()) {
		node->SetTag(HelloParams::GetClassType());
		_init();
	}
}


HelloParams::~HelloParams() {
}


//Set everything to default values.
//Data Manager is absent.
void HelloParams::_init() {
	SetLineThickness(1.0);

	vector <double> pt1;
	vector <double> pt2;

	GetBox()->GetExtents(pt1, pt2);

	SetPoint1(pt1);
	SetPoint2(pt2);
}
