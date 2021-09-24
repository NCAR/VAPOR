#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/NetCDFCFCollection.h>
#include <vapor/DerivedVar.h>
#include <vapor/DerivedVarMgr.h>
#include <vapor/UDUnitsClass.h>
#include <vapor/utils.h>
#include <vapor/DCCF.h>

#pragma once

namespace VAPoR {

//!
//! \class DCUGRID
//! \ingroup Public_VDCUGRID
//!
//! \brief Class for reading a UGRID data set
//! stored as a series
//! of NetCDF files: https://ugrid-conventions.github.io/ugrid-conventions/
//!
//! \author John Clyne
//! \date    July, 2021
//!
class VDF_API DCUGRID : public VAPoR::DCCF {
public:
    int OpenVariableRead(size_t ts, string varname, int level = 0, int lod = 0) override;

    int Read(int fd, int *data) override;
    int ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *data) override;

    int CloseVariable(int fd) override;

protected:
    int initialize(const vector<string> &paths, const std::vector<string> &options) override;

    int initAuxilliaryVars(NetCDFCFCollection *ncdfc, std::map<string, DC::AuxVar> &auxVarsMap) override;

    int initDataVars(NetCDFCFCollection *ncdfc, std::map<string, DC::DataVar> &dataVarsMap) override;

    int initMesh(NetCDFCFCollection *ncdfc, std::map<string, DC::Mesh> &meshMap) override;

    string getMapProjection() const override { return ("+proj=eqc +ellps=WGS84 +lon_0=0.0 +lat_0=0.0"); }

private:
    // Struct to contain the most general form of UGRID mesh
    //
    struct uGridMeshType {
        int            topology;
        vector<string> nodeCoordinates;
        string         faceNodeConnectivity;
        string         faceDimension;
        string         edgeNodeConnectivity;
        string         edgeDimension;
        string         faceEdgeConnectivity;
        string         faceFaceConnectivity;
        string         edgeFaceConnectivity;
        string         boundaryNodeConnectivity;
        vector<string> faceCoordinates;
        vector<string> edgeCoordinates;
    };
    std::map<string, uGridMeshType> _uGridMeshMap;

    std::map<string, vector<int>> _faceNodeConnectivityMap;
    std::map<int, string>         _openConnectivityMaps;

    string _getLayeredVerticalCoordVar(NetCDFCFCollection *ncdfc, string varName) const;

    void _getUGridMeshFromFile(NetCDFCFCollection *ncdfc, string meshVarName, uGridMeshType &m);

    string _getMeshNodeDimName(NetCDFCFCollection *ncdf, const uGridMeshType &m) const;

    string _getMeshFaceDimName(NetCDFCFCollection *ncdf, const uGridMeshType &m) const;

    size_t _getMeshMaxNodesPerFace(NetCDFCFCollection *ncdf, const uGridMeshType &m) const;

    bool _getVarTimeCoords(NetCDFCFCollection *ncdfc, string varName, string &coordName) const;

    int _initFaceNodeConnectivityMap(NetCDFCFCollection *ncdfc);
};
};    // namespace VAPoR
