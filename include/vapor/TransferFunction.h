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
//	Description:	Defines the TransferFunction class  
//		This is the mathematical definition of the transfer function
//		It is defined in terms of floating point coordinates, converted
//		into a mapping of quantized values (LUT) with specified domain at
//		rendering time.  Interfaces to the TFEditor and Various Params classes.
//		The TransferFunction deals with an mapping on the interval [0,1]
//		that is remapped to a specified interval by the user.
//
#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H

#include <vapor/MapperFunction.h>

namespace VAPoR {

class PARAMS_API TransferFunction : public MapperFunction 
{

public:

 TransferFunction( ParamsBase::StateSave *ssave);

 TransferFunction( ParamsBase::StateSave *ssave, XmlNode *node);

 virtual ~TransferFunction();

 //! Save this transfer function to a file
 //! \param[in] f opened ofstream for writing the transfer function
 //! \return true if successful
 int SaveToFile(string path);

 //! Load a transfer function from a file, insert it in specified RenderParams
 //! \param[in] is ifstream opened for obtaining the transfer function from file
 //! \param[in] p RenderParams that will own the transfer function
 int LoadFromFile(string path);

 // Get static string identifier for this params class
 //
 static string GetClassType() {
	return("TransferFunctionParams");
 }

private:
	
};
};
#endif //TRANSFERFUNCTION_H
