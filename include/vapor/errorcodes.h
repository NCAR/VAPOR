//-- errorcodes.h ------------------------------------------------------------
//   
//                   Copyright (C)  2004
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------
//
//      File:           errorcodes.h
//
//      Author:         Kenny Gruchalla
//                      National Center for Atmospheric Research
//                      PO 3000, Boulder, Colorado
//
//      Date:           August 2006
//
//      Description:    VAPoR error codes used for message reporting. 
//
//                      The high-order bits of the codes are bit masked 
//                      providing type information: diagonstic, warning, error,
//                      and fatal.
//
//                      For example, if (code & VAPoR::WARNING) then 'code'
//                      is a warning type. 
//
//----------------------------------------------------------------------------

namespace VAPoR
{
  enum errorcodes
  {
    // VDF Errors: 0x1 - 0xFF:  Handled specially:
	// The Vaporgui application will not clear
	// error codes less than 0xfff, until the
	// error is later detected in the application.
	// Any errors discovered in the application
	// must be set with an error code > 0xfff, or
	// the error will cause methods
	// in the VDF library to fail.
    VAPOR_ERROR_VDF				= 0x1,
	VAPOR_ERROR_XML				= 0x2,
    // Diagnostic Codes  0x1001 - 0x1FFF
	//Currently, diagnostic codes are not used
    VAPOR_DIAGNOSTIC = 0x1000,


    
    // Warning Codes     0x2001 - 0x2FFF
    VAPOR_WARNING               = 0x2000,
    VAPOR_WARNING_GL_SHADER_LOG = 0x2001,
	VAPOR_WARNING_FLOW			= 0x2100,
	VAPOR_WARNING_SEEDS			= 0x2101,
	VAPOR_WARNING_FLOW_DATA		= 0x2102,
	VAPOR_WARNING_FLOW_STOP		= 0x2103,
	VAPOR_WARNING_DATA_UNAVAILABLE	= 0x2004,
	VAPOR_WARNING_GL_ERROR		= 0x2105,
	VAPOR_WARNING_TWO_D			= 0x2106,
    
    // Error Codes       0x4001 - 0x4FFF
    VAPOR_ERROR						= 0x4000,
    VAPOR_ERROR_GL_RENDERING		= 0x4001,
    VAPOR_ERROR_GL_UNKNOWN_UNIFORM	= 0x4002,
	VAPOR_ERROR_DATA_UNAVAILABLE	= 0x4003,
	VAPOR_ERROR_DATA_TOO_BIG		= 0x4004,
	VAPOR_ERROR_DRIVER_FAILURE		= 0x4005,
	VAPOR_ERROR_GL_VENDOR			= 0x4006,
	VAPOR_ERROR_DVR					= 0x4007,
	VAPOR_ERROR_IMAGE_CAPTURE		= 0x4008,
	VAPOR_ERROR_SPHERICAL			= 0x4009,
	VAPOR_ERROR_STRETCHED			= 0x4010,
	VAPOR_ERROR_PARAMS				= 0x4011,
	VAPOR_ERROR_TWO_D				= 0x4012,
	VAPOR_ERROR_GEOREFERENCE		= 0x4013,
	VAPOR_ERROR_SCRIPTING			= 0x4014,
	VAPOR_ERROR_PARSING				= 0x4015,
	VAPOR_ERROR_VDC_MERGE			= 0x4016,
	VAPOR_ERROR_GL_SHADER			= 0x4017,

	//Flow errors
	VAPOR_ERROR_FLOW = 0x4100,
	VAPOR_ERROR_SEEDS = 0x4101,
	VAPOR_ERROR_INTEGRATION = 0x4102,
	VAPOR_ERROR_FLOW_DATA = 0x4103,
	VAPOR_ERROR_FLOW_SAVE = 0x4104,
    
    // Fatal Codes       0x8001 - 0x8FFF
    VAPOR_FATAL = 0x8000
  };   
};
