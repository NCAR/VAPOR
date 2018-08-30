#ifndef DVRPARAMS_H
#define DVRPARAMS_H

using namespace VAPoR;

class PARAMS_API DVRParams : public RayCasterParams {
public:
    DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave);
    DVRParams(DataMgr *dataManager, ParamsBase::StateSave *stateSave, XmlNode *xmlNode);

    static std::string GetClassType() { return ("DVRParams"); }
};

#endif
