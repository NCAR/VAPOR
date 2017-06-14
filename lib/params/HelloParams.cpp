
#include <string>
#include <vapor/DataStatus.h>
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
    DataMgr *dataMgr, ParamsBase::StateSave *ssave) : RenderParams(dataMgr, ssave, HelloParams::GetClassType()) {

    //restart provides default values even with no data mgr
    _init();
}

HelloParams::HelloParams(
    DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node) : RenderParams(dataMgr, ssave, node) {

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

#ifdef DEAD
//Initialize for new metadata.
void HelloParams::
    Validate(int type) {
    //Command capturing should already be disabled
    assert(!Command::isRecording());
    bool doOverride = (type == 0);
    DataMgr *dataMgr = GetDataMgr();
    if (!dataMgr)
        return;
    if (dataMgr->GetDataVarNames(3, true).size() == 0)
        return;

    string varname = validatePrimaryVar(type, dataMgr);

    // Perform validation unique to Hello Params.
    //Both point1 and point2 need to be inside the variable extents.
    //By default they will be at opposite corners of variable extents.
    vector<double> minExts, maxExts;
    size_t minTS = _dataStatus->getMinTimestep();
    dataMgr->GetVariableExtents(minTS, varname, 0, minExts, maxExts);
    if (doOverride) {
        //Set to default:
        SetPoint1(minExts);
        SetPoint2(maxExts);
    } else {
        //force the points to lie inside the extents
        vector<double> pt1, pt2;
        pt1 = GetPoint1();
        pt2 = GetPoint2();
        for (int i = 0; i < 3; i++) {
            if (pt1[i] < minExts[i])
                pt1[i] = minExts[i];
            if (pt2[i] < minExts[i])
                pt2[i] = minExts[i];
            if (pt1[i] > maxExts[i])
                pt1[i] = maxExts[i];
            if (pt2[i] > maxExts[i])
                pt2[i] = maxExts[i];
        }
        SetPoint1(pt1);
        SetPoint2(pt2);
    }
    if (doOverride) { //set constant color to red
        const float col[3] = {1.f, 0.f, 0.f};
        SetConstantColor(col);
    }
    initializeBypassFlags();
    return;
}
#endif

//Set everything to default values.
//Data Manager is absent.
void HelloParams::_init() {
    SetLineThickness(1.0);

    vector<double> pt1;
    vector<double> pt2;

    GetBox()->GetExtents(pt1, pt2);

    SetPoint1(pt1);
    SetPoint2(pt2);
}
