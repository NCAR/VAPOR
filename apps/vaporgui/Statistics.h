//************************************************************************
//																	  *
//		   Copyright (C)  2016										*
//	 University Corporation for Atmospheric Research				  *
//		   All Rights Reserved										*
//																	  *
//************************************************************************/
//
//  File:	   Statistics.h
//
//  Author:	 Scott Pearse
//		  National Center for Atmospheric Research
//		  PO 3000, Boulder, Colorado
//
//  Date:	   August 2016
//
//  Description:	Implements the Statistics class.
//
#ifdef WIN32
#pragma warning(disable : 4100)
#endif

#ifndef STATISTICS_H
#define STATISTICS_H

#include <qdialog.h>
#include <qwidget.h>
#include <vapor/DataMgr.h>
#include <vapor/StructuredGrid.h>
#include "ui_statsWindow.h"
#include "ui_errMsg.h"
#include "RangeController.h"


//
//! \class Statistics
//! \brief ???
//!
//! \author Scott Pearse
//! \version $revision$
//! \date $Date$
//!
//!

/*class DataMgr {
	public:
	RegularGrid* GetGrid(size_t ts, string varname, int reflevel,
						int lod, const size_t min[3], const size_t max[3],
						bool lock = false) {return new RegularGrid;}
	vector<double> GetExtents(size_t ts=0) {vector<double> foo; return foo;}
	long GetNumTimeSteps() const {return 0;}
	vector<size_t> GetCRatios() {vector<size_t> r; return r;}
	int GetNumTransforms(){return 0;}
	vector<string> GetVariables3D() {vector<string> s; return s;}
	vector<string> GetVariables2DXY() {vector<string> s; return s;}
	vector<string> GetVariables2DYZ() {vector<string> s; return s;}
	vector<string> GetVariables2DXZ() {vector<string> s; return s;}
	void GetEnclosingRegion(size_t ts, double minu[3], double maxu[3],
						size_t min[3], size_t max[3], int reflevel=0,
						int lod=0) {};
	void GetDim(size_t dim[3], int reflevel=0) {};
	void MapUserToVox(size_t ts, double vcoord0[3], size_t vcoord1[3], int ref=0, int lod=0) {};
	double GetTSUserTime(size_t ts);
};*/

class sErrMsg : public QDialog, public Ui_ErrMsg {
	Q_OBJECT
	public:
		sErrMsg() {setupUi(this);}
};

class Statistics : public QDialog, public Ui_StatsWindow {

	Q_OBJECT


	public:
		Statistics(QWidget* parent);
		~Statistics();	
		int initDataMgr(VAPoR::DataMgr* dm);
		void showMe();
		int initialize();

	public slots:
		void restoreExtents();
		void minTSChanged();
		void maxTSChanged();
		void autoUpdateClicked();
		void refinementChanged(int);
		void cRatioChanged(int);
		void newVarAdded(int);
		void updateButtonPressed() {update();}
		void initRegion(bool activeRegion=false);
		void copyActiveRegion();
		void varRemoved(int);
		void exportText();
		void regionSlidersChanged();
		void addStatistic(int);
		void removeStatistic(int);

	private:
		int GetExtents(vector<double>& extents);
		int initVariables();
		void adjustTables();
		void initCRatios();
		void initRefinement();
		void initTimes();
		void initRangeControllers();
		void setNewExtents();
		void update();
		void refreshTable();
		void generateTableColumns();
		void addCalculationToTable(string varname);
		void makeItRed();
		void updateSliders();
		void errReport(string msg) const;
		void rGridError(int ts, string varname);

		bool calcMean(string varname);
		bool calcMedian(string varname);
		bool calcStdDev(string varname);

		struct _statistics {
			size_t row;
			double min;
			double max;
			double mean;
			double median;
			double stddev;
		};

		unsigned char _MIN;
		unsigned char _MAX;
		unsigned char _MEAN;
		unsigned char _SIGMA;
		unsigned char _MEDIAN;
		unsigned char _calculations;

		sErrMsg* _errMsg;

		Range* _xRange;
		MinMaxSlider* _xMinSlider;
		MinMaxSlider* _xMaxSlider;
		MinMaxLineEdit* _xMinLineEdit;
		MinMaxLineEdit* _xMaxLineEdit;
		SinglePointSlider* _xSinglePointSlider;
		SinglePointLineEdit* _xSinglePointLineEdit;
		CenterSizeSlider* _xCenterSlider;
		CenterSizeSlider* _xSizeSlider;
		CenterSizeLineEdit* _xCenterLineEdit;
		CenterSizeLineEdit* _xSizeLineEdit;
		MinMaxTableCell* _xMinCell;
		MinMaxTableCell* _xMaxCell;
		MinMaxLabel* _minXMinLabel;
		MinMaxLabel* _minXMaxLabel;
		MinMaxLabel* _maxXMinLabel;
		MinMaxLabel* _maxXMaxLabel;
		MinMaxLabel* _centerXMinLabel;
		MinMaxLabel* _centerXMaxLabel;
		SizeLabel* _sizeXMinLabel;
		SizeLabel* _sizeXMaxLabel;
		MinMaxLabel* _spXMinLabel;
		MinMaxLabel* _spXMaxLabel;

		Range* _yRange;
		MinMaxSlider* _yMinSlider;
		MinMaxSlider* _yMaxSlider;
		MinMaxLineEdit* _yMinLineEdit;
		MinMaxLineEdit* _yMaxLineEdit;
		SinglePointSlider* _ySinglePointSlider;
		SinglePointLineEdit* _ySinglePointLineEdit;
		CenterSizeSlider* _yCenterSlider;
		CenterSizeSlider* _ySizeSlider;
		CenterSizeLineEdit* _yCenterLineEdit;
		CenterSizeLineEdit* _ySizeLineEdit;
		MinMaxTableCell* _yMinCell;
		MinMaxTableCell* _yMaxCell;
		MinMaxLabel* _minYMinLabel;
		MinMaxLabel* _minYMaxLabel;
		MinMaxLabel* _maxYMinLabel;
		MinMaxLabel* _maxYMaxLabel;
		MinMaxLabel* _centerYMinLabel;
		MinMaxLabel* _centerYMaxLabel;
		SizeLabel* _sizeYMinLabel;
		SizeLabel* _sizeYMaxLabel;
		MinMaxLabel* _spYMinLabel;
		MinMaxLabel* _spYMaxLabel;

		Range* _zRange;
		MinMaxSlider* _zMinSlider;
		MinMaxSlider* _zMaxSlider;
		MinMaxLineEdit* _zMinLineEdit;
		MinMaxLineEdit* _zMaxLineEdit;
		SinglePointSlider* _zSinglePointSlider;
		SinglePointLineEdit* _zSinglePointLineEdit;
		CenterSizeSlider* _zCenterSlider;
		CenterSizeSlider* _zSizeSlider;
		CenterSizeLineEdit* _zCenterLineEdit;
		CenterSizeLineEdit* _zSizeLineEdit;
		MinMaxTableCell* _zMinCell;
		MinMaxTableCell* _zMaxCell;
		MinMaxLabel* _minZMinLabel;
		MinMaxLabel* _minZMaxLabel;
		MinMaxLabel* _maxZMinLabel;
		MinMaxLabel* _maxZMaxLabel;
		MinMaxLabel* _centerZMinLabel;
		MinMaxLabel* _centerZMaxLabel;
		SizeLabel* _sizeZMinLabel;
		SizeLabel* _sizeZMaxLabel;
		MinMaxLabel* _spZMinLabel;
		MinMaxLabel* _spZMaxLabel;

		VAPoR::DataMgr* _dm;
//		RegularGrid* _rGrid;
		VAPoR::StructuredGrid* _rGrid;
		string _defaultVar;
		vector<string> _vars;
		vector<string> _vars3d;
		vector<string> _vars2d;
		vector<size_t> _cRatios;
		vector<double> _extents;
		vector<double> _fullExtents;
		vector<double> _uCoordMin;
		vector<double> _uCoordMax;
		map<string, _statistics> _stats;
		size_t _varRows;
		size_t _minTS;
		size_t _maxTS;
		int _times;
		int _refLevels;
		int _refLevel;
		int _regionSelection;
		size_t _cRatio;
		size_t _vCoordMin[3];
		size_t _vCoordMax[3];
		size_t _minTime;
		size_t _maxTime;
		bool _autoUpdate;
		bool _resettingRegion;
		bool _regionInitialized;
		bool _initialized;
		bool _slidersInitialized;
};
#endif
