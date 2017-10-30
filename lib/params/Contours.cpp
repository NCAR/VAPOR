#include <iostream>
#include <vapor/ContourParams.h>
#include <vapor/glutil.h>

using namespace VAPoR;
using namespace Wasp;

static ParamsRegistrar<ContourParams::Contours> 
	registrar(ContourParams::Contours::GetClassType());

const string ContourParams::Contours::_minTag = "Min";
const string ContourParams::Contours::_countTag = "Count";
const string ContourParams::Contours::_spacingTag = "Spacing";

ContourParams::Contours::Contours(ParamsBase::StateSave* ssave)
    : ParamsBase(ssave, Contours::GetClassType()) {}

ContourParams::Contours::Contours(ParamsBase::StateSave* ssave, XmlNode* node)
    : ParamsBase(ssave, node) {}

ContourParams::Contours::~Contours() {
    MyBase::SetDiagMsg("Contours::~Contours() this=%p", this);
}

vector<double> ContourParams::Contours::GetIsovalues() const {
    vector<double> vals;
    double min = GetMin();
    int count = GetCount();
    double spacing = GetSpacing();

    for (int i=0; i<count; i++) {
        vals.push_back(min + spacing*i);
    }

    return vals;
}

void ContourParams::Contours::SetIsovalues(vector<double> vals) {
    double min = vals[0];
    int count = vals.size();
    double spacing;
    if (vals.size() > 1) {
        spacing = vals[1] - vals[0];
    }
    else {
		spacing = GetSpacing();
        //spacing = 0;
    }

    SetMin(min);
    SetCount(count);
    SetSpacing(spacing);
}
