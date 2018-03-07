//-- AnnotationRenderer.h ----------------------------------------------------------
//   
//                   Copyright (C)  2011
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------
//
//      File:           AnnotationRenderer.h
//
//      Author:         Alan Norton
//
//
//      Description:  Definition of AnnotationRenderer class
//
//
//
//----------------------------------------------------------------------------

#ifndef	ANNOTATIONRENDERER_H
#define	ANNOTATIONRENDERER_H

#include <vapor/Grid.h>
#include <vapor/Renderer.h>
#include <vapor/textRenderer.h>
#include <vapor/Transform.h>
#include <vapor/DataMgrUtils.h>

namespace VAPoR {

class DataStatus;

//! \class AnnotationRenderer
//! \brief Class that draws various geometry as specified by AnnotationParams
//! \author Alan Norton
//! \version 3.0
//! \date July 2015

class RENDER_API AnnotationRenderer : public MyBase {

private:
 struct billboard;	
      
public:
 //! Constructor, must invoke Renderer constructor
 AnnotationRenderer(
	const ParamsMgr *pm, const DataStatus *dataStatus, string winName
 );

 //! Method to initialize GL rendering.  Must be called from a GL context.
 //! \param[in] sm A pointer to a ShaderMgr
 void InitializeGL(ShaderMgr *sm);

 //! Destructor
 virtual ~AnnotationRenderer();

 //! Render the in-scene features
	void InScenePaint(size_t ts);

 //! Render the overlay features
	void OverlayPaint(size_t ts);

	void AddText(string text, int x, int y, int size, 
				float color[3], int type=0);

	void DrawText();

	void DrawText(vector<billboard> billboards);

	void ClearText(int type=-1);

#ifdef	DEAD
 //! Clear all the text objects
	void invalidateCache();
#endif

protected:


private:
 struct billboard {
	string text;
	int x;
	int y;
	int size;
	float color[3];
 };
 vector<billboard> _billboards;
 vector<billboard> _timeAnnot;
 vector<billboard> _axisAnnot;
 vector<billboard> _miscAnnot;

 const ParamsMgr* m_paramsMgr;
 const DataStatus* m_dataStatus;
 string m_winName;
 ShaderMgr *m_shaderMgr;
 int _currentTimestep;
 bool _textObjectsValid;
 TextObject* _textObject;
 string _fontFile;

	void _drawAxes(
		std::vector<double> min,
		std::vector<double> max,
		std::vector<double> origin,
		std::vector<double> color,
		double width);
	void _drawTic(double startPosn[],
		double endPosn[],
		double width,
		std::vector<double> color);

//! Render the domain fram
	void drawDomainFrame(size_t ts) const;


 std::vector<double> getDomainExtents(string dmName = "") const;
 AxisAnnotation* getCurrentAxisAnnotation();
 string getCurrentDataMgrName() const;
 void scaleNormalizedCoordinatesToWorld(
	std::vector<double> &coords,
	string dataMgrName);


#ifdef	DEAD
//! Render the region frame
	void drawRegionBounds(size_t ts) const;
#endif

 // Draw the axis lines, while building text labels.
 //
 void drawAxisTics(AxisAnnotation* aa=NULL);
 void applyTransform(Transform *t);
 void renderText(double text, double coords[], AxisAnnotation *aa = NULL);
 Transform* getTransform(string dataMgr="");
 void convertPointToLon(double &xCoord, string dataMgr="");
 void convertPointToLat(double &yCoord, string dataMgr="");
 void convertPointToLonLat(double &xCoord, double &yCoord, string dataMgr="");

 // Draw Axis arrows
 //
 void drawAxisArrows(
	std::vector <double> minExts, std::vector <double> maxExts
 );

#ifdef	DEAD
//! Static method to convert axis coordinates between user and lat-lon
//! It is OK for outputs to equal corresponding inputs.
//! \param[in] toLatLon indicates whether conversion is to LatLon (true) or to user (false)
//! \param[in] ticDirs are directions of tics on each axis.
//! \param[in] fromMinTic is a 3-vector indicating minimum tic coordinates being mapped.
//! \param[in] fromMaxTic is a 3-vector indicating maximum tic coordinates being mapped.
//! \param[in] fromOrigin is a 3-vector indicating origin coordinates being mapped.
//! \param[in] fromTicLength is a 3-vector indicating tic lengths before conversion
//! \param[out] toMinTic is result 3-vector of minimum tic coordinates 
//! \param[out] toMaxTic is result 3-vector of maximum tic coordinates
//! \param[out] toOrigin is result 3-vector of origin coordinates
//! \param[out] toTicLength is a 3-vector indicating tic lengths after conversion
	static void ConvertAxes(bool toLatLon, const vector<long> ticDirs, const vector<double> fromMinTic, const vector<double> fromMaxTic, const vector<double> fromOrigin, const vector< double> fromTicLength,
		double toMinTic[3],double toMaxTic[3], double toOrigin[3], double toTicLength[3]);
 
  //This method converts lon lat to user coords, assuming a "flat" earth so axes will not wrap.
	static void flatConvertFromLonLat(double x[2], double minLon, double maxLon, double minX, double maxX);

#endif
  };
  
};

#endif //ANNOTATIONRENDERER_H
