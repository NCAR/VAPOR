
#ifndef BARBPARAMS_H
#define BARBPARAMS_H

#include <vapor/RenderParams.h>
#include <vapor/DataMgr.h>

namespace VAPoR {

//! \class BarbParams
//! \brief Class that supports drawing Barbs based on 2D or 3D vector field
//! \author Scott Pearse
//! \version 3.0
//! \date June 2017
class PARAMS_API BarbParams : public RenderParams {
  public:
    BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave);

    BarbParams(DataMgr *dataMgr, ParamsBase::StateSave *ssave, XmlNode *node);

    virtual ~BarbParams();

    //! Get the vector scaling factor
    //! \retval double scale factor
    //
    double GetVectorScale() const {
        return GetValueDouble(_vectorScaleTag, 1.0);
    }

#ifdef DEAD
    //! \copydoc Params::Validate()
    virtual void Validate(int type);
#endif

    //! \copydoc RenderParams::IsOpaque()
    virtual bool IsOpaque() const;

    //!
    //! \copydoc RenderParams::usingVariable()
    virtual bool usingVariable(const std::string &varname);

    //! Determine the size of the discrete grid
    //! E.g. the grid on which barbs are placed.
    //! \retval vector<long> grid
    const vector<long> GetGrid() const {
        const vector<long> defaultGrid(3, 1);
        return (GetValueLongVec(_gridTag, defaultGrid));
    }

    //! Determine if rake grid is aligned to data grid
    //! \retval bool true if aligned.
    bool IsAlignedToData() const {
        return (GetValueLong(_alignGridTag, (bool)false));
    }

    //! Determine line thickness in voxels
    //! \retval double line thickness
    double GetLineThickness() const {
        return (GetValueDouble(_lineThicknessTag, 1.0));
    }

    // Get static string identifier for this params class
    //
    static string GetClassType() {
        return ("BarbParams");
    }

  private:
    void _init();
    static const string _vectorScaleTag;
    static const string _lineThicknessTag;
    static const string _gridTag;
    static const string _alignGridTag;
    static const string _alignGridStridesTag;

}; //End of Class BarbParams
}; // namespace VAPoR

#endif
