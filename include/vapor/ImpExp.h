//
//      $Id$
//


#ifndef	_ImpExp_h_
#define	_ImpExp_h_

#include <stack>
#include <expat.h>
#include <vapor/MyBase.h>
#include <vapor/common.h>
#include <vapor/XmlNode.h>
#include <vapor/ExpatParseMgr.h>
#ifdef WIN32
#pragma warning(disable : 4251)
#endif
namespace VAPoR {


//
//! \class ImpExp
//! \brief A class for managing data set metadata
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//! The ImpExp class is used to import/export state to/from a VAPoR session
//!
class VDF_API ImpExp : public Wasp::MyBase, public ParsedXml {
public:

 //! Create an Import Export object
 //!
 //
 ImpExp();


 virtual ~ImpExp();


 //! Export a volume subregion description for use by another application
 //!
 //! This method exports data set state information to facilitate sharing
 //! with another application. In particular, the path name of the VDF file,
 //! a volume time step, a named volume variable, and volumetric region 
 //! of interest are exported. The exported information may be retreive
 //! via the Import() method
 //!
 //! \note The region bounds are specified relative to the finest 
 //! resolution volume
 //!
 //! \note Presently the medium of exchange is an XML file, the path to 
 //! which is internally hardwired and based on the user's uid. 
 //! Thus it is not possible for applications running under different uid's
 //! to share data. Furthermore, there exists only one XML file per uid
 //! on a system. This will undoubtedly change in the future.
 //!
 //! \param[in] path Path to the metadata file
 //! \param[in] ts Time step of exported volume
 //! \param[in] varname Variable name of exported volume
 //! \param[in] min Minimum region extents in voxel coordinates relative
 //! to the \b finest resolution 
 //! \param[in] max Maximum region extents in voxel coordinates relative
 //! to the \b finest resolution 
 //! \param[in] timeseg Time segment range
 //! \retval status Returns a non-negative integer on success
 //! \sa Import()
 //
 int Export(
	const string &path, size_t ts, const string &varname, 
	const size_t min[3], const size_t max[3], const size_t timeseg[2]
 );

 //! Import a volume subregion description for use by another application
 //!
 //! This method imports data set state information to facilitate sharing
 //! with another application. In particular, the path name of the VDF file,
 //! a volume time step, a named volume variable, and volumetric region 
 //! of interest are imported. The imported information is assumed to 
 //! have been generated via the the most recent call to the Export() method.
 //!
 //! \param[out] path Path to the metadata file
 //! \param[out] ts Time step of exported volume
 //! \param[out] varname Variable name of exported volume
 //! \param[out] min Minimum region extents in voxel coordinates relative
 //! to the \b finest resolution 
 //! \param[out] max Maximum region extents in voxel coordinates relative
 //! to the \b finest resolution 
 //! \param[out] timeseg Time segment range
 //! \retval status Returns a non-negative integer on success
 //! \sa Export()
 //
 int Import(
	string *path, size_t *ts, string *varname, 
	size_t min[3], size_t max[3], size_t timeseg[2]
 );
static string GetPath();
private:
	int	_objInitialized;	// has the obj successfully been initialized?
	XmlNode	*_rootnode;		// root node of the xml tree

	static const string _rootTag;
	static const string _pathNameTag;
	static const string _timeStepTag;
	static const string _varNameTag;
	static const string _regionTag;
	static const string _timeSegmentTag;

	bool elementStartHandler(ExpatParseMgr*, int depth , std::string& tag, const char **attr);
	bool elementEndHandler(ExpatParseMgr*, int depth , std::string& );

	// XML Expat element handler helpers. A different handler is defined
	// for each possible state (depth of XML tree) from 0 to 1
	//
	void _startElementHandler0(ExpatParseMgr*, const string &tag, const char **attrs);
	void _startElementHandler1(ExpatParseMgr*,const string &tag, const char **attrs);
	void _endElementHandler0(ExpatParseMgr*,const string &tag);
	void _endElementHandler1(ExpatParseMgr*,const string &tag);

	

};


};

#endif	//	_ImpExp_h_
