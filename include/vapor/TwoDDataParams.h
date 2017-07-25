
#ifndef TWODDATAPARAMS_H
#define TWODDATAPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class TwoDDataParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
//! \author Alan Norton
//! \version 3.0
class PARAMS_API TwoDDataParams : public RenderParams {
  public:
    TwoDDataParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    TwoDDataParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~TwoDDataParams();

#ifdef DEAD
    //! \copydoc Params::Validate()
    virtual void Validate(int type);
#endif

    //! \copydoc RenderParams::IsOpaque()
    virtual bool IsOpaque() const;

    //!
    //! \copydoc RenderParams::usingVariable()
    virtual bool usingVariable(const std::string &varname);

    // Get static string identifier for this params class
    //
    static string GetClassType() {
        return ("TwoDDataParams");
    }

  private:
    void _init();
#ifdef DEAD
    void _validateTF(int type, DataMgr *dataMgr);
#endif

}; //End of Class TwoDDataParams
}; // namespace VAPoR

#endif
