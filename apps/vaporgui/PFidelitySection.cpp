#include "PFidelitySection.h"
#include "VComboBox.h"
#include "PCheckbox.h"
#include <vapor/RenderParams.h>

using namespace VAPoR;

// ==================================
//          PFidelitySection
// ==================================

PFidelitySection::PFidelitySection() : PSection("Data Fidelity")
{
    Add(new PQuickFidelitySelector);
    Add(new PCheckbox("FidelityUseAdvanced", "Advanced"));
    Add((new PLODSelector)->EnableBasedOnParam("FidelityUseAdvanced"));
    Add((new PRefinementSelector)->EnableBasedOnParam("FidelityUseAdvanced"));
}

// ==================================
//      PQuickFidelitySelector
// ==================================

PQuickFidelitySelector::PQuickFidelitySelector() : PLineItem("", "Fidelity", _vComboBox = new VComboBox({"Medium"}))
{
    connect(_vComboBox, &VComboBox::ValueChanged, this, &PQuickFidelitySelector::dropdownTextChanged);
}

void PQuickFidelitySelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    VAssert(rp && "Params must be RenderParams");

    auto dm = getDataMgr();
    auto vn = rp->GetFirstVariableName();
    int  nLod = dm->GetCRatios(vn).size();
    int  nRef = dm->GetNumRefLevels(vn);
    int  minOptions = vn.empty() ? 0 : max(nLod, nRef);

    vector<string> items;
    if (minOptions >= 2) items.push_back("Low");
    if (minOptions >= 3) items.push_back("Medium");
    if (minOptions >= 1)
        items.push_back("High");
    else
        items.push_back("<none>");

    _vComboBox->SetOptions(items);

    int lod = rp->GetCompressionLevel();
    int ref = rp->GetRefinementLevel();
    int n = items.size();

    _vComboBox->SetIndex(paramsToSimple(n, nLod, nRef, lod, ref));

    long ts = rp->GetCurrentTimestep();
    for (int i = 0; i < n; i++) {
        simpleToParams(n, nLod, nRef, i, &lod, &ref);
        bool exists = dm->VariableExists(ts, vn, 0, ref);
        if (!exists) _vComboBox->SetItemEnabled(i, false);
    }
}

void PQuickFidelitySelector::dropdownTextChanged(std::string)
{
    auto *rp = (RenderParams *)getParams();
    auto  dm = getDataMgr();
    auto  vn = rp->GetFirstVariableName();
    int   nLod = dm->GetCRatios(vn).size();
    int   nRef = dm->GetNumRefLevels(vn);
    int   lod, ref;
    simpleToParams(_vComboBox->GetCount(), nLod, nRef, _vComboBox->GetCurrentIndex(), &lod, &ref);

    rp->BeginGroup("Change lod/cRatio");
    rp->SetCompressionLevel(lod);
    rp->SetRefinementLevel(ref);
    rp->EndGroup();
}

void PQuickFidelitySelector::simpleToParams(int nSimple, int nLod, int nRef, int simple, int *lod, int *ref)
{
    if (nSimple == 1) {
        *lod = 0;
        *ref = 0;
    } else {
        *lod = simple * (nLod - 1) / (nSimple - 1);
        *ref = simple * (nRef - 1) / (nSimple - 1);
    }
}

int PQuickFidelitySelector::paramsToSimple(int nSimple, int nLod, int nRef, int lod, int ref)
{
    int div = nLod + nRef - 2;
    if (div == 0) return 0;
    return (1 + lod + ref) * (nSimple - 1) / div;
}

// ==================================
//           PLODSelector
// ==================================

PLODSelector::PLODSelector() : PLineItem("", "Level of Detail", _vComboBox = new VComboBox({"0 (1000:1)"}))
{
    connect(_vComboBox, &VComboBox::IndexChanged, this, &PLODSelector::dropdownIndexChanged);
}

void PLODSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    VAssert(rp && "Params must be RenderParams");

    auto varName = rp->GetFirstVariableName();
    auto dm = getDataMgr();
    auto cr = dm->GetCRatios(varName);
    long timestep = rp->GetCurrentTimestep();

    vector<string> items;
    for (int i = 0; i < cr.size(); i++) items.push_back(to_string(i) + " (" + to_string(cr[i]) + ":1)");

    _vComboBox->SetOptions(items);
    _vComboBox->SetIndex(rp->GetCompressionLevel());

    for (int i = 0; i < cr.size(); i++) {
        bool exists = dm->VariableExists(timestep, varName, 0, i);
        if (!exists) _vComboBox->SetItemEnabled(i, false);
    }
}

void PLODSelector::dropdownIndexChanged(int i)
{
    RenderParams *rp = (RenderParams *)getParams();
    rp->SetCompressionLevel(i);
}

// ==================================
//        PRefinementSelector
// ==================================

PRefinementSelector::PRefinementSelector() : PLineItem("", "Refinement Level", _vComboBox = new VComboBox({"0 (100x100x100)"}))
{
    connect(_vComboBox, &VComboBox::IndexChanged, this, &PRefinementSelector::dropdownIndexChanged);
}

void PRefinementSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    VAssert(rp && "Params must be RenderParams");

    auto varName = rp->GetFirstVariableName();
    auto dm = getDataMgr();
    int  nrf = dm->GetNumRefLevels(varName);

    vector<string> items;

    for (int i = 0; i < nrf; i++) {
        vector<size_t> dims;
        dm->GetDimLensAtLevel(varName, i, dims);
        if (dims.empty()) continue;
        auto   itr = dims.begin();
        string item = to_string(i) + " (" + to_string(*itr++);
        for (; itr != dims.end(); ++itr) item += "x" + to_string(*itr);
        item += ")";
        items.push_back(item);
    }

    _vComboBox->SetOptions(items);
    _vComboBox->SetIndex(rp->GetRefinementLevel());
}

void PRefinementSelector::dropdownIndexChanged(int i)
{
    RenderParams *rp = (RenderParams *)getParams();
    rp->SetRefinementLevel(i);
}
