#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <cmath>

#include <vapor/DCUtils.h>
#include <vapor/DCUGRID.h>
#include <vapor/VAssert.h>

using namespace VAPoR;
using namespace std;

namespace {

// Attribute names and values for "dummy" UGRID mesh variable
//
const string cfRoleAttName = "cf_role";
const string cfRoleValueName = "mesh_topology";
const string topologyAttName = "topology_dimension";
const string nodeCoordinatesAttName = "node_coordinates";
const string faceNodeConnectivityAttName = "face_node_connectivity";
const string faceDimensionAttName = "face_dimension";
const string edgeNodeConnectivityAttName = "edge_node_connectivity";
const string edgeDimensionAttName = "edge_dimension";
const string faceEdgeConnectivityAttName = "face_edge_connectivity";
const string faceFaceConnectivityAttName = "face_face_connectivity";
const string edgeFaceConnectivityAttName = "edge_face_connectivity";
const string boundaryNodeConnectivityAttName = "boundary_node_connectivity";
const string faceCoordinatesAttName = "face_coordinates";
const string edgeCoordinatesAttName = "edge_coordinates";

// Attribute names and values for data variables
//
const string meshAttName = "mesh";
const string locationAttName = "location";


template<class T> int getVar(NetCDFCFCollection *ncdfc, size_t ts, string varname, T *buf)
{
    int fd = ncdfc->OpenRead(ts, varname);
    if (fd < 0) return (fd);

    int rc = ncdfc->Read(buf, fd);
    if (rc < 0) return (fd);

    return (ncdfc->Close(fd));
}

// Read a netcdf attribute of type int
//
void getAttInt(NetCDFCFCollection *ncdfc, string varName, string attName, int &v)
{
    v = 0;
    vector <long> values;
    ncdfc->GetAtt(varName, attName, values);
    if (values.size())
    {
        v = values[0];
    }
}

// Read a netcdf attribute of type string
//
void getAttString(NetCDFCFCollection *ncdfc, string varName, string attName, string &v)
{
    v = "";
    ncdfc->GetAtt(varName, attName, v);
}

bool isUGridDummyVar(NetCDFCFCollection *ncdfc, string varName)
{
    string v;
    getAttString(ncdfc, varName, cfRoleAttName, v);
    return(v == cfRoleValueName);
}

void unzipLongitude(vector <int> &connVar, size_t numFaces, size_t maxNodes, const vector <float> &lonVar, int missingNode) {

    VAssert(connVar.size() == numFaces * maxNodes);

	for (size_t j=0; j<numFaces; j++) {

		// Sanitize the connectivity array
		//
		for (size_t i=0; i < maxNodes; i++) {
			int nodeIdx = connVar[j*maxNodes + i];
			if (nodeIdx < 0 || nodeIdx >= lonVar.size()) {
				connVar[j*maxNodes + i] = missingNode;
			}
		}


		// Find first valid node
		//
		size_t i = 0;
		for (; i < maxNodes; i++) {
			int nodeIdx = connVar[j*maxNodes + i];
			if (nodeIdx != missingNode) {
				break;
			}
		}

		float p0 = 0.0;
		if (i < maxNodes) {
				int nodeIdx = connVar[j*maxNodes + i];
				p0 = lonVar[nodeIdx];
				i++;
		}
		for(; i<maxNodes; i++) {
			int nodeIdx = connVar[j*maxNodes + i];
			if (nodeIdx == missingNode || (std::abs(p0 - lonVar[nodeIdx])) > 180.0) {
				connVar[j*maxNodes + i] = -2;
			}
		}
	}
}

};    // namespace






// The vertical coordinate for a "layered" mesh is not explicity identified
// by a mesh "dummy" variable. Instead we have to identify them using
// the rules of the NetCDF CF conventions upon which UGRID is based
//
string DCUGRID::_getLayeredVerticalCoordVar(NetCDFCFCollection *ncdfc, string varName) const {

    // First check for CF "1D coordinate" variable. I.e a variable
    // with the same name as the vertical dimension
    //
	vector <string> dimNames = ncdfc->GetSpatialDimNames(varName);
    if (dimNames.size() != 2) return("");

    if (ncdfc->IsCoordVarCF(dimNames[0]) && ncdfc->IsVertCoordVar(dimNames[0])) {
        return(dimNames[0]);
    }

    // We didn't find a "1D coordinate" variable, so see if varName's
    // "coordinates" attribute names a vertical coordinate. I.e. look
    // for what the CF conventions call an "auxilliary" variable
    //
	string meshName;
	getAttString(ncdfc, varName, meshAttName, meshName);

	if (meshName.empty() || (_uGridMeshMap.find(meshName) == _uGridMeshMap.end())) {
        return("");
	}

    string s;
    ncdfc->GetAtt(varName, "coordinates", s);

    vector <string> atts;
    Wasp::StrToWordVec(s, atts);
    for (auto att: atts) {
        if (ncdfc->IsVertCoordVar(att)) return(att);
    }

    return("");
}


// Read metadata for a UGRID "dummy" mesh variable, but do no validatation
//
void DCUGRID::_getUGridMeshFromFile(NetCDFCFCollection *ncdfc, string meshVarName, uGridMeshType &m)
{
	m.topology = 0;
	m.nodeCoordinates = {};
	m.faceNodeConnectivity = "";
	m.faceDimension = "";
	m.edgeNodeConnectivity = "";
	m.edgeDimension = "";
	m.faceEdgeConnectivity = "";
	m.faceFaceConnectivity = "";
	m.edgeFaceConnectivity = "";
	m.boundaryNodeConnectivity = "";
	m.faceCoordinates = {};
	m.edgeCoordinates = {};

    string s;

	getAttInt(ncdfc, meshVarName, topologyAttName, m.topology);

	getAttString(ncdfc, meshVarName, nodeCoordinatesAttName, s);
    Wasp::StrToWordVec(s, m.nodeCoordinates);

	getAttString(ncdfc, meshVarName, faceNodeConnectivityAttName, m.faceNodeConnectivity);
	getAttString(ncdfc, meshVarName, faceDimensionAttName, m.faceDimension);
	getAttString(ncdfc, meshVarName, edgeNodeConnectivityAttName, m.edgeNodeConnectivity);
	getAttString(ncdfc, meshVarName, edgeDimensionAttName, m.edgeDimension);
	getAttString(ncdfc, meshVarName, faceEdgeConnectivityAttName, m.faceEdgeConnectivity);
	getAttString(ncdfc, meshVarName, faceFaceConnectivityAttName, m.faceFaceConnectivity);
	getAttString(ncdfc, meshVarName, edgeFaceConnectivityAttName, m.edgeFaceConnectivity);
	getAttString(ncdfc, meshVarName, boundaryNodeConnectivityAttName, m.boundaryNodeConnectivity);

	getAttString(ncdfc, meshVarName, faceCoordinatesAttName, s);
    Wasp::StrToWordVec(s, m.faceCoordinates);

	getAttString(ncdfc, meshVarName, edgeCoordinatesAttName, s);
    Wasp::StrToWordVec(s, m.edgeCoordinates);

}

// Get the meshes node dimension by way of meshes' coordinate variables
//
string DCUGRID::_getMeshNodeDimName(NetCDFCFCollection *ncdfc, const uGridMeshType &m) const
{

    if (! m.nodeCoordinates.size()) { return("");}

    // Pick the first coordinate variable. According to the spec they must
    // all be dimension'd by nNodes
    //
    string cVarName = m.nodeCoordinates[0];

    vector <string> dimNames = ncdfc->GetSpatialDimNames(cVarName);
    if (! dimNames.size()) { return("");}

    return(dimNames[0]);
}

// Get the meshes face dimension by way of meshes' face-node connectivity
// variables
//
string DCUGRID::_getMeshFaceDimName(NetCDFCFCollection *ncdfc, const uGridMeshType &m) const
{
    if (m.faceNodeConnectivity.empty()) { return("");}

    vector <string> dimNames = ncdfc->GetSpatialDimNames(m.faceNodeConnectivity);
    if (! dimNames.size()) { return("");}

    return(dimNames[0]);
}

// Get the meshes max nodes per face
//
size_t DCUGRID::_getMeshMaxNodesPerFace(NetCDFCFCollection *ncdfc, const uGridMeshType &m) const
{
    if (m.faceNodeConnectivity.empty()) { return(0);}

    vector <size_t> dimLens = ncdfc->GetSpatialDims(m.faceNodeConnectivity);
    if ( dimLens.size() < 2) { return(0);}

    return(dimLens[1]);
}
    

// Get the time coordinate variable for a named variable
//
bool DCUGRID::_getVarTimeCoords(NetCDFCFCollection *ncdfc, string varName, string &coordName) const 
{
    coordName.clear();

    vector <string> vars = ncdfc->GetTimeCoordVars();

    string timeDimName = ncdfc->GetTimeDimName(varName);

    if (find(vars.begin(), vars.end(), timeDimName) != vars.end()) {
        coordName = timeDimName;
    }

    return(true);

}


int DCUGRID::_initFaceNodeConnectivityMap(NetCDFCFCollection *ncdfc) {
    _faceNodeConnectivityMap.clear();

    // We need to synthesize the node-on-face connectivity map for each
    // mesh, splitting
    // faces that straddle the min and max longitude extent
    //
    vector <string> meshNames = GetMeshNames();
    for (auto mName : meshNames) {
        DC::Mesh mesh;

        bool ok = GetMesh(mName, mesh);
        VAssert(ok);

        string connVarName = mesh.GetFaceNodeVar();

        // Get the longitude coordinate variable name for this mesh
        //
        string lonVarName;
        vector <string> coordVarNames = mesh.GetCoordVars();
        for (auto coordVarName : coordVarNames) {
            DC::CoordVar cvar;

            ok = GetCoordVarInfo(coordVarName, cvar);
            VAssert(ok);

            if (cvar.GetAxis() == 0) {
                lonVarName = coordVarName;
                break;
            }
        }
        if (lonVarName.empty()) {
            SetErrMsg("Invalid mesh %s : No longitude variable", mName.c_str());
            return(-1);
        }

        // If a connection variable has a Fill Value attribute it indicates
        // missing nodes from a face. I.e. faces that less than the maximum
        // number of nodes.
        //
        int missingNode = -1;
        vector <long> atts;
        ok = GetAtt(connVarName, "_FillValue", atts);
        if (ok && atts.size() > 0) missingNode = atts[0];
      

        vector <size_t> connVarDims;
        GetDimLens(connVarName, connVarDims);
        VAssert(connVarDims.size() == 2);

        vector <size_t> lonVarDims;
        GetDimLens(lonVarName, lonVarDims);
        VAssert(lonVarDims.size() == 1);

		// Allocate space for the synthetic variable
        //
        _faceNodeConnectivityMap[connVarName] = vector <int> (connVarDims[0]*connVarDims[1], 0);

        vector <int> &connVar = _faceNodeConnectivityMap[connVarName];
        int rc = getVar(ncdfc, 0, connVarName, connVar.data());
        if (rc<0) {
            SetErrMsg("Unable to read connectivity variable %s", connVarName.c_str());
            return(rc);
        }

        // N.B. Can't call DC::GetVar because this class overrides it
        //
        vector <float> lonVar(lonVarDims[0], 0);
        rc = getVar(ncdfc, 0, lonVarName, lonVar.data());
        if (rc<0) {
            SetErrMsg("Unable to read longitude coordinate variable %s", lonVarName.c_str());
            return(rc);
        }

        unzipLongitude(connVar, connVarDims[1], connVarDims[0], lonVar, missingNode);

    }

    return(0);
}

int DCUGRID::initialize(const vector<string> &paths, const std::vector<string> &options) {

    int rc = DCCF::initialize(paths, options);
    if (rc<0) return(rc);

    return(_initFaceNodeConnectivityMap(_ncdfc));

}

int DCUGRID::initMesh(NetCDFCFCollection *ncdfc, std::map<string, DC::Mesh> &meshMap)   
{
    //
    // Get names of variables of all the 0D variables so we can look
    // for the UGRID "dummy" variables. N.B. we can't use 
    // NetCDFCFCollection::GetDataVariableNames() because that class
    // distinguishes data variables from other variables in a way
    // that is not compatible with UGRID
    //
    vector<string> vars = ncdfc->GetVariableNames(0, true);

	for (auto v : vars) {
        if (! isUGridDummyVar(ncdfc, v)) { continue; }

        uGridMeshType m;
        
        _getUGridMeshFromFile(ncdfc, v, m);

        _uGridMeshMap[v] = m;
    }

    // Next go through list of all variables in the file, identify those that
    // are associated with one of the meshes we just found, and convert
    // from uGridMeshType to DC::Mesh
    //  
    vars.clear();
    for (int i = 1; i < 4; i++) {
        vector<string> v = ncdfc->GetVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }   
        
    // For each variable determine the mesh
    //  
    for (auto varName : vars) {
        // variable type must be float or int
        //
        int type = ncdfc->GetXType(varName);
        if (!(NetCDFSimple::IsNCTypeFloat(type) || NetCDFSimple::IsNCTypeInt(type))) continue;

		string meshName;
        getAttString(ncdfc, varName, meshAttName,meshName);

		if (meshName.empty() || (_uGridMeshMap.find(meshName) == _uGridMeshMap.end())) {
            continue;
        }

        auto itr = _uGridMeshMap.find(meshName);
        if (itr == _uGridMeshMap.end()) { continue; }

        const uGridMeshType &m = itr->second;

        string nodeDimName = _getMeshNodeDimName(ncdfc, m);
        string faceDimName = _getMeshFaceDimName(ncdfc, m);
        size_t maxNodesPerFace = _getMeshMaxNodesPerFace(ncdfc, m);

        // We could calculate the max faces per node, but this isn't
        // used and should probably be removed from the Mesh class
        //
        size_t maxFacesPerNode = 0;
        
        vector <string> dimNames = ncdfc->GetSpatialDimNames(varName);
        reverse(dimNames.begin(), dimNames.end());


        if (dimNames.size() == 1) {
			// Create new mesh. We're being lazy here and probably should only
			// create one if it doesn't ready exist
			//
			meshMap[meshName] = Mesh(meshName, maxNodesPerFace, maxFacesPerNode, nodeDimName, faceDimName, m.nodeCoordinates, m.faceNodeConnectivity, "");
        }
        else if (dimNames.size() == 2) {

            // Layered grids (dimNames.size() == 2) require special handling
            // to identify the vertical coordinate. Also, because non layered 
            // (2D) and layered (3D) variables can share the same UGRID mesh
            // we need to generate a unique mesh name for one of them
            //

            string layeredVarName = _getLayeredVerticalCoordVar(ncdfc, varName);
            if (layeredVarName.empty()) continue;

            vector <string> coordVarNames = m.nodeCoordinates;
            coordVarNames.push_back(layeredVarName);

            meshName += layeredVarName;

			meshMap[meshName] = Mesh(meshName, maxNodesPerFace, maxFacesPerNode, nodeDimName, faceDimName, dimNames[1], coordVarNames, m.faceNodeConnectivity, "");
        }
        else {
            continue;
        }
    }

    // Should the correct behavior be to return an error code if 
    // no valid meshes are found? Right now need this is done so that MainForm
    // can auto-detect the file. We should probably introduce a separate
    // detection method on the DC class.
    //
    if (meshMap.size() == 0) {
        SetErrMsg("No valid UGRID meshes found");
        return(-1);
    }

    return(0);
}

int DCUGRID::initAuxilliaryVars(NetCDFCFCollection *ncdfc, std::map<string, DC::AuxVar> &auxVarsMap)
{
    auxVarsMap.clear();

    vector<bool> periodic(3, false);
    //
    // Get names of variables in the UGRID data set that have 1 or 2
    // "spatial" dimensions
    //
    vector<string> vars;
    for (int i = 1; i < 3; i++) {
        vector<string> v = ncdfc->GetVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    for (auto varName : vars) {
        // variable type must be int
        //
        int type = ncdfc->GetXType(varName);
        if (!(NetCDFSimple::IsNCTypeInt(type))) { continue; }

        vector<string> dimnames = ncdfc->GetSpatialDimNames(varName);
        if (!dimnames.size()) continue;

        reverse(dimnames.begin(), dimnames.end());

        auxVarsMap[varName] = AuxVar(varName, "", DC::INT32, "", vector<size_t>(), periodic, dimnames);

    }

    return (0);
}

int DCUGRID::initDataVars(NetCDFCFCollection *ncdfc, std::map<string, DC::DataVar> &dataVarsMap)
{
    dataVarsMap.clear();

    vector<bool> periodic(3, false);
    //
    // Get names of variables in the UGRID data set that have 1 or 2
    // "spatial" dimensions
    //
    vector<string> vars;
    for (int i = 1; i < 3; i++) {
        vector<string> v = ncdfc->GetVariableNames(i, true);
        vars.insert(vars.end(), v.begin(), v.end());
    }

    // For each variable add a member to dataVarsMap
    //
    for (auto varName : vars) {

        // variable type must be float
        //
        int type = ncdfc->GetXType(varName);
        if (!(NetCDFSimple::IsNCTypeFloat(type))) { continue; }

        // Checking for whether a variable can be a coordinate variable
        // eliminates a lot of variables from being data variables. There
        // doesn't seem to be any reason why a variable can't be both a
        // data variable and a coordinate variable. Leaving this code 
        // commented out for now.
        //
        // if (IsCoordVar(varName)) {continue; }
        // Currently coordinate variables can't be data variables :-(
        //
        if (ncdfc->IsCoordinateVar(varName)) continue;

        // variable must have valid mesh attribute
        //
		string meshName;
        getAttString(ncdfc, varName, meshAttName,meshName);

		if (meshName.empty()) { continue; }

        if (_uGridMeshMap.find(meshName) == _uGridMeshMap.end()) {
            SetDiagMsg("No valid mesh for variable named %s", varName.c_str());
            continue;
        }

        // Need to generate a unique mesh name for layered grids since
        // UGRID allows 2D and 3D layered variables to share same mesh, 
        // but DC class does not.
        //
        vector <string> dimNames = ncdfc->GetSpatialDimNames(varName);
        if (dimNames.size() == 2) {
            meshName += _getLayeredVerticalCoordVar(ncdfc, varName);
        }

        // Only node-centered variables supported currently
        //
		string locationName;
        getAttString(ncdfc, varName, locationAttName,locationName);

		if (locationName != "node") {
            SetDiagMsg("Only node-centered data supported for variable named %s", varName.c_str());
            continue;
        }

        string coordName;
        bool ok = _getVarTimeCoords(ncdfc, varName, coordName);
        if (! ok) {
            SetDiagMsg("Invalid variable : %s", varName.c_str());
            continue;
        }

        string units;
        ncdfc->GetAtt(varName, "units", units);

        double mv;
        bool   has_missing = ncdfc->GetMissingValue(varName, mv);

        if (!has_missing) {
            dataVarsMap[varName] = DataVar(varName, units, DC::FLOAT, periodic, meshName, coordName, DC::Mesh::NODE);
        } else {
            dataVarsMap[varName] = DataVar(varName, units, DC::FLOAT, periodic, meshName, coordName, DC::Mesh::NODE, mv);
        }

        int rc = DCUtils::CopyAtt(*ncdfc, varName, dataVarsMap[varName]);
        if (rc < 0) return (-1);
    }

    return (0);
}

int DCUGRID::OpenVariableRead(size_t ts, string varname, int level, int lod) {

    // Pass through normal open operation if this is not a synthesized 
    // connectivity variable
    //
    if (_faceNodeConnectivityMap.find(varname) == _faceNodeConnectivityMap.end()) {
        return(DC::OpenVariableRead(ts, varname, level, lod));
    }

    FileTable::FileObject *f = new FileTable::FileObject(ts, varname);
    return (_fileTable.AddEntry(f));
}

int DCUGRID::Read(int fd, int *data) {

	DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    // Pass through normal read operation if this is not a synthesized 
    // connectivity variable
    //
    if (_faceNodeConnectivityMap.find(f->GetVarname()) == _faceNodeConnectivityMap.end()) {
        return(DC::Read(fd, data));
    }

    const vector <int> &connVar = _faceNodeConnectivityMap[f->GetVarname()];
    std::copy_n(connVar.data(), connVar.size(), data);
    return(0);
}

int DCUGRID::ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *data)
{

	DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    if (_faceNodeConnectivityMap.find(f->GetVarname()) == _faceNodeConnectivityMap.end()) {
        return(DC::ReadRegion(fd, min, max, data));
    }
    VAssert(min.size() == max.size());

    size_t n = 1;
    for (int i=0; i<min.size(); i++) {
        VAssert(min[i] == 0);
        n *= max[i]+1;
    }

    const vector <int> &connVar = _faceNodeConnectivityMap[f->GetVarname()];
    VAssert(n == connVar.size());
    std::copy_n(connVar.data(), connVar.size(), data);
    return(0);
}

int DCUGRID::CloseVariable(int fd) {

	DC::FileTable::FileObject *f = _fileTable.GetEntry(fd);

    if (!f) {
        SetErrMsg("Invalid file descriptor : %d", fd);
        return (-1);
    }

    if (_faceNodeConnectivityMap.find(f->GetVarname()) == _faceNodeConnectivityMap.end()) {
        return(DC::CloseVariable(fd));
    }

    _fileTable.RemoveEntry(fd);
    delete f;

    return(0);
}
