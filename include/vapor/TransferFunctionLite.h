//************************************************************************
//									*
//		     Copyright (C)  2004				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		transferfunction.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2004
//
//	Description:	Defines the TransferFunctionLite class
//		This is the mathematical definition of the transfer function
//		It is defined in terms of floating point coordinates, converted
//		into a mapping of quantized values (LUT) with specified domain at
//		rendering time.  Interfaces to the TFEditor class.
//		The TransferFunctionLite deals with an mapping on the interval [0,1]
//		that is remapped to a specified interval by the user.
//
#ifndef TRANSFERFUNCTIONBASE_H
#define TRANSFERFUNCTIONBASE_H
#define MAXCONTROLPOINTS 50
#include <vapor/MapperFunctionBase.h>
#include <vapor/ExpatParseMgr.h>
#include <vapor/TFInterpolator.h>

namespace VAPoR {
class TFEditor;
class XmlNode;

class PARAMS_API TransferFunctionLite : public MapperFunctionBase {

  public:
    TransferFunctionLite();
    TransferFunctionLite(int nBits);
    virtual ~TransferFunctionLite();
    virtual TransferFunctionLite *deepCopy(ParamNode *newRoot) {
        //Unnecessary, since this class does not use ParamBase registration.
        return 0;
    }

    //
    // Note:  All public methods use actual real coords.
    // (Protected methods use normalized points in [0,1]
    //

    //
    // Transfer function has identical min,max map bounds, but
    // Parent class has them potentially unequal.
    //
    void setMinMapValue(float minVal) {
        setMinOpacMapValue(minVal);
        setMinColorMapValue(minVal);
    }

    void setMaxMapValue(float val) {
        setMaxOpacMapValue(val);
        setMaxColorMapValue(val);
    }

    float getMinMapValue() { return getMinColorMapValue(); }
    float getMaxMapValue() { return getMaxColorMapValue(); }

    int mapFloatToIndex(float f) { return mapFloatToColorIndex(f); }
    float mapIndexToFloat(int indx) { return mapColorIndexToFloat(indx); }

    void setVarNum(int var) {
        colorVarNum = var;
        opacVarNum = var;
    }

    //
    // Methods to save and restore transfer functions.
    // The gui opens the FILEs that are then read/written
    // Failure results in false/null pointer
    //
    bool saveToFile(ofstream &f);

  protected:
};
};     // namespace VAPoR
#endif //TRANSFERFUNCTIONBASE_H
