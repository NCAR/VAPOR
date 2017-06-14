
#ifndef _WaveFiltHaar_h_
#define _WaveFiltHaar_h_

namespace VAPoR {

//
//! \class WaveFiltHaar
//! \brief Haar FIR filters
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! This class provides FIR filters for the Haar wavelet
//!
class WASP_API WaveFiltHaar : public WaveFiltBase {

  public:
    //! Create a set of Haar wavelet filters
    //!
    WaveFiltHaar();
    virtual ~WaveFiltHaar();

  private:
    void _analysis_initialize();
    void _synthesis_initialize();
};

} // namespace VAPoR

#endif
