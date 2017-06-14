//
//      $Id$
//

#ifndef _DataMgrFactory_h_
#define _DataMgrFactory_h_

#include <vector>
#include <string>
#include <vapor/DataMgr.h>

namespace VAPoR {

class VDF_API DataMgrFactory : public Wasp::MyBase {
public:
    static DataMgr *New(const vector<string> &files, size_t mem_size, string ftype = "vdf");
};

};    // namespace VAPoR

#endif    //	_DataMgrFactory_h_
