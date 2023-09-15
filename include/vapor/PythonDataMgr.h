#include <vapor/DataMgr.h>

#pragma once

namespace VAPoR {

class DCRAM;

//! \class PythonDataMgr
//! \brief DataMgr for data loaded from python scripts
//! \author Stas Jaroszynski

class VDF_API PythonDataMgr : public DataMgr {
public:
    PythonDataMgr(string format, size_t mem_size, int nthreads = 0);
    virtual ~PythonDataMgr();
    
    void AddRegularData(string name, const float *buf, vector<int> dims);
    DCRAM *GetDC() const;
    void ClearCache(string varname);
};

}
