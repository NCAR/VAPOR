#include <string>
#include <vapor/WaveFiltBase.h>

using namespace std;

#ifndef _WaveFiltCoif_h_
#define _WaveFiltCoif_h_

namespace VAPoR {

//
//! \class WaveFiltCoif
//! \brief Coiflet family FIR filters
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This class provides FIR filters for the Coiflet family of wavelets
//!
class WASP_API WaveFiltCoif : public WaveFiltBase {

  public:
    //! Create a set of Coiflet filters
    //!
    //! \param[in] wavename The Coiflet family wavelet member. Valid values
    //! are "coif1", "coif2", "coif3", "coif4", and "coif5"
    //!
    WaveFiltCoif(const string &wavename);
    virtual ~WaveFiltCoif();

  private:
    void _analysis_initialize(int member);
    void _synthesis_initialize(int member);
};

} // namespace VAPoR

#endif
