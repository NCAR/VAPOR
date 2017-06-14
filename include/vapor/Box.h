//************************************************************************
//									*
//		     Copyright (C)  2011				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Box.h
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		April 2011
//
//	Description:	Defines the Box class
//		This supports control of a 2D or 3D box-shaped region that can be
//		rotated and changed over time.
//
#ifndef BOX_H
#define BOX_H

#include <vapor/ParamsBase.h>
#include <vapor/DataStatus.h>


namespace VAPoR {


//! \class Box
//! \ingroup Public_Params
//! \brief 3D or 2D box with options for orientation angles .
//! \author Alan Norton
//! \version 3.0
//! \date    March 2014

//! The Box class supports various rectangular boxes, possibly rotated,  
//! used in VAPOR to specify extents and also used in Manipulators.
//! 
class PARAMS_API Box : public ParamsBase {
public:
	
	//! Create a Box object from scratch
	//
	Box(ParamsBase::StateSave *ssave);

	//! Create a Box object from an existing XmlNode tree
	//
	Box(ParamsBase::StateSave *ssave, XmlNode *node);

	virtual ~Box();



 //! Set the box min and max extents
 //!
 //! Set the extents of the box. 
 //!
 //! \param[in] minExt 2 or 3-element vector containing the minimum coordinates 
 //! of the box, specified in the order X, Y, Z
 //! \param[in] maxExt 2 or 3-element vector containing the maximum coordinates 
 //! of the box, specified in the order X, Y, Z
 //!
 //
 void SetExtents(
	const vector <double> &minExt, const vector <double> &maxExt
 );

 //! Get the box min and max extents
 //!
 //! Get the box's coordinate extents.  If IsPlanar() is true a 2-element
 //! array is returned for \p minExt and \p maxExt, if false a 
 //! 3-element array is returned.
 //!
 //! \param[out] minExt 2 or 3-element vector containing the minimum 
 //! coordinates 
 //! of the box, specified in the order X, Y, Z
 //! \param[out] maxExt 2 or 3-element vector containing the 
 //! maximum coordinates 
 //! of the box, specified in the order X, Y, Z
 //!
 //! \sa IsPlanar(), GetOrientation(), SetExtents()
 //
 void GetExtents(
	vector <double> &minExt, vector <double> &maxExt
 ) const;

 //! Indicate whether or not the box is constrained to be planar.  
 //!
 //! \retval bool True if the box is planar
 //!
 //! \sa GetOrientation()
 //
 bool IsPlanar() const {
	return GetValueLong(Box::m_planarTag, (long) false);
 }

 //! Constain the box to be planar or not
 //!
 //! Set the value of the planar state, indicating whether or not
 //! the box is constrained to be planar.
 //!
 //! \param[in] value bool indicates whether or be planar
 //!
 //! \sa SetExtents()
 //
 void SetPlanar(bool value);

 //! Indicate the orientation of a (2D) box
 //! This is 0,1, or 2 based on the axis orthogonal to the box.
 //! A 0 indiciates the X axis, 1 indicates the Y axis, and 2 the Z 
 //! axis.
 //!
 //! \retval int The axis to which the box is orthogonal, if planar. Otherwise
 //! the return value is meaningless.
 //!
 //! \sa IsPlanar()
 //
 int GetOrientation() const {
	return GetValueLong(Box::m_orientationTag, 2);
 }

 //! Set the value of the orientation state, indicating the axis
 //! orthogonal to a 2D box 
 //!
 //! \param[in] value long indicates the orientation value
 //!
 //! \sa IsPlanar(), GetOrientation()
 //
 void SetOrientation(long value) {
	SetValueLong(Box::m_orientationTag, "Set box orientation", (long)value);
 }

#ifdef	DEAD
	//! Get the stretched local box extents as a float array.  If timestep is >= 0, then get it just
	//! for the specified timestep
	//! \param[out] extents[6] float Returned extents
	//! \param[in] timestep int Specific time step being retrieved, or -1 for generic time steps
	//! \retcode int zero if successful.
	int GetStretchedLocalExtents(double extents[6], int timestep = -1);

	//! Get the local box extents as a vector.  First 6 values are default; additional
	//! values are associated with non-default regions
	//! \sa GetTimes()
	//!
	//! \param[out] extents const vector<double>& returned extents
	vector<double>  GetLocalExtents() const {
		const vector<double> localExtents(6,0.);
		return GetValueDoubleVec(_extentsTag,localExtents);
	}

	//! Specify the local extents.  If time step is -1, then set the generic extents.
	//! Otherwise set the extents for a specific timestep.
	//! \param[in] extents vector<double>& Six doubles that will be new extents
	//! \param[in] timestep int Specified time step, or -1 for generic times
	//! \retval int zero if successful.
	void SetLocalExtents(const vector<double>& extents, int timestep = -1);

	void SetLocalExtents(
		const vector<double>& minExt, const vector<double>& maxExt, 
		int timestep = -1
	) {
		assert(minExt.size() == maxExt.size() && minExt.size() == 3);
		vector <double> extents = minExt;
		extents.insert(extents.end(), maxExt.begin(), maxExt.end());
		SetLocalExtents(extents, timestep);
	}

	//! Specify the local extents as a double array.  If time step is -1, then set the generic extents.
	//! Otherwise set the extents for a specific timestep.
	//! \param[in] double extents[6] 6 doubles that will be new extents
	//! \param[in] int timestep specified time step, or -1 for generic times
	//! \retval int zero if successful.
	void SetLocalExtents(const double extents[6], int timestep = -1);

	//! Specify the local extents as a float array.  If time step is -1, then set the generic extents.
	//! Otherwise set the extents for a specific timestep.
	//! \param[in] float extents[6]
	//! \param[in] int timestep specified time step, or -1 for generic times
	//! \retval int zero if successful.
	void SetLocalExtents(const float extents[6], int timestep = -1);
#endif

#ifdef	DEAD
	//! Specify the stretched local extents as a float array.  If time step is -1, then set the generic extents.
	//! Otherwise set the extents for a specific timestep.
	//! \param[in] float extents[6]
	//! \param[in] int timestep specified time step, or -1 for generic times
	//! \retval int zero if successful.
	void SetStretchedLocalExtents(const double extents[6], int timestep = -1);
#endif


	//! Get the three orientation angles (theta, phi, psi)
	//! Defaults to empty vector if no angles are set.
	//! \retval const vector<double> vector of length 3 of angles.
	vector<double> GetAngles() const {
		const vector<double> defaultAngles(3,0.);
		return GetValueDoubleVec(Box::m_anglesTag,defaultAngles);
	}

#ifdef	DEAD
	//! Get the angles as a double array
	//! \param [out] double angles[3] array of three doubles for theta, phi, psi
	//! \retval int zero if successful
	void GetAngles(double ang[3]){
		const vector<double> angv = GetAngles();
		for (int i=0; i<3; i++) ang[i] = angv[i];
	}

	//! Get the angles as a float array
	//! \param [out] angles[3] float array of three floats for theta, phi, psi
	//! \retval zero if successful
	void GetAngles(float ang[3]){
		const vector<double> angv = GetAngles();
		for (int i=0; i<3; i++) ang[i] = angv[i];
	}
#endif

	//! Set the angles from a double array
	//! \param [in] ang double[3] array of three doubles for theta, phi, psi
	//! \retval int zero on success
	void SetAngles(const double angles[3]){
		vector<double> ang;
		for (int i = 0; i<3;i++) ang.push_back(angles[i]);
		SetValueDoubleVec(m_anglesTag, "change box angles",ang);
	}

	//! Set the angles from a float array
	//! \param [in] angles float[3] array of three floats for theta, phi, psi
	//! \retval int zero on success
	void SetAngles(const float angles[3]){
		vector<double> angl;
		for (int i = 0; i<3;i++) angl.push_back((double)angles[i]);
		SetValueDoubleVec(m_anglesTag, "change box angles",angl);
	}

	//! Set the three orientation angles (theta, phi, psi) from a vector of doubles
	//! \param[in] vals const vector<double>& vector of length 3 of angles.
	void SetAngles(const vector<double>& vals) {
		SetValueDoubleVec(m_anglesTag, "Change box angles",vals);
	}
#ifdef	DEAD
	//! Get the time(s) as a long vector.
	//! The first one should be negative, marking the default extents.
	//! Subsequent times are nonnegative integers indicating times for nondefault extents.
	//! Number of times should be 1/6 of the number of extents values
	//! \sa GetExtents()
	//! \retval vector<long>& vector of longs
	const vector<long> GetTimes() { 
		return( GetValueLongVec(Box::_timesTag));
	}
	//! Set the time(s) as a long vector.
	//! The first one should be negative, marking the default extents.
	//! Subsequent times are nonnegative integers indicating times for nondefault extents.
	//! This vector should be the same size as the extents vector.
	//! \param [in] const vector<long>& vector of times
	void SetTimes(const vector<long>& times) { 
		SetValueLongVec(Box::_timesTag, "Change box times",times);
	}

	void buildLocalCoordTransform(
		double transformMatrix[12], double extraThickness, 
		int timestep, double rotation = 0., int axis= -1
	) const;
#endif

	// Get static string identifier for this params class
	//
	static string GetClassType() {
		return("BoxParams");
	}


private:
	
#ifdef	DEAD
//! method supports rotated boxes such as probe
//! Specifies an axis-aligned box containing the rotated box.
//! By default it just finds the box extents.
//! Caller must supply extents array, which gets its values filled in.
//! \param[out] float[6] Extents of containing box
//! \params[in] rotated indicates if the box is possibly rotated
	void calcContainingBoxExtents(
		double extents[6], bool rotated = false
	) const {

		if (!rotated) GetLocalExtents(extents,-1);
		else calcRotatedBoxExtents(extents);

	}

//! If the box is rotated, this method calculated the minimal axis-aligned extents
//! containing all 8 corners of the box.
//! \param[out] double extents[6] is smallest extents containing the box.
	void calcRotatedBoxExtents(double extents[6]) const;

//! method supports rotated boxes such as probe
//! Specifies an axis-aligned box containing the stretched rotated box.
//! By default it just finds the box extents.
//! Caller must supply extents array, which gets its values filled in.
//! \param[out] float[6] Extents of containing box
//! \params[in] rotated indicates if the box is possibly rotated
	void calcContainingStretchedBoxExtents(
		double extents[6], bool rotated = false
	) const {

		assert( ! rotated);
		if (!rotated) GetStretchedLocalExtents(extents,-1);
		//else calcRotatedStretchedBoxExtents(extents);

	}


//! If the box is rotated, this method calculated the minimal axis-aligned extents
//! of a stretched box containing all 8 corners of the box.
//! \param[out] double extents[6] is smallest extents containing the box.
	void calcRotatedStretchedBoxExtents(
		vector <double> stretchFactors, double extents[6]
	) const;


	//Used only by params with rotated boxes:
	bool cropToBox(const double boxExts[6]);
	bool intersectRotatedBox(double boxexts[6], double pointFound[3], double probeCoords[2]);
	bool fitToBox(const double boxExts[6]);
	void setBoxToExtents(const double extents[6]);
	int interceptBox(const double boxExts[6], double intercept[6][3]);

	void getRotatedVoxelExtents(string varname, float voxdims[2], int numRefinements);


	void rotateAndRenormalize(int axis, double rotVal);
	///@}


	void convertThetaPhiPsi(
		double *newTheta, double* newPhi, double* newPsi, 
		int axis, double rotation
	) const;

	//Not part of public API
	void calcLocalBoxCorners(
		double corners[8][3], float extraThickness, int timestep, 
		double rotation = 0., int axis = -1
	) const ;
	
#endif

	static const string m_anglesTag;
	static const string m_extentsTag;
	static const string m_planarTag;
	static const string m_orientationTag;
};
};
#endif
