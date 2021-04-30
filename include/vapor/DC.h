#include <vector>
#include <algorithm>
#include <map>
#include <iostream>
#include <vapor/MyBase.h>

#ifndef _DC_H_
    #define _DC_H_

namespace VAPoR {

//!
//! \class DC
//! \ingroup Public_VDC
//!
//! \brief A Template Method design pattern for reading a collection of data.
//!
//! \author John Clyne
//! \date    January, 2015
//!
//! The Data Collection (DC) class defines an Template Method for
//! reading metadata and sampled
//! data from a data collection.  A data collection is a set of
//! related data, most typically the discrete outputs from a single numerical
//! simulation. The DC class is a Template Method: if provides an
//! abstract interface for accessing a data collection, and a set of
//! protected pure virtual functions that must be implemented by
//! derived classes providing access to a particular file format.
//!
//! Variables in a DC may have 1, 2, or 3 topological dimensions, and 0 or 1
//! temporal dimensions.
//!
//! The DC is structured in the spirit of the "NetCDF Climate and Forecast
//! (CF) Metadata Conventions", version 1.6, 5, December 2011.
//! It supports only a subset of the CF functionality (e.g. there is no
//! support for "Discrete Sampling Geometries"). Moreover, it is
//! more restrictive than the CF in a number of areas. Particular
//! items of note include:
//!
//! \li The API supports variables with 0 to 3 topological dimensions only.
//!
//! \li Coordinate variables representing time must be 1D
//!
//! \li All data variables have a "coordinate" attribute identifying
//! the coordinate (or auxiliary coordinate) variables associated with
//! each axis
//!
//! \li To be consistent with VAPOR, when specified in vector form the
//! ordering of dimension lengths
//! and dimension names is from fastest varying dimension to slowest.
//! For example, if
//! 'dims' is a vector of dimensions, then dims[0] is the fastest varying
//! dimension, dim[1] is the next fastest, and so on. This ordering is the
//! opposite of the ordering used by NetCDF.
//!
//! \li The API supports unstructured grids and attempts to follow
//! the UGRID conventions.
//!
//! This class inherits from Wasp::MyBase. Unless otherwise documented
//! any method that returns an integer value is returning status. A negative
//! value indicates failure. Error messages are logged via
//! Wasp::MyBase::SetErrMsg(). Methods that return a boolean do
//! not, unless otherwise documented, log an error message upon
//! failure (return of false).
//!
//! \param level
//! \parblock
//! Grid refinement level for multiresolution variables.
//! Compressed variables in the DC, if they exist, have a multi-resolution
//! representation: the sampling grid for multi-resolution variables
//! is hierarchical, and the dimension lengths of adjacent levels in the
//! hierarchy differ by a factor of two. The \p level parameter is
//! used to select a particular depth of the hierarchy.
//!
//! To provide maximum flexibility as well as compatibility with previous
//! versions of the DC the interpretation of \p level is somewhat
//! complex. Both positive and negative values may be used to specify
//! the refinement level and have different interpretations.
//!
//! For positive
//! values of \p level, a value of \b 0 indicates the coarsest
//! member of the
//! grid hierarchy. A value of \b 1 indicates the next grid refinement
//! after the coarsest, and so on. Using postive values the finest level
//! in the hierarchy is given by GetNumRefLevels() - 1. Values of \p level
//! that are greater than GetNumRefLevels() - 1 are treated as if they
//! were equal to GetNumRefLevels() - 1.
//!
//! For negative values of \p level a value of -1 indicates the
//! variable's native grid resolution (the finest resolution available).
//! A value of -2 indicates the next coarsest member in the hierarchy after
//! the finest, and so
//! on. Using negative values the coarsest available level in the hierarchy is
//! given by negating the value returned by GetNumRefLevels(). Values of
//! \p level that are less than the negation of GetNumRefLevels() are
//! treated as if they were equal to the negation of the GetNumRefLevels()
//! return value.
//! \endparblock
//! \param lod
//! \parblock
//! The level-of-detail parameter, \p lod, selects
//! the approximation level for a compressed variable.
//! The \p lod parameter is similar to the \p level parameter in that it
//! provides control over accuracy of a compressed variable. However, instead
//! of selecting the grid resolution the \p lod parameter controls
//! the compression factor by indexing into the \p cratios vector (see below).
//! As with the \p level parameter, both positive and negative values may be
//! used to index into \p cratios and
//! different interpretations.
//!
//! For positive
//! values of \p lod, a value of \b 0 indicates the
//! the first element of \p cratios, a value of \b 1 indicates
//! the second element, and so on up to the size of the
//! \p cratios vector (See DC::GetCRatios()).
//!
//! For negative values of \p lod a value of \b -1 indexes the
//! last element of \p cratios, a value of \b -2 indexes the
//! second to last element, and so on.
//! Using negative values the first element of \p cratios - the greatest
//! compression rate - is indexed by negating the size of the
//! \p cratios vector.
//! \endparblock
//! \param cratios A monotonically decreasing vector of
//! compression ratios. Compressed variables in the DC are stored
//! with a fixed, finite number of compression factors. The \p cratios
//! vector is used to specify the available compression factors (ratios).
//! A compression factor of 1 indicates no compression (1:1). A value
//! of 2 indciates two to one compression (2:1), and so on. The minimum
//! valid value of \p cratios is \b 1. The maximum value is determined
//! by a number of factors and can be obtained using the CompressionInfo()
//! method.
//!
//! \param bs An ordered list of block dimensions that specifies the
//! spatial block decomposition of the variable. The rank of \p bs may be less
//! than that of a variable's array dimensions (or empty), in which case only
//! the \b n fastest varying variable dimensions will be blocked, where
//! \b n is the rank of \p bs. The ordering of the dimensions in \p bs
//! is from fastest to slowest. A block is the basic unit of compression
//! in the DC: variables are decomposed into blocks, and individual blocks
//! are compressed independently. Note, the time dimension is never blocked.
//!
//! \param wname Name of wavelet used for transforming compressed
//! variables between wavelet and physical space. Valid values
//! are "bior1.1", "bior1.3", "bior1.5", "bior2.2", "bior2.4",
//! "bior2.6", "bior2.8", "bior3.1", "bior3.3", "bior3.5", "bior3.7",
//! "bior3.9", "bior4.4"
//!
//!
class VDF_API DC : public Wasp::MyBase {
public:
    //! External storage types for primitive data
    //
    enum XType { INVALID = -1, FLOAT, DOUBLE, UINT8, INT8, INT32, INT64, TEXT };

    //! \class Dimension
    //!
    //! \brief Metadata describing a named dimension length
    //!
    //! Describes an array dimension with a name and an
    //! associated length. Dimension lengths may vary (e.g. over time).
    //!
    class Dimension {
    public:
        Dimension()
        {
            _name.clear();
            _lengths.clear();
        }

        //! Dimension class constructor for multi-length dimension
        //!
        //! \param[in] name The name of dimension
        //! \param[in] lengths A vector of dimension lengths.
        //!
        Dimension(std::string name, std::vector<size_t> lengths)
        {
            _name = name;
            _lengths = lengths;
        };

        //! Dimension class constructor for constant-length dimension
        //!
        //! \param[in] name The name of dimension
        //! \param[in] length The dimension length.
        //!
        Dimension(std::string name, size_t length)
        {
            _name = name;
            _lengths.clear();
            _lengths.push_back(length);
        };

        virtual ~Dimension(){};

        //! Get dimension name
        //
        string GetName() const { return (_name); };

        //! Return dimension length
        //!
        //! For a multi-length dimension the first length is returned
        //!
        size_t GetLength() const { return (_lengths.size() ? _lengths[0] : 0); };

        //! Return a dimension length
        //!
        //! \param[in] index Return the length of the dimension for the indicated
        //! element. If \p index is out of range the value 0 is returned
        //!
        size_t GetLength(size_t index) const { return (index < _lengths.size() ? _lengths[index] : 0); };

        //! Return boolean indicating whether dimension can vary over time
        //!
        //! This method returns true if the dimesion length can vary over time.
        //
        bool IsTimeVarying() const { return (_lengths.size() > 1); }

        friend std::ostream &operator<<(std::ostream &o, const Dimension &dimension);

    private:
        string              _name;
        std::vector<size_t> _lengths;
    };

    //! \class Mesh
    //! \version 3.1
    //!
    //! \brief Metadata describing a computational mesh
    //!
    //! This class describes the properties of a computational mesh upon
    //! which data variables are sampled. The class can represent both
    //! structured and unstructured grids.
    //! The design of the Mesh class is inspired by the capabilities of the
    //! UGRID conventions:
    //! <a href=http://ugrid-conventions.github.io/ugrid-conventions/>UGRID</a>
    //!
    //! Towards that end we adopt some of UGRID's terminology:
    //!
    //! \b Definitions
    //!
    //! \li \b node A point, a coordinate pair or triplet: the most basic
    //! element of the topology. Aka "vertext".
    //!
    //! \li \b edge A line bounded by two nodes.
    //!
    //! \li \b face A plane or surface enclosed by a set of edges. In a 2D
    //! horizontal application one may consider the word "polygon", but in the
    //! hierarchy of elements the word "face" is most common.
    //!
    //! \li \b volume A volume enclosed by a set of faces. Aka "cell".
    //!
    //! \li \b topology The topology defines the connectivity of the
    //! vertices and defines the elements.
    //!
    //! \param[in] name A string containing the name of the mesh.
    //!
    //! \param[in] coord_vars A list of names of the coordinate variables,
    //! each specifying the X, Y, or Z spatial coordinates for the nodes
    //! in the mesh.  If a coordinate variable for an X, Y, or Z axis
    //! is not specified the node's coordinate's for the missing axis are
    //! assumed to be zero. The dimensionality of the coordinate variable
    //! may be less than that of the dimensionality of the grid, in which
    //! case the coordinates along the unspecified dimension are assumed
    //! invariant.
    //!
    //! \param[in] max_nodes_per_face Specifies maximum number of nodes
    //! (or edges) a face of the mesh may have.
    //!
    //! \param[in] max_faces_per_node Specifies maximum number of faces
    //! that may share a node
    //!
    //! \param[in] node_dim_name A string containing the name of the dimension
    //! specifying the total number of nodes in the unstructured
    //! portion of the mesh; for layered-grid unstructured meshes \p node_dim_name
    //! specifies the number of nodes in a single layer
    //!
    //! \param[in] face_dim_name A string containing the name of the dimension
    //! specifying the total number of faces in the unstructured
    //! portion of the mesh; for layered-grid unstructured meshes \p face_dim_name
    //! specifies the number of faces in a single layer
    //!
    //! \param[in] face_node_var  The name of a 2D Auxiliary variable
    //! (See DC:AuxVar) with
    //! integer type identifying for
    //! every face the indices of its corner nodes. The corner nodes should be
    //! specified in anticlockwise direction as viewed from above The
    //! connectivity array will be a matrix of size
    //! face_dim x \p max_nodes_per_face, where face_dim is the length of the
    //! dimension named by \p face_dim_name, and is the slowest varying
    //! of the two dimensions. If a face has less corner nodes
    //! \p max_nodes_per_face then the last node indices shall be equal to -1.
    //! Indecies start from zero.
    //!
    //! \param[in] node_face_var  The name of a 2D Auxiliary variable
    //! (See DC:AuxVar) with
    //! integer type identifying for
    //! every node the indices of the faces that share the node.
    //! The faces should be
    //! specified in anticlockwise direction as viewed from above The
    //! connectivity array will be a matrix of size
    //! node_dim x \p max_faces_per_node , where node_dim is the length of the
    //! dimension named by \p node_dim_name, and is the slowest varying
    //! of the two dimensions. If a node has less faces then
    //! \p max_faces_per_node then the last face indices shall be equal to -1.
    //! Indecies start from zero.
    //!
    //! \note The \p node_face_var is not defined by the UGRID conventions
    //!
    class Mesh {
    public:
        //! Type of mesh
        //
        enum Type {
            STRUCTURED,         //!< A structured mesh
            UNSTRUC_2D,         //!< An unstructured mesh with 2D topology
            UNSTRUC_LAYERED,    //!< An unstructured layered mesh
            UNSTRUC_3D          //!< An fully unstructured mesh with 3D topology
        };

        //! Location of sampled data variables within the mesh
        //
        enum Location {
            NODE,     //!< Samples located at mesh nodes
            EDGE,     //!< Samples located at mesh edge centers
            FACE,     //!< Samples located at mesh face centers
            VOLUME    //!< Samples located at mesh volume centers
        };

        Mesh()
        {
            _name.clear();
            _dim_names.clear();
            _coord_vars.clear();
            _max_nodes_per_face = 0;
            _max_faces_per_node = 0;
            _node_dim_name.clear();
            _face_dim_name.clear();
            _layers_dim_name.clear();
            _face_node_var.clear();
            _node_face_var.clear();
            _face_edge_var.clear();
            _face_face_var.clear();
            _edge_dim_name.clear();
            _edge_node_var.clear();
            _edge_face_var.clear();
            _mtype = STRUCTURED;
        }

        //! Construct structured mesh
        //!
        //! This method constructs a structured mesh.
        //!
        //! \param[in] name Name of the mesh. If \p name is the empty string
        //! a name will be created from the concatenation of the elements
        //! of the \p dim_names parameter
        //!
        //! \param[in] dim_names An ordered list of names of the
        //! spatial dimensions of
        //! the mesh. The ordering is from fastest varying dimension to slowest.
        //! The number of elements in \p dim_names determines the dimensionality
        //! of the mesh, but not, in general, the topological dimension of the
        //! mesh. The rank of \p dim_names defines
        //! the topological dimension.
        //!
        Mesh(std::string name, std::vector<string> dim_names, std::vector<string> coord_vars);

        //! Construct unstructured 2D mesh
        //!
        //! This method constructs an unstructured mesh with 2D topology.
        //!
        Mesh(std::string name, size_t max_nodes_per_face, size_t max_faces_per_node, std::string node_dim_name, std::string face_dim_name, std::vector<std::string> coord_vars,
             std::string face_node_var, std::string node_face_var);

        //! Construct unstructured layered mesh
        //!
        //! This method constructs an unstructured layered mesh.
        //!
        //! \param[in] layers_dim_name Name of dimension specifying the number
        //! of layers in the mesh.
        //
        Mesh(std::string name, size_t max_nodes_per_face, size_t max_faces_per_node, std::string node_dim_name, std::string face_dim_name, std::string layers_dim_name,
             std::vector<std::string> coord_vars, std::string face_node_var, std::string node_face_var);

        //! Construct unstructured 3d mesh
        //!
        //! This method constructs an unstructured 3d mesh.
        //!
        //! \param[in] layers_dim_name Name of dimension specifying the number
        //! of layers in the mesh.
        //
        Mesh(
          std::string name,
          int max_nodes_per_face,
          int max_faces_per_node,
          std::string node_dim_name,
          std::string face_dim_name,
          std::vector <string> coord_vars
        );
        
        //! Return the type of mesh
        //!
        //! Returns one of:
        //!
        //! \li \c STRUCTURED A structured mesh
        //!
        //! \li \c UNSTRUC_2D An unstructured mesh with 2D topology
        //!
        //! \li \c UNSTRUC_LAYERED An unstructured layered mesh
        //!
        //! \li \c UNSTRUC_3D An fully unstructured mesh with 3D topology
        //!
        //!
        Mesh::Type GetMeshType() const { return (_mtype); }

        //! Get mesh name
        //
        string GetName() const { return (_name); };

        //! Get dim names
        //!
        //! Returns ordered list of dimension names for the mesh nodes. The
        //! ordering is from fastest to slowest varying dimension.
        //! For
        //! structured meshes the size of the returned vector matches the
        //! topological dimensionality of the mesh.
        //!
        //! For
        //! unstructured meshes the returned contains the names of the
        //! node dimensions.
        //
        std::vector<string> GetDimNames() const { return (_dim_names); };

        //! Get coordinate variable names
        //!
        //! Returns the list of coordinate variable names associated with
        //! the nodes of the mesh.
        //
        std::vector<string> GetCoordVars() const { return (_coord_vars); };
        void                SetCoordVars(std::vector<string> coord_vars) { _coord_vars = coord_vars; };

        //! Get geometric dimension of cells
        //!
        //! Returns the geometric dimension of the cells in the mesh. I.e.
        //! the number of spatial spatial coordinates for each grid point.
        //! Valid values are 0..3. The geometric dimension must be equal to
        //! or greater than the topology dimension.
        //!
        //! \sa GetTopologyDim()
        //
        size_t GetGeometryDim() const { return (_coord_vars.size()); }

        //! Get the maximum number of nodes per face
        //!
        //! Return the maximum number of nodes that a face in the mesh may have.
        //! For structured meshes this value is always four.
        //!
        size_t GetMaxNodesPerFace() const { return (_max_nodes_per_face); };

        //! Get the maximum number of faces per node
        //!
        //! Return the maximum number of faces that a node in the mesh may have.
        //! For structured meshes this value is always four.
        //!
        size_t GetMaxFacesPerNode() const { return (_max_faces_per_node); };

        //! Get the name of the node dimension
        //!
        //! For unstructured meshes this method returns the name of the
        //! node dimension. For structured meshes an emptry string is returned.
        //!
        string GetNodeDimName() const { return (_node_dim_name); };

        //! Get the name of the face dimension
        //!
        //! For unstructured meshes this method returns the name of the
        //! face dimension. For structured meshes an emptry string is returned.
        //!
        string GetFaceDimName() const { return (_face_dim_name); };

        //! Get the name of the layers dimension
        //!
        //! For unstructured layered meshes this method returns the name of the
        //! node dimension. For other meshes an empty string is returned.
        //!
        string GetLayersDimName() const { return (_layers_dim_name); };

        //! Get the name of face node connectivity var
        //!
        //! Return the name of the face node connecity variable
        //!
        string GetFaceNodeVar() const { return (_face_node_var); }
        void   SetFaceNodeVar(std::string face_node_var)
        {
            if (_mtype == STRUCTURED) return;
            _face_node_var = face_node_var;
        }

        //! Get the name of node face connectivity var
        //!
        //! Return the name of the node face connecity variable
        //!
        string GetNodeFaceVar() const { return (_node_face_var); }
        void   SetNodeFaceVar(std::string node_face_var)
        {
            if (_mtype == STRUCTURED) return;
            _node_face_var = node_face_var;
        }

        //! Set the name of the optional edge dimension
        //!
        //! For unstructured meshes this method sets the name of the optional
        //! edge dimension. It is only needed if data are located on mesh edges.
        //!
        //! \param[in] edge_dim_name Name of the edge dimension
        //!
        void SetEdgeDimName(std::string edge_dim_name)
        {
            if (_mtype == STRUCTURED) return;
            _edge_dim_name = edge_dim_name;
        }

        //! Get the name of the optional edge dimension
        //!
        //! This method gets the name of the optional
        //! edge dimension. If not explicitly set with SetEdgeDimName() an
        //! empty string is returned.
        //!
        std::string GetEdgeDimName() const { return (_edge_dim_name); }

        //! Set the name of optional edge node connectivity index variable
        //!
        //! For meshes with data located on edges this method specifies the
        //! name of an index variable identifying for every edge the indices of its
        //! begining and ending nodes. The connectivity array will thus be a matrix
        //! of size edge_dim x 2, where edge_dim is the dimension length named
        //! by edge_dim_name.
        //!
        //! \sa GetEdgeDimName()
        //!
        void SetEdgeNodeVar(std::string edge_node_var)
        {
            if (_mtype == STRUCTURED) return;
            _edge_node_var = edge_node_var;
        }

        //! Get the name of optional edge node connectivity index variable
        //!
        //! An emptry string is returned if not explicitly set with
        //! SetEdgeNodeVar()
        //
        std::string GetEdgeNodeVar() const { return (_edge_node_var); }

        //! Set the name of optional face edge connectivity index variable
        //!
        //! The name of an optional 2D Auxiliary variable
        //! (See DC:AuxVar) with
        //! integer type.
        //! If specified, names an index variable identifying
        //! for every face the indices of its edges. The edges should be specified
        //! in anticlockwise direction as viewed from above. This connectivity
        //! array will be a matrix of size face_dim x max_faces_per_node,
        //! where face_dim is the dimension length named by \p face_dim_name,
        //! and is the slowest varying of the two dimensions.
        //!  If a face
        //! has less corners/edges than max_faces_per_node then the last edge
        //! indices shall be equal to -1. The starting index is 0.
        //!
        //
        void SetFaceEdgeVar(std::string face_edge_var)
        {
            if (_mtype == STRUCTURED) return;
            _face_edge_var = face_edge_var;
        }

        //! Get the name of optional face edge connectivity index variable
        //!
        //! An emptry string is returned if not explicitly set with
        //! SetFaceEdgeVar()
        //
        std::string GetFaceEdgeVar() const { return (_face_edge_var); }

        //! Set the name of optional face-face connectivity index variable
        //!
        //! The name of an optional 2D Auxiliary variable
        //! (See DC:AuxVar) with
        //! integer type.
        //! If specified, names an index variable identifying
        //! for every face the indices of its adjacent faces. The faces should
        //! be specified
        //! in anticlockwise direction as viewed from above. This connectivity
        //! array will be a matrix of size face_dim x max_faces_per_node, where
        //! face_dim is the dimension length named by \p face_dim_name, and
        //! is the slowest varying of the two dimensions. If a face
        //! has less corners/edges than max_faces_per_node then the last face
        //! indices shall be equal to -1. The starting index is 0.
        //!
        //
        void SetFaceFaceVar(std::string face_face_var)
        {
            if (_mtype == STRUCTURED) return;
            _face_face_var = face_face_var;
        }

        //! Get the name of optional face edge connectivity index variable
        //!
        //! An emptry string is returned if not explicitly set with
        //! SetFaceFaceVar()
        //
        std::string GetFaceFaceVar() const { return (_face_face_var); }

        //! Set the name of optional edge-face connectivity index variable
        //!
        //! The name of an optional 2D Auxiliary variable
        //! (See DC:AuxVar) with
        //! integer type.
        //! If specified, names an index variable identifying
        //! for every edge the indices of its adjacent faces.
        //! This connectivity array is thus a matrix of size edge_dim x 2,
        //! where edge_dim is the dimension length named by \p edge_dim_name, and
        //! is the slowest varying of the two dimensions. It is
        //! intended to be used in combination with data defined on edges.
        //! The starting index is 0.
        //!
        //! \sa SetEdgeDimName()
        //
        void SetEdgeFaceVar(std::string edge_face_var)
        {
            if (_mtype == STRUCTURED) return;
            _edge_face_var = edge_face_var;
        }

        //! Get the name of optional face edge connectivity index variable
        //!
        //! An emptry string is returned if not explicitly set with
        //! SetEdgeFaceVar()
        //
        std::string GetEdgeFaceVar() const { return (_edge_face_var); }

        //! Get topological dimension of the mesh
        //!
        //! Return the number of topological dimensions for the mesh cells. Valid
        //! values are in the range 0..3, with, for example, 0 corresponding
        //! to points; 1 to lines; 2 to triangles, quadrilaterals, etc.; and
        //! 3 to hexahedron, tetrahedron, etc.
        //!
        //! \sa GetGeometryDim()
        //
        size_t GetTopologyDim() const;

        friend std::ostream &operator<<(std::ostream &o, const Mesh &mesh);

    private:
        string              _name;
        std::vector<string> _dim_names;
        std::vector<string> _coord_vars;
        size_t              _max_nodes_per_face;
        size_t              _max_faces_per_node;
        string              _node_dim_name;
        string              _face_dim_name;
        string              _layers_dim_name;
        string              _face_node_var;
        string              _node_face_var;
        string              _face_edge_var;
        string              _face_face_var;
        string              _edge_dim_name;
        string              _edge_node_var;
        string              _edge_face_var;
        Mesh::Type          _mtype;

        void _Mesh(string name, std::vector<string> coord_vars, int max_nodes_per_face, int max_faces_per_node, Type type);
    };

    //! \class Attribute
    //!
    //! \brief Variable or global metadata
    //!
    class Attribute {
    public:
        Attribute()
        {
            _name = "";
            _type = FLOAT;
            _values.clear();
        };

        //! Attribute constructor
        //!
        //! \param[in] name The name of the attribute
        //! \param[in] type External representation format
        //! \param[in] values A vector specifying the attribute's values
        //
        Attribute(string name, XType type, const std::vector<float> &values);
        Attribute(string name, XType type, const std::vector<double> &values);
        Attribute(string name, XType type, const std::vector<int> &values);
        Attribute(string name, XType type, const std::vector<long> &values);
        Attribute(string name, XType type, const string &values);
        Attribute(string name, XType type)
        {
            _name = name;
            _type = type;
            _values.clear();
        };
        virtual ~Attribute(){};

        //! Get attribute name
        //
        string GetName() const { return (_name); };

        //! Get an attribute's external representation type
        //
        XType GetXType() const { return (_type); };

        //! Get an attribute's value(s)
        //!
        //! Get the value(s) for an attribute, performing type conversion
        //! as necessary from the external storage type to the desired type
        //!
        void GetValues(std::vector<float> &values) const;
        void GetValues(std::vector<double> &values) const;
        void GetValues(std::vector<int> &values) const;
        void GetValues(std::vector<long> &values) const;
        void GetValues(string &values) const;

        //! Set an attribute's value(s)
        //!
        //! Set the value(s) for an attribute, performing type conversion
        //! as necessary to meet the external storage type.
        //!
        void SetValues(const std::vector<float> &values);
        void SetValues(const std::vector<double> &values);
        void SetValues(const std::vector<int> &values);
        void SetValues(const std::vector<long> &values);
        void SetValues(const string &values);

        friend std::ostream &operator<<(std::ostream &o, const Attribute &attr);

    private:
        string _name;
        XType  _type;
        union podunion {
            float  f;
            double d;
            int    i;
            long   l;
            char   c;
        };
        std::vector<podunion> _values;
    };

    //! \class BaseVar
    //!
    //! \brief Base class for storing variable metadata
    //
    class BaseVar {
    public:
        BaseVar()
        {
            _name.clear();
            _units.clear();
            _type = FLOAT;
            _wname.clear();
            _cratios.clear();
            _periodic.clear();
            _atts.clear();
        }

        //! Constructor
        //!
        //! \param[in] name The variable's name
        //!
        //! \param[in] units A string recognized by Udunits-2 specifying the
        //! unit measure for the variable. An empty string indicates that the
        //! variable is unitless.
        //! \param[in] type The external storage type for variable data
        //! \param[in] wname The wavelet family name for compressed variables
        //! \param[in] cratios Specifies a vector of compression factors for
        //! compressed variable definitions. If empty, or if cratios.size()==1
        //! and cratios[0]==1, the variable is not
        //! compressed
        //!
        //! \deprecated Results are undefined if the rank of
        //! of \p periodic does not match that of \p dimensions.
        //!
        BaseVar(string name, string units, XType type, string wname, std::vector<size_t> cratios, std::vector<bool> periodic)
        : _name(name), _units(units), _type(type), _wname(wname), _cratios(cratios), _periodic(periodic)
        {
            if (_cratios.size() == 0) _cratios.push_back(1);
        };

        //! No compression constructor
        //!
        //! \param[in] name The variable's name
        //! \deprecated \param[in] dimensions An ordered vector
        //! specifying the variable's spatial
        //! and/or temporal dimensions
        //!
        //! \param[in] units A string recognized by Udunits-2 specifying the
        //! unit measure for the variable. An empty string indicates that the
        //! variable is unitless.
        //! \param[in] type The external storage type for variable data
        //! factor for the variable.
        //! \deprecated \param[in] periodic An ordered array of booleans
        //! specifying the
        //! spatial boundary periodicity.
        //! Results are undefined if the rank of
        //! of \p periodic does not match that of \p dimensions.
        //!
        BaseVar(string name, string units, XType type, std::vector<bool> periodic);

        virtual ~BaseVar(){};

        //! Get variable name
        //
        string GetName() const { return (_name); };
        void   SetName(string name) { _name = name; };

        //! Access variable units
        //
        string GetUnits() const { return (_units); };
        void   SetUnits(string units) { _units = units; };

        //! Access variable external storage type
        //
        XType GetXType() const { return (_type); };
        void  SetXType(XType type) { _type = type; };

        //! Access variable's wavelet family name
        //
        string GetWName() const { return (_wname); };
        void   SetWName(string wname) { _wname = wname; };

        //! Access variable's compression ratios
        //
        std::vector<size_t> GetCRatios() const { return (_cratios); };

        void SetCRatios(std::vector<size_t> cratios)
        {
            _cratios = cratios;
            if (_cratios.size() == 0) _cratios.push_back(1);
        };

        //! \deprecated Access variable bounary periodic
        //
        std::vector<bool> GetPeriodic() const { return (_periodic); };
        void              SetPeriodic(std::vector<bool> periodic) { _periodic = periodic; };

        //! Access variable attributes
        //
        const std::map<string, Attribute> &GetAttributes() const { return (_atts); };
        void                               SetAttributes(std::map<string, Attribute> &atts) { _atts = atts; };

        bool GetAttribute(string name, Attribute &att) const
        {
            std::map<string, Attribute>::const_iterator itr = _atts.find(name);
            if (itr == _atts.end()) return (false);
            att = itr->second;
            return (true);
        }

        void SetAttribute(const Attribute &att) { _atts[att.GetName()] = att; }

        //! Return true if no wavelet is defined
        //
        bool IsCompressed() const { return (!_wname.empty()); };

        friend std::ostream &operator<<(std::ostream &o, const BaseVar &var);

    private:
        string                      _name;
        string                      _units;
        XType                       _type;
        string                      _wname;
        std::vector<size_t>         _cratios;
        std::vector<bool>           _periodic;
        std::map<string, Attribute> _atts;
    };

    //! \class CoordVar
    //! \brief Coordinate variable metadata
    //
    class CoordVar : public BaseVar {
    public:
        CoordVar()
        {
            _dim_names.clear();
            _time_dim_name.clear();
            _axis = 0;
            _uniform = true;
        }

        //! Construct coordinate variable
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type, bool compressed,
        //!  string wname,
        //!  std::vector <size_t> cratios,
        //!  std::vector <bool> periodic)
        //!
        //! \param[in] dim_names An ordered list of names of the spatial
        //! dimensions of
        //! the coordinate variable. The ordering is from fastest varying
        //! dimension to slowest.
        //! The number of elements in \p dim_names determines the dimensionality
        //! of the coordinate variable.
        //!
        //! \param[in] time_dim_name Name of time varying dimension, if any. If
        //! the coordinate variable varies over time this parameter names
        //! the time dimension. If \p time_dim_name is the empty string
        //! the coordiante variable is constant over time.
        //!
        //! \param[in] axis an int in the range 0..3 indicating the coordinate
        //! axis, one of X, Y, Z, or T, respectively
        //! \param[in] uniform A bool indicating whether the coordinate variable
        //! is uniformly sampled.
        //
        CoordVar(string name, string units, XType type, string wname, std::vector<size_t> cratios, std::vector<bool> periodic, std::vector<string> dim_names, string time_dim_name, int axis,
                 bool uniform)
        : BaseVar(name, units, type, wname, cratios, periodic), _dim_names(dim_names), _time_dim_name(time_dim_name), _axis(axis), _uniform(uniform)
        {
        }

        //! Construct coordinate variable without compression
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type, std::vector <bool> periodic)
        //!
        //! \param[in] dim_names An ordered list of names of the spatial
        //! dimensions of
        //! the coordinate variable. The ordering is from fastest varying
        //! dimension to slowest.
        //! The number of elements in \p dim_names determines the dimensionality
        //! of the coordinate variable.
        //!
        //! \param[in] time_dim_name Name of time varying dimension, if any. If
        //! the coordinate variable varies over time this parameter names
        //! the time dimension. If \p time_dim_name is the empty string
        //! the coordiante variable is constant over time.
        //!
        //! \param[in] axis an int in the range 0..3 indicating the coordinate
        //! axis, one of X, Y, Z, or T, respectively
        //! \param[in] uniform A bool indicating whether the coordinate variable
        //! is uniformly sampled.
        //
        CoordVar(string name, string units, XType type, std::vector<bool> periodic, int axis, bool uniform, std::vector<string> dim_names, string time_dim_name)
        : BaseVar(name, units, type, periodic), _dim_names(dim_names), _time_dim_name(time_dim_name), _axis(axis), _uniform(uniform)
        {
        }

        virtual ~CoordVar(){};

        //! Access coordinate variable spatial dimension names
        //! \version 3.1
        //
        std::vector<string> GetDimNames() const { return (_dim_names); };
        void                SetDimNames(std::vector<string> dim_names) { _dim_names = dim_names; };

        //! Access coordinate variable time dimension name
        //! \version 3.1
        //
        string GetTimeDimName() const { return (_time_dim_name); };
        void   SetTimeDimName(string time_dim_name) { _time_dim_name = time_dim_name; };

        //! Access coordinate variable axis
        //
        int  GetAxis() const { return (_axis); };
        void SetAxis(int axis) { _axis = axis; };

        //! Access coordinate variable uniform sampling flag
        //
        bool GetUniform() const { return (_uniform); };
        void SetUniform(bool uniform) { _uniform = uniform; };

        friend std::ostream &operator<<(std::ostream &o, const CoordVar &var);

    private:
        std::vector<string> _dim_names;
        string              _time_dim_name;
        int                 _axis;
        bool                _uniform;
    };

    //! \class DataVar
    //! \brief Data variable metadata
    //!
    //! This class defines metadata associatd with a Data variable
    //!
    class DataVar : public BaseVar {
    public:
        DataVar()
        {
            _mesh.clear();
            _time_coord_var.clear();
            _location = Mesh::NODE;
            _maskvar.clear();
            _has_missing = false;
            _missing_value = 0.0;
        }

        //! Construct Data variable definition with missing values
        //!
        //! Elements of the variable whose value matches that specified by
        //! \p missing_value are considered invalid
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  string wname,
        //!  std::vector <size_t> cratios,
        //!  std::vector <bool> periodic)
        //!
        //! \param[in] mesh Name of mesh upon which this variable is sampled
        //!
        //! \param[in] time_coord_var Name of time coordinate variable. If
        //! the variable is time varying this parameter provides the name of
        //! it's time coordinate variable. If the variable is invariant over time
        //! \p time_coord_var should be the empty string.
        //!
        //! \param[in] location Location of samples on Mesh. Samples can be
        //! located at the Mesh nodes, edge centers, face centers, or
        //! volume centers.
        //!
        //! \param[in] missing_value  Value of the missing value indicator
        //!
        DataVar(string name, string units, XType type, string wname, std::vector<size_t> cratios, std::vector<bool> periodic, string mesh, string time_coord_var, Mesh::Location location,
                double missing_value)
        : BaseVar(name, units, type, wname, cratios, periodic), _mesh(mesh), _time_coord_var(time_coord_var), _location(location), _maskvar(""), _has_missing(true), _missing_value(missing_value)
        {
        }

        //! Construct Data variable definition with a mask variable
        //!
        //! This version of the constructor specifies the name of a variable
        //! \p varmask whose contents indicate the presense or absense of invalid
        //! entries in the data variable. The contents of the mask array are treated
        //! as booleans, true values indicating valid data. The rank of of the
        //! variable may be less than or equal to that of \p name. The dimensions
        //! of \p maskvar must match the fastest varying dimensions of \p name.
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  string wname,
        //!  std::vector <size_t> cratios,
        //!  std::vector <bool> periodic)
        //!
        //! \param[in] mesh Name of mesh upon which this variable is sampled
        //!
        //! \param[in] time_coord_var Name of time coordinate variable. If
        //! the variable is time varying this parameter provides the name of
        //! it's time coordinate variable. If the variable is invariant over time
        //! \p time_coord_var should be the empty string.
        //!
        //! \param[in] location Location of samples on Mesh. Samples can be
        //! located at the Mesh nodes, edge centers, face centers, or
        //! volume centers.
        //!
        //! \param[in] missing_value  Value used to fill masked values
        //! \param[in] maskvar  Name of variable containing mask array.
        //!
        DataVar(string name, string units, XType type, string wname, std::vector<size_t> cratios, std::vector<bool> periodic, string mesh, string time_coord_var, Mesh::Location location,
                double missing_value, string maskvar)
        : BaseVar(name, units, type, wname, cratios, periodic), _mesh(mesh), _time_coord_var(time_coord_var), _location(location), _maskvar(maskvar), _has_missing(true), _missing_value(missing_value)
        {
        }

        //! Construct Data variable definition without missing values
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  string wname,
        //!  std::vector <size_t> cratios,
        //!  vector <bool> periodic)
        //!
        //! \deprecated \param[in] coord_vars Names of coordinate variables
        //! associated
        //! with this variables dimensions
        //!
        //! \param[in] mesh Name of mesh upon which this variable is sampled
        //!
        //! \param[in] time_coord_var Name of time coordinate variable. If
        //! the variable is time varying this parameter provides the name of
        //! it's time coordinate variable. If the variable is invariant over time
        //! \p time_coord_var should be the empty string.
        //!
        //! \param[in] location Location of samples on Mesh. Samples can be
        //! located at the Mesh nodes, edge centers, face centers, or
        //! volume centers.
        //!
        DataVar(string name, string units, XType type, string wname, std::vector<size_t> cratios, std::vector<bool> periodic, string mesh, string time_coord_var, Mesh::Location location)
        : BaseVar(name, units, type, wname, cratios, periodic), _mesh(mesh), _time_coord_var(time_coord_var), _location(location), _maskvar(""), _has_missing(false), _missing_value(0.0)
        {
        }

        //! Construct Data variable definition with missing values but no compression
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  std::vector <bool> periodic)
        //!
        //! \deprecated \param[in] coord_vars Names of coordinate variables
        //! associated
        //! with this variables dimensions
        //!
        //! \param[in] mesh Name of mesh upon which this variable is sampled
        //!
        //! \param[in] time_coord_var Name of time coordinate variable. If
        //! the variable is time varying this parameter provides the name of
        //! it's time coordinate variable. If the variable is invariant over time
        //! \p time_coord_var should be the empty string.
        //!
        //! \param[in] location Location of samples on Mesh. Samples can be
        //! located at the Mesh nodes, edge centers, face centers, or
        //! volume centers.
        //!
        //! \param[in] missing_value  Value of the missing value indicator
        //!
        DataVar(string name, string units, XType type, std::vector<bool> periodic, string mesh, string time_coord_var, Mesh::Location location, double missing_value)
        : BaseVar(name, units, type, periodic), _mesh(mesh), _time_coord_var(time_coord_var), _location(location), _maskvar(""), _has_missing(true), _missing_value(missing_value)
        {
        }

        //! Construct Data variable definition with a mask but no compression
        //!
        //! This version of the constructor specifies the name of a variable
        //! \p varmask whose contents indicate the presense or absense of invalid
        //! entries in the data variable. The contents of the mask array are treated
        //! as booleans, true values indicating valid data. The rank of of the
        //! variable may be less than or equal to that of \p name. The dimensions
        //! of \p maskvar must match the fastest varying dimensions of \p name.
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  std::vector <bool> periodic)
        //!
        //! \deprecated \param[in] coord_vars Names of coordinate
        //! variables associated
        //!
        //! \param[in] mesh Name of mesh upon which this variable is sampled
        //!
        //! \param[in] time_coord_var Name of time coordinate variable. If
        //! the variable is time varying this parameter provides the name of
        //! it's time coordinate variable. If the variable is invariant over time
        //! \p time_coord_var should be the empty string.
        //!
        //! \param[in] location Location of samples on Mesh. Samples can be
        //! located at the Mesh nodes, edge centers, face centers, or
        //! volume centers.
        //!
        //! with this variables dimensions
        //! \param[in] missing_value  Value used to fill masked values
        //! \param[in] maskvar  Name of variable containing mask array.
        //!
        DataVar(string name, string units, XType type, std::vector<bool> periodic, string mesh, string time_coord_var, Mesh::Location location, double missing_value, string maskvar)
        : BaseVar(name, units, type, periodic), _mesh(mesh), _time_coord_var(time_coord_var), _location(location), _maskvar(maskvar), _has_missing(true), _missing_value(missing_value)
        {
        }

        //! Construct Data variable definition with no missing values or compression
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  std::vector <bool> periodic)
        //!
        //! \param[in] coord_vars Names of coordinate variables associated
        //! with this variables dimensions
        //!
        //! \param[in] time_coord_var Name of time coordinate variable. If
        //! the variable is time varying this parameter provides the name of
        //! it's time coordinate variable. If the variable is invariant over time
        //! \p time_coord_var should be the empty string.
        //!
        //!
        DataVar(string name, string units, XType type, std::vector<bool> periodic, string mesh, string time_coord_var, Mesh::Location location)
        : BaseVar(name, units, type, periodic), _mesh(mesh), _time_coord_var(time_coord_var), _location(location), _maskvar(""), _has_missing(false), _missing_value(0.0)
        {
        }

        virtual ~DataVar(){};

        //! Access variable's mesh name
        //! \version 3.1
        //
        string GetMeshName() const { return (_mesh); };
        void   SetMeshName(string mesh) { _mesh = mesh; };

        //! Access variable's time coordinate variable name
        //! \version 3.1
        //
        string GetTimeCoordVar() const { return (_time_coord_var); }
        void   SetTimeCoordVar(string time_coord_var) { _time_coord_var = time_coord_var; }

        //! Access variable's sampling location on mesh
        //! \version 3.1
        //
        Mesh::Location GetSamplingLocation() const { return (_location); }

        //! Access data variable's mask variable names
        //
        string GetMaskvar() const { return (_maskvar); };
        void   SetMaskvar(string maskvar) { _maskvar = maskvar; };

        //! Access data variable's missing data flag
        //
        bool GetHasMissing() const { return (_has_missing); };
        void SetHasMissing(bool has_missing) { _has_missing = has_missing; };

        //! Access data variable's missing data value
        //
        double GetMissingValue() const { return (_missing_value); };
        void   SetMissingValue(double missing_value) { _missing_value = missing_value; };

        VDF_API friend std::ostream &operator<<(std::ostream &o, const DataVar &var);

    private:
        string         _mesh;
        string         _time_coord_var;
        Mesh::Location _location;
        string         _maskvar;
        bool           _has_missing;
        double         _missing_value;
    };

    //! \class AuxVar
    //! \version 3.1
    //!
    //! \brief Auxiliary variable metadata
    //!
    //! This class defines metadata associatd with an Auxiliary variable.
    //! An Auxiliary variable is neither a data variable, nor a coordinate
    //! variable.
    //!
    class AuxVar : public BaseVar {
    public:
        AuxVar()
        {
            _dim_names.clear();
            _offset = 0;
        }

        //! Construct Auxiliary variable definition
        //!
        //! \copydetails BaseVar(string name,
        //!  string units, XType type,
        //!  string wname,
        //!  std::vector <size_t> cratios,
        //!  std::vector <bool> periodic)
        //!
        //! \param[in] dim_names An ordered list of names of the dimensions of
        //! the coordinate variable. The ordering is from fastest varying
        //! dimension to slowest.
        //! The number of elements in \p dim_names determines the dimensionality
        //! of the auxiliary variable.
        //!
        AuxVar(string name, string units, XType type, string wname, std::vector<size_t> cratios, std::vector<bool> periodic, std::vector<string> dim_names)
        : BaseVar(name, units, type, wname, cratios, periodic), _dim_names(dim_names), _offset(0)
        {
        }

        virtual ~AuxVar(){};

        //! Access Auxiliary variable dimension names
        //
        std::vector<string> GetDimNames() const { return (_dim_names); };
        void                SetDimNames(std::vector<string> dim_names) { _dim_names = dim_names; };

        //! Access Auxiliary variable's offset
        //!
        //! The value of \p offset should be added to the Auxiliary variable's data
        //
        long GetOffset() const { return (_offset); };
        void SetOffset(long offset) { _offset = offset; };

        VDF_API friend std::ostream &operator<<(std::ostream &o, const AuxVar &var);

    private:
        std::vector<string> _dim_names;
        long                _offset;
    };

    //! Class constuctor
    //!
    //!
    DC();

    virtual ~DC(){};

    //! Initialize the DC class
    //!
    //! Prepare a DC for reading. This method prepares
    //! the master DC file indicated by \p path for reading.
    //! The method should be called immediately after the constructor,
    //! before any other class methods. This method
    //! exists only because C++ constructors can not return error codes.
    //!
    //! \param[in] path Path name of file that contains, or will
    //! contain, the DC master file for this data collection
    //!
    //! \param[in] options A vector of option pairs (name, value) that
    //! may be accepted by the derived class.
    //!
    //! \retval status A negative int is returned on failure
    //!
    //
    virtual int Initialize(const std::vector<string> &paths, const std::vector<string> &options = std::vector<string>()) { return (initialize(paths, options)); }

    //! Return a dimensions's definition
    //!
    //! This method returns the definition of the dimension named
    //! by \p dimname as a reference to a DC::Dimension object. If
    //! \p dimname is not defined as a dimension then the name of \p dimension
    //! will be the empty string()
    //!
    //! \param[in] dimname A string specifying the name of the dimension.
    //! \param[out] dimension The returned Dimension object reference
    //! \retval bool If the named dimension can not be found false is returned.
    //!
    virtual bool GetDimension(string dimname, DC::Dimension &dimension) const { return (getDimension(dimname, dimension)); }

    //! Return names of all defined dimensions
    //!
    //! This method returns the list of names of all of the dimensions
    //! defined in the DC.
    //!
    virtual std::vector<string> GetDimensionNames() const { return (getDimensionNames()); }

    //! Return names of all defined meshes
    //!
    //! This method returns the list of names of all of the meshes
    //! defined in the DC.
    //!
    virtual std::vector<string> GetMeshNames() const { return (getMeshNames()); }

    //! Return a Mesh's definition
    //!
    //! This method returns the definition of the mesh named
    //! by \p mesh_name as a reference to a DC::Mesh object.
    //!
    //! \param[in] mesh_name A string specifying the name of the Mesh.
    //! \param[out] mesh The returned Mesh object reference
    //! \retval bool If the named mesh can not be found false is returned.
    //!
    virtual bool GetMesh(string mesh_name, DC::Mesh &mesh) const { return (getMesh(mesh_name, mesh)); }

    //! Return the ordered list of dimensions for a mesh
    //!
    //! This method is a convenience function that returns the ordered
    //! vector of dimension lengths for the mesh named \p mesh. If \p mesh
    //! is unknown, or invalid false is returned.
    //!
    //! \sa DC::GetMesh(), DC::GetDimension()
    //
    virtual bool GetMeshDimLens(const string &mesh_name, std::vector<size_t> &dims) const;

    //! Return the ordered list of dimension names for a mesh
    //!
    //! This method is a convenience function that returns the ordered
    //! vector of dimension names for the mesh named \p mesh. If \p mesh
    //! is unknown, or invalid false is returned.
    //!
    //! \sa DC::GetMesh(), DC::GetDimension()
    //
    virtual bool GetMeshDimNames(const string &mesh_name, std::vector<string> &dimnames) const;

    //! Return a coordinate variable's definition
    //!
    //! Return a reference to a DC::CoordVar object describing
    //! the coordinate variable named by \p varname
    //!
    //! \param[in] varname A string specifying the name of the coordinate
    //! variable.
    //! \param[out] coordvar A CoordVar object containing the definition
    //! of the named variable.
    //! \retval bool False is returned if the named coordinate variable does
    //! not exist, and the contents of \p cvar will be undefined.
    //!
    virtual bool GetCoordVarInfo(string varname, DC::CoordVar &cvar) const { return (getCoordVarInfo(varname, cvar)); }

    //! Return a data variable's definition
    //!
    //! Return a reference to a DC::DataVar object describing
    //! the data variable named by \p varname
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \param[out] datavar A DataVar object containing the definition
    //! of the named Data variable.
    //!
    //! \retval bool If the named data variable cannot be found false
    //! is returned and the values of \p datavar are undefined.
    //!
    virtual bool GetDataVarInfo(string varname, DC::DataVar &datavar) const { return (getDataVarInfo(varname, datavar)); }

    //! Return metadata about an auxiliary variable
    //!
    //! If the variable \p varname is defined as an auxiliary
    //! variable its metadata will
    //! be returned in \p var.
    //!
    //! \retval bool If the named variable cannot be found false
    //! is returned and the values of \p var are undefined.
    //!
    //! \sa GetDataVarInfo(), GetCoordVarInfo()
    //
    virtual bool GetAuxVarInfo(string varname, DC::AuxVar &var) const { return (getAuxVarInfo(varname, var)); }

    //! Return metadata about a data or coordinate variable
    //!
    //! If the variable \p varname is defined as either a
    //! data or coordinate variable its metadata will
    //! be returned in \p var.
    //!
    //! \retval bool If the named variable cannot be found false
    //! is returned and the values of \p var are undefined.
    //!
    //! \sa GetDataVarInfo(), GetCoordVarInfo()
    //
    virtual bool GetBaseVarInfo(string varname, DC::BaseVar &var) const { return (getBaseVarInfo(varname, var)); }

    //! Return a list of names for all of the defined data variables.
    //!
    //! Returns a list of names for all data variables defined
    //!
    //! \sa DC::DataVar
    //
    virtual std::vector<string> GetDataVarNames() const { return (getDataVarNames()); }

    //! Return a list of names for all of the defined coordinate variables.
    //!
    //! Returns a list of names for all coordinate variables defined
    //!
    //! \sa DC::CoordVar
    //
    virtual std::vector<string> GetCoordVarNames() const { return (getCoordVarNames()); }

    //! Return a list of names for all of the defined Auxiliary variables.
    //!
    //! Returns a list of names for all Auxiliary variables defined
    //!
    //! \sa DC::AuxVar
    //
    virtual std::vector<string> GetAuxVarNames() const { return (getAuxVarNames()); }

    //! Return the number of refinement levels for the indicated variable
    //!
    //! Compressed variables may have a multi-resolution grid representation.
    //! This method returns the number of levels in the hiearchy. A value
    //! of one indicates that only the native resolution is available.
    //! A value of two indicates that two levels, the native plus the
    //! next coarsest are available, and so on.
    //!
    //! \param[in] varname Data or coordinate variable name.
    //!
    //! \retval num If \p varname is unknown one is returned. if \p varname
    //! is not compressed (has no multi-resolution representation) one is
    //! returned. Otherwise the total number of levels in the multi-resolution
    //! hierarchy are returned.
    //
    virtual size_t GetNumRefLevels(string varname) const { return (getNumRefLevels(varname)); }

    //! Read an attribute
    //!
    //! This method reads an attribute from the DC. The attribute can either
    //! be "global", if \p varname is the empty string, or bound to a variable
    //! if \p varname indentifies a variable in the DC.
    //!
    //! \param[in] varname The name of the variable the attribute is bound to,
    //! or the empty string if the attribute is global
    //! \param[in] attname The attributes name
    //! \param[out] type The primitive data type storage format.
    //! This is the type that will be used to store the
    //! attribute on disk
    //! \param[out] values A vector to contain the returned floating point
    //! attribute values
    //!
    //! \retval status True is returned on success. False is returned if either
    //! the variable or the attribute is undefined.
    //!
    //
    virtual bool GetAtt(string varname, string attname, vector<double> &values) const { return (getAtt(varname, attname, values)); }

    virtual bool GetAtt(string varname, string attname, vector<long> &values) const { return (getAtt(varname, attname, values)); }

    virtual bool GetAtt(string varname, string attname, string &values) const { return (getAtt(varname, attname, values)); }

    //! Return a list of available attribute's names
    //!
    //! Returns a vector of all attribute names for the
    //! variable, \p varname. If \p varname is the empty string the names
    //! of all of the global attributes are returned. If \p varname is
    //! not defined an empty vector is returned.
    //!
    //! \param[in] varname The name of the variable to query,
    //! or the empty string if the names of global attributes are desired.
    //! \retval attnames A vector of returned attribute names
    //!
    //! \sa GetAtt()
    //
    virtual std::vector<string> GetAttNames(string varname) const { return (getAttNames(varname)); }

    //! Return the external data type for an attribute
    //!
    //! Returns the external storage type of the named variable attribute.
    //!
    //! \param[in] varname The name of the variable to query,
    //! or the empty string if the names of global attributes are desired.
    //! \param[in] name Name of the attribute.
    //!
    //! \retval If an attribute named by \p name does not exist, a
    //! negative value is returned.
    //!
    virtual XType GetAttType(string varname, string attname) const { return (getAttType(varname, attname)); }

    //! Return a variable's array dimension lengths at a specified refinement level
    //!
    //! Compressed variables may have a multi-resolution grid representation.
    //! This method returns the variable's ordered array
    //! dimension lengths,
    //! and block dimensions
    //! at the multiresolution refinement level specified by \p level.
    //!
    //! If the variable named by \p varname is not compressed the variable's
    //! native dimensions are returned.
    //!
    //! \note The number of elements in \p dims_at_level will match that of
    //! \p bs_at_level.  If the data are not blocked the value of each
    //! element of \p bs_at_level will be 1.
    //!
    //! \param[in] varname Data or coordinate variable name.
    //! \param[in] level Specifies a member of a multi-resolution variable's
    //! grid hierarchy as described above.
    //! \param[out] dims_at_level An ordered vector containing the variable's
    //! dimensions at the specified refinement level
    //! \param[out] bs_at_level An ordered vector containing the variable's
    //! block dimensions at the specified refinement level
    //!
    //! \retval status Zero is returned upon success, otherwise -1.
    //!
    //! \note For unstructured grids the number of dimensions may be
    //! less than the topological dimension returned by DC::Mesh::GetTopologyDim().
    //!
    //! \sa VAPoR::DC, DC::DataVar::GetBS(), DC::GetVarDimLens()
    //! \sa ReadRegionBlock()
    //
    virtual int GetDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const
    {
        return (getDimLensAtLevel(varname, level, dims_at_level, bs_at_level));
    }

    //! Return a variable's array dimension lengths
    //!
    //! This method is equivalent to calling GetDimLensAtLevel() with \p level
    //! equal to -1
    //!
    virtual int GetDimLens(string varname, std::vector<size_t> &dims)
    {
        vector<size_t> dummy;
        return (GetDimLensAtLevel(varname, -1, dims, dummy));
    }

    //! Return default Proj4 map projection string.
    //!
    //! For georeference data sets that have map projections this
    //! method returns the default properly formatted Proj4 projection string
    //! for mapping from geographic to cartographic coordinates.
    //! If no
    //! projection exists, an empty string is returned.
    //!
    //! \retval  projstring An empty string if a Proj4 map projection is
    //! not available for the named variable, otherwise a properly
    //! formatted Proj4 projection
    //! string is returned.
    //!
    //
    virtual string GetMapProjection() const { return (getMapProjection()); }

    //! Open the named variable for reading
    //!
    //! This method prepares a data or coordinate variable, indicated by a
    //! variable name and time step pair, for subsequent read operations by
    //! methods of this class.  The value of the refinement levels
    //! parameter, \p level, indicates the resolution of the volume in
    //! the multiresolution hierarchy as described by GetDimLensAtLevel().
    //!
    //! The level-of-detail parameter, \p lod, selects
    //! the approximation level. Valid values for \p lod are integers in
    //! the range 0..n-1, where \e n is returned by
    //! DC::BaseVar::GetCRatios().size(), or the value -1 may be used
    //! to select the best approximation available.
    //!
    //! An error occurs, indicated by a negative return value, if the
    //! volume identified by the {varname, timestep, level, lod} tupple
    //! is not available. Note the availability of a volume can be tested
    //! with the VariableExists() method.
    //!
    //! \param[in] ts Time step of the variable to read. This is the integer
    //! offset into the variable's temporal dimension. If the variable
    //! does not have a temporal dimension \p ts is ignored.
    //! \param[in] varname Name of the variable to read
    //! \param[in] level Refinement level of the variable. Ignored if the
    //! variable is not compressed.
    //! \param[in] lod Approximation level of the variable. A value of -1
    //! indicates the maximum approximation level defined for the DC.
    //! Ignored if the variable is not compressed.
    //!
    //! \retval status Returns a non-negative file descriptor on success
    //!
    //!
    //! \sa GetNumRefLevels(), DC::BaseVar::GetCRatios(), OpenVariableRead()
    //
    virtual int OpenVariableRead(size_t ts, string varname, int level = 0, int lod = 0) { return (_openVariableRead(ts, varname, level, lod)); }

    //! Close the currently opened variable
    //!
    //! Close the handle for variable opened with OpenVariableRead()
    //! \param[in] fd A valid file descriptor returned by OpenVariableRead()
    //!
    //! \sa  OpenVariableRead()
    //
    virtual int CloseVariable(int fd) { return (_closeVariable(fd)); }

    //! Read all spatial values of the currently opened variable
    //!
    //! This method reads, and decompresses as necessary,
    //!  the contents of the currently opened variable into the array
    //! \p data. The number of values
    //! read into \p data is given by the product of the spatial
    //! dimensions of the open variable at the refinement level specified.
    //!
    //! It is the caller's responsibility to ensure \p data points
    //! to adequate space.
    //!
    //! \param[in] fd A valid file descriptor returned by OpenVariableRead()
    //! \param[out] data An array of data to be written
    //! \retval status Returns a non-negative value on success
    //!
    //! \sa OpenVariableRead()
    //
    int virtual Read(int fd, float *data) { return (_readTemplate(fd, data)); }

    int virtual Read(int fd, int *data) { return (_readTemplate(fd, data)); }

    //! Read a single slice of data from the currently opened variable
    //!
    //! Decompress, as necessary, and read a single hyperslice of
    //! data from the variable
    //! indicated by the most recent call to OpenVariableRead().
    //! The dimensions and number of slices are given by
    //! GetHyperSliceInfo().
    //!
    //! This method should be called exactly NZ times for each opened variable,
    //! where NZ is the dimension of slowest varying dimension returned by
    //! GetDimLensAtLevel().
    //!
    //! It is the caller's responsibility to ensure \p slice points
    //! to adequate space.
    //!
    //! \param[in] fd A valid file descriptor returned by OpenVariableRead()
    //! \param[out] slice A slice of data
    //! \retval status Returns a non-negative value on success
    //!
    //! \sa OpenVariableRead(), GetHyperSliceInfo()
    //!
    virtual int ReadSlice(int fd, float *slice) { return (_readSliceTemplate(fd, slice)); }
    virtual int ReadSlice(int fd, int *slice) { return (_readSliceTemplate(fd, slice)); }

    //! Read in and return a subregion from the currently opened
    //! variable
    //!
    //! This method reads and returns a subset of variable data.
    //! The \p min and \p max vectors, whose dimensions must match the
    //! spatial rank of the currently opened variable, identify the minimum and
    //! maximum extents, in grid coordinates, of the subregion of interest. The
    //! minimum and maximum valid values of an element of \b min or \b max
    //! are \b 0 and
    //! \b n-1, respectively, where \b n is the length of the associated
    //! dimension at the opened refinement level.
    //!
    //! The region
    //! returned is stored in the memory region pointed to by \p region. It
    //! is the caller's responsbility to ensure adequate space is available.
    //!
    //! \param[in] fd A valid file descriptor returned by OpenVariableRead()
    //! \param[in] min Minimum region extents in grid coordinates
    //! \param[in] max Maximum region extents in grid coordinates
    //! \param[out] region The requested volume subregion
    //!
    //! \retval status Returns a non-negative value on success
    //! \sa OpenVariableRead(), GetDimLensAtLevel(), GetDimensionNames()
    //
    virtual int ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (readRegion(fd, min, max, region)); }
    virtual int ReadRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (readRegion(fd, min, max, region)); }

    //! Read in and return a blocked subregion from the currently opened
    //! variable.
    //!
    //! This method is identical to ReadRegion() with the exceptions
    //! that:
    //!
    //! \li The vectors \p start and \p count must be aligned
    //! with the underlying storage block of the variable. See
    //! DC::GetDimLensAtLevel()
    //!
    //! For data that are not blocked (i.e. the elements of the
    //! \p bs_at_level parameter returned by GetDimsAtLevel() are all 1)
    //! this method is identical to ReadRegion()
    //!
    //! \li The hyperslab copied to \p region will preserve its underlying
    //! storage blocking (the data will not be contiguous, unless the
    //! data are not blocked)
    //!
    virtual int ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) { return (readRegionBlock(fd, min, max, region)); }
    virtual int ReadRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (readRegionBlock(fd, min, max, region)); }

    //! Read an entire variable in one call
    //!
    //! This method reads and entire variable (all time steps, all grid points)
    //! from a DC.  This is the simplest interface for reading data from
    //! a DC. If the variable is split across multiple files GetVar()
    //! ensures that the data are correctly gathered and assembled into memory
    //! Any variables currently opened with OpenVariableRead() are first closed.
    //! Thus variables need not be opened with OpenVariableRead() prior to
    //! calling GetVar();
    //!
    //! It is an error to call this method in \b define mode
    //!
    //! \param[in] varname Name of the variable to write
    //! \param[in] level Refinement level of the variable.
    //! Ignored if the variable is not compressed.
    //! \param[in] lod Approximation level of the variable. A value of -1
    //! indicates the maximum approximation level defined for the DC.
    //! Ignored if the variable is not compressed.
    //! \param[out] data Pointer to where data will be copied. It is the
    //! caller's responsbility to ensure \p data points to sufficient memory.
    //!
    //! \retval status A negative int is returned on failure
    //!
    //
    virtual int GetVar(string varname, int level, int lod, float *data) { return (_getVarTemplate(varname, level, lod, data)); }
    virtual int GetVar(string varname, int level, int lod, int *data) { return (_getVarTemplate(varname, level, lod, data)); }

    //! Read an entire variable at a given time step in one call
    //!
    //! This method reads and entire variable (all grid points) at
    //! time step \p ts
    //! from a DC.  This is the simplest interface for reading data from
    //! a DC.
    //! Any variables currently opened with OpenVariableRead() are first closed.
    //! Thus variables need not be opened with OpenVariableRead() prior to
    //! calling GetVar();
    //!
    //! It is an error to call this method in \b define mode
    //!
    //! \param[in] ts Time step of the variable to write. This is the integer
    //! offset into the variable's temporal dimension. If the variable
    //! does not have a temporal dimension \p ts is ignored.
    //! \param[in] varname Name of the variable to write
    //! \param[in] level Refinement level of the variable.
    //! Ignored if the variable is not compressed.
    //! \param[in] lod Approximation level of the variable. A value of -1
    //! indicates the maximum approximation level defined for the DC.
    //! Ignored if the variable is not compressed.
    //! \param[out] data Pointer to where data will be copied. It is the
    //! caller's responsbility to ensure \p data points to sufficient memory.
    //!
    //! \retval status A negative int is returned on failure
    //!
    //
    virtual int GetVar(size_t ts, string varname, int level, int lod, float *data) { return (_getVarTemplate(ts, varname, level, lod, data)); }
    virtual int GetVar(size_t ts, string varname, int level, int lod, int *data) { return (_getVarTemplate(ts, varname, level, lod, data)); }

    //! Returns true if indicated data volume is available
    //!
    //! Returns true if the variable identified by the timestep, variable
    //! name, refinement level, and level-of-detail is present in
    //! the data set. Returns false if
    //! the variable is not available.
    //!
    //! \param[in] ts A valid time step between 0 and GetNumTimesteps()-1
    //! \param[in] varname A valid variable name
    //! \param[in] reflevel Refinement level requested.
    //! \param[in] lod Compression level of detail requested.
    //! refinement level contained in the DC.
    //
    virtual bool VariableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const { return (variableExists(ts, varname, reflevel, lod)); };

    //! Get dimensions of hyperslice read by ReadSlice
    //!
    //! Returns the dimensions of a hyperslice when the variable
    //! \p varname is opened at level \p level and read using ReadSlice();
    //!
    //! \param[in] varname A valid variable name
    //! \param[in] reflevel Refinement level requested.
    //! \param[out] dims An ordered vector containing the variable's
    //! hyperslice dimensions at the specified refinement level
    //! \param[out] nslice Number of hyperslices
    //!
    //! \sa GetDimLensAtLevel(), OpenVariableRead(), ReadSlice()
    //!
    virtual int GetHyperSliceInfo(string varname, int level, std::vector<size_t> &dims, size_t &nslice);

    //! Return a list of data variables with a given topological dimension
    //!
    //! Returns a list of all data variables defined having a
    //! a topological dimension \p ndim.
    //!
    //! \param[in] ndim Topological dimension
    //!
    //! \sa GetVarTopologyDim()
    //
    virtual std::vector<string> GetDataVarNames(int ndim) const;

    //
    //! Return an ordered list of the variables dimensions
    //!
    //! Returns a list of a variables dimensions ordered from fastest
    //! to slowest. If \p time is true and the variable is time varying
    //! the time dimension will be included. The time dimension is always
    //! the slowest varying dimension.
    //!
    //! \param[in] varname A valid variable name
    //! \param[in] spatial If true only return spatial dimensions
    //!
    //! \param[out] dimensions Ordered list of variable dimensions
    //! on success.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarDimensions(string varname, bool spatial, vector<DC::Dimension> &dimensions) const;

    //
    //! Return an ordered list of the variables dimension lengths
    //!
    //! Returns a list of a variables dimension lengths ordered from fastest
    //! to slowest. If \p spatial is true and the variable is time varying
    //! the time dimension will be included. The time dimension is always
    //! the slowest varying dimension.
    //!
    //! \param[in] varname A valid variable name
    //! \param[in] spatial If true only return spatial dimensions
    //!
    //! \param[out] dimensions Ordered list of variable dimension lengths
    //! on success.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarDimLens(string varname, bool spatial, vector<size_t> &dimlens) const;

    //! Return an ordered list of the variables dimension lengths
    //!
    //! Returns a list of a variable's spatial dimension lengths ordered from
    //! fastest to slowest, and, if the variable is time varying, the variable's
    //! time dimension lengths is returned as well.
    //!
    //! \param[in] varname A valid variable name
    //! \param[out] sdimlens Ordered list of variable spatial dimension lengths
    //! on success.
    //! \param[out] time_dimlen The variable's time dimension length. If
    //! the variable is not time varying \p time_dim_length will be set to 0.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarDimLens(string varname, vector<size_t> &sdimlens, size_t &time_dimlen) const;

    //! Return an ordered list of the variables dimension names
    //!
    //! Returns a list of a variables dimension names ordered from fastest
    //! to slowest. If \p spatial is true and the variable is time varying
    //! the time dimension names will be included. The time dimension is always
    //! the slowest varying dimension.
    //!
    //! \param[in] varname A valid variable name
    //! \param[in] spatial If true only return spatial dimensions
    //!
    //! \param[out] dimensions Ordered list of variable dimension names
    //! on success.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarDimNames(string varname, bool spatial, vector<string> &dimnames) const;

    //! Return an ordered list of the variables dimension names
    //!
    //! Returns a list of a variable's spatial dimension names ordered from
    //! fastest to slowest, and, if the variable is time varying, the variable's
    //! time dimension name is returned as well.
    //!
    //! \param[in] varname A valid variable name
    //! \param[out] sdimnames Ordered list of variable spatial dimension names
    //! on success.
    //! \param[out] time_dimname The variable's time dimension name. If
    //! the variable is not time varying \p time_dim_name will be set to the
    //! emptry string.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarDimNames(string varname, vector<string> &sdimnames, string &time_dimname) const;

    //! Return the topological dimension of a variable
    //!
    //! Return the topological dimension of the mesh the defines
    //! the variable data \p varname
    //!
    //! \retval dim Topological dimension or zero if variable is not known
    //!
    //! \sa DC::Mesh::GetTopologyDim()
    //
    virtual size_t GetVarTopologyDim(string varname) const;

    //! Return the geometric dimension of a variable
    //!
    //! Return the geometric dimension of the mesh the defines
    //! the variable data \p varname. I.e. return the number of spatial coordinate
    //! variables associated with each node in the mesh.
    //!
    //! \retval dim Geometric dimension or zero if variable is not known
    //!
    //! \sa DC::Mesh::GetGeometryDim()
    //
    virtual size_t GetVarGeometryDim(string varname) const;

    //! Return a boolean indicating whether a variable is time varying
    //!
    //! This method returns \b true if the variable named by \p varname is
    //! a DataVar and it has a time coordiante
    //! (See DataVar::GetTimeCoord()), or if the variable is a CoordVar and
    //! its axis is time. Otherwise false is returnd.
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \retval bool Returns true if variable \p varname exists and is
    //! time varying.
    //!
    virtual bool IsTimeVarying(string varname) const;

    //! Return a boolean indicating whether a variable is compressed
    //!
    //! This method returns \b true if the variable named by \p varname is defined
    //! and it has a compressed representation. If either of these conditions
    //! is not true the method returns false.
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \retval bool Returns true if variable \p varname exists and is
    //! compressed
    //!
    //
    virtual bool IsCompressed(string varname) const;

    //! Return the time dimension length for a variable
    //!
    //! Returns the number of time steps (length of the time dimension)
    //! for which a variable is defined. If \p varname does not have a
    //! time coordinate 1 is returned. If \p varname is not defined
    //! as a variable zero is returned;
    //!
    //! \param[in] varname A string specifying the name of the variable.
    //! \retval count The length of the time dimension, or a negative
    //! int if \p varname is undefined.
    //!
    //! \retval n Returns the number of time steps for which the variable is
    //! defined, or zero if the variable is not defined.
    //!
    //! \sa IsTimeVarying()
    //!
    //
    virtual int GetNumTimeSteps(string varname) const;

    //! Return the compression ratio vector for the indicated variable
    //!
    //! Return the compression ratio vector for the indicated variable.
    //! The vector returned contains an ordered list of available
    //! compression ratios for the variable named by \p variable.
    //! If the variable is not compressed, or the variable named
    //! \p varname does not exist, the \p cratios parameter will
    //! contain a single element, one.
    //!
    //! \retval cratios Ordered vector of compression ratios
    //!
    //
    virtual std::vector<size_t> GetCRatios(string varname) const;

    //! Return a boolean indicating whether a variable is a data variable
    //!
    //! This method returns \b true if a data variable is defined
    //! with the name \p varname.  Otherwise the method returns false.
    //!
    //! \retval bool Returns true if \p varname names a defined data variable
    //!
    virtual bool IsDataVar(string varname) const
    {
        vector<string> names = GetDataVarNames();
        return (find(names.begin(), names.end(), varname) != names.end());
    }

    //! Return a boolean indicating whether a variable is a coordinate variable
    //!
    //! This method returns \b true if a coordinate variable is defined
    //! with the name \p varname.  Otherwise the method returns false.
    //!
    //! \retval bool Returns true if \p varname names a defined coordinate
    //! variable
    //!
    virtual bool IsCoordVar(string varname) const
    {
        vector<string> names = GetCoordVarNames();
        return (find(names.begin(), names.end(), varname) != names.end());
    }

    //! Return a boolean indicating whether a variable is an Auxiliary variable
    //! \version 3.1
    //!
    //! This method returns \b true if a Auxiliary variable is defined
    //! with the name \p varname.  Otherwise the method returns false.
    //!
    //! \retval bool Returns true if \p varname names a defined Auxiliary
    //! variable
    //!
    virtual bool IsAuxVar(string varname) const
    {
        vector<string> names = GetAuxVarNames();
        return (find(names.begin(), names.end(), varname) != names.end());
    }

    //! Return an ordered list of a data variable's coordinate names
    //!
    //! Returns a list of a coordinate variable names for the variable
    //! \p varname, ordered from fastest
    //! to slowest. If \p spatial is true and the variable is time varying
    //! the time coordinate variable name will be included. The time
    //! coordinate variable is always
    //! the slowest varying coordinate axis
    //!
    //! \param[in] varname A valid variable name
    //! \param[in] spatial If true only return spatial dimensions
    //!
    //! \param[out] coordvars Ordered list of coordinate variable names.
    //!
    //! \retval Returns true upon success, false if the variable is
    //! not defined.
    //!
    //
    virtual bool GetVarCoordVars(string varname, bool spatial, std::vector<string> &coord_vars) const;

    //! Get mesh connectivity variables for a data variable
    //!
    //! Return the mesh connectivity variables for a data variable. For
    //! a structured grid all connectivity variables will be empty. For
    //! an unstructured mesh only the \p face_node_var, \p node_face_var
    //! are guaranteed to be set to valid variable names
    //!
    bool GetVarConnVars(string varname, string &face_node_var, string &node_face_var, string &face_edge_var, string &face_face_var, string &edge_node_var, string &edge_face_var) const;

    //! Get the rank of a variable
    //!
    //! This method returns the number of rank of the array describing a
    //! variable. For structured data variables the rank is equal to
    //! the topological dimension (See GetTopologyDim()).
    //!
    //! \param[in] varname Name of variable to query
    //!
    //! \retval Array rank. A value between 0 and 3, inclusive. If
    //! \p varname is unknown 0 is returned.
    //!
    virtual size_t GetNumDimensions(string varname) const;

    //! Return a list of all of the available time coordinate variables
    //!
    //! This method returns all time coordinate variables defined
    //!
    std::vector<string> GetTimeCoordVarNames() const;

    class VDF_API FileTable {
    public:
        FileTable();
        virtual ~FileTable();

        class FileObject {
        public:
            FileObject() : _ts(0), _varname(""), _level(0), _lod(0), _slice(0), _aux(0) {}

            FileObject(size_t ts, string varname, int level = 0, int lod = 0, int aux = 0) : _ts(ts), _varname(varname), _level(level), _lod(lod), _slice(0), _aux(aux) {}

            size_t GetTS() const { return (_ts); }
            string GetVarname() const { return (_varname); }
            int    GetLevel() const { return (_level); }
            int    GetLOD() const { return (_lod); }
            int    GetSlice() const { return (_slice); }
            void   SetSlice(int slice) { _slice = slice; }
            int    GetAux() const { return (_aux); }

        private:
            size_t _ts;
            string _varname;
            int    _level;
            int    _lod;
            int    _slice;
            int    _aux;
        };

        int         AddEntry(FileObject *obj);
        FileObject *GetEntry(int fd) const;
        void        RemoveEntry(int fd);
        vector<int> GetEntries() const;

    private:
        std::vector<FileTable::FileObject *> _table;
    };

protected:
    DC::FileTable _fileTable;

    //! \copydoc Initialize()
    //
    virtual int initialize(const std::vector<string> &paths, const std::vector<string> &options = std::vector<string>()) = 0;

    //! \copydoc GetDimension()
    //
    virtual bool getDimension(string dimname, DC::Dimension &dimension) const = 0;

    //! \copydoc GetDimensionNames()
    //
    virtual std::vector<string> getDimensionNames() const = 0;

    //! \copydoc GetMeshNames()
    //
    virtual std::vector<string> getMeshNames() const = 0;

    //! \copydoc GetMesh()
    //
    virtual bool getMesh(string mesh_name, DC::Mesh &mesh) const = 0;

    //! \copydoc GetCoordVarInfo()
    //
    virtual bool getCoordVarInfo(string varname, DC::CoordVar &cvar) const = 0;

    //! \copydoc GetDataVarInfo()
    //
    virtual bool getDataVarInfo(string varname, DC::DataVar &datavar) const = 0;

    //! \copydoc GetAuxVarInfo()
    //
    virtual bool getAuxVarInfo(string varname, DC::AuxVar &var) const = 0;

    //! \copydoc GetBaseVarInfo()
    //
    virtual bool getBaseVarInfo(string varname, DC::BaseVar &var) const = 0;

    //! \copydoc GetDataVarNames()
    //
    virtual std::vector<string> getDataVarNames() const = 0;

    //! \copydoc GetCoordVarNames()
    //
    virtual std::vector<string> getCoordVarNames() const = 0;

    //! \copydoc GetAuxVarNames()
    //
    virtual std::vector<string> getAuxVarNames() const = 0;

    //! \copydoc GetNumRefLevels()
    //
    virtual size_t getNumRefLevels(string varname) const = 0;

    //! \copydoc GetAtt(string varname, string attname, vector <double> &values)
    //
    virtual bool getAtt(string varname, string attname, vector<double> &values) const = 0;

    //! \copydoc GetAtt(string varname, string attname, vector <long> &values)
    //
    virtual bool getAtt(string varname, string attname, vector<long> &values) const = 0;

    //! \copydoc GetAtt(string varname, string attname, string &values)
    //
    virtual bool getAtt(string varname, string attname, string &values) const = 0;

    //! \copydoc GetAttNames()
    //
    virtual std::vector<string> getAttNames(string varname) const = 0;

    //! \copydoc GetAttType()
    //
    virtual XType getAttType(string varname, string attname) const = 0;

    //! \copydoc GetBlockSize()
    //
    virtual vector<size_t> getBlockSize() const { return (vector<size_t>()); }

    //! \copydoc GetDimLensAtLevel()
    //
    virtual int getDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const = 0;

    //! \copydoc GetMapProjection()
    //
    virtual string getMapProjection() const = 0;

    //! \copydoc OpenVariableRead()
    //
    virtual int openVariableRead(size_t ts, string varname, int level = 0, int lod = 0) = 0;

    //! \copydoc CloseVariable()
    //
    virtual int closeVariable(int fd) = 0;

    //! \copydoc ReadRegion()
    //
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) = 0;

    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) = 0;

    //! \copydoc ReadRegionBlock()
    //
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region) = 0;

    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) = 0;

    //! \copydoc VariableExists()
    //
    virtual bool variableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const = 0;

private:
    virtual bool _getCoordVarDimensions(string varname, bool spatial, vector<DC::Dimension> &dimensions) const;

    virtual bool _getDataVarDimensions(string varname, bool spatial, vector<DC::Dimension> &dimensions) const;

    virtual bool _getAuxVarDimensions(string varname, vector<DC::Dimension> &dimensions) const;

    vector<size_t> _getBlockSize() const;

    virtual int _openVariableRead(size_t ts, string varname, int level = 0, int lod = 0);

    virtual int _closeVariable(int fd);

    template<class T> int _readSliceTemplate(int fd, T *slice);

    template<class T> int _readTemplate(int fd, T *data);

    template<class T> int _getVarTemplate(string varname, int level, int lod, T *data);

    template<class T> int _getVarTemplate(size_t ts, string varname, int level, int lod, T *data);
};
};    // namespace VAPoR

#endif
