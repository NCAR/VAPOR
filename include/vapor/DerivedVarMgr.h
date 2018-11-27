#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/DC.h>
#include <vapor/DerivedVar.h>

#ifndef _DERIVEDVARMGR_H_
    #define _DERIVEDVARMGR_H_

namespace VAPoR {

//!
//! \class DerivedVarMgr
//!
//! \brief Derived variables constructed from other variables
//!
//! \author John Clyne
//! \date    January, 2017
//!
//!
class VDF_API DerivedVarMgr : public VAPoR::DC {
public:
    //! Class constuctor
    //!
    DerivedVarMgr();

    virtual ~DerivedVarMgr(){};

    void AddCoordVar(DerivedCoordVar *cvar);

    void AddDataVar(DerivedDataVar *dvar);

    void RemoveVar(const DerivedVar *var);

    DerivedVar *GetVar(string name) const;

    bool HasVar(string name) const { return (GetVar(name) != NULL); }

    void AddMesh(const Mesh &m);

protected:
    //! \copydoc Initialize()
    //
    virtual int initialize(const std::vector<string> &paths, const std::vector<string> &options = std::vector<string>());

    //! \copydoc GetDimension()
    //
    virtual bool getDimension(string dimname, DC::Dimension &dimension) const { return (false); }

    //! \copydoc GetDimensionNames()
    //
    virtual std::vector<string> getDimensionNames() const { return (std::vector<string>()); }

    //! \copydoc GetMeshNames()
    //
    virtual std::vector<string> getMeshNames() const;

    //! \copydoc GetMesh()
    //
    virtual bool getMesh(string mesh_name, DC::Mesh &mesh) const;

    //! \copydoc GetCoordVarInfo()
    //
    virtual bool getCoordVarInfo(string varname, DC::CoordVar &cvarInfo) const;

    //! \copydoc GetDataVarInfo()
    //
    virtual bool getDataVarInfo(string varname, DC::DataVar &datavarInfo) const;

    //! \copydoc GetAuxVarInfo()
    //
    virtual bool getAuxVarInfo(string varname, DC::AuxVar &varInfo) const { return (false); }

    //! \copydoc GetBaseVarInfo()
    //
    virtual bool getBaseVarInfo(string varname, DC::BaseVar &varInfo) const;

    //! \copydoc GetDataVarNames()
    //
    virtual std::vector<string> getDataVarNames() const;

    //! \copydoc GetCoordVarNames()
    //
    virtual std::vector<string> getCoordVarNames() const;

    //! \copydoc GetAuxVarNames()
    //
    virtual std::vector<string> getAuxVarNames() const { return (std::vector<string>()); }

    //! \copydoc GetNumRefLevels()
    //
    virtual size_t getNumRefLevels(string varname) const;

    //! \copydoc GetAtt(string varname, string attname, vector <double> &values)
    //
    virtual bool getAtt(string varname, string attname, vector<double> &values) const;

    //! \copydoc GetAtt(string varname, string attname, vector <long> &values)
    //
    virtual bool getAtt(string varname, string attname, vector<long> &values) const;

    //! \copydoc GetAtt(string varname, string attname, string &values)
    //
    virtual bool getAtt(string varname, string attname, string &values) const;

    //! \copydoc GetAttNames()
    //
    virtual std::vector<string> getAttNames(string varname) const;

    //! \copydoc GetAttType()
    //
    virtual XType getAttType(string varname, string attname) const;

    //! \copydoc GetDimLensAtLevel()
    //
    virtual int getDimLensAtLevel(string varname, int level, std::vector<size_t> &dims_at_level, std::vector<size_t> &bs_at_level) const;

    //! \copydoc GetMapProjection()
    //
    virtual string getMapProjection() const { return (string("")); }

    //! \copydoc OpenVariableRead()
    //
    virtual int openVariableRead(size_t ts, string varname, int level = 0, int lod = 0);

    //! \copydoc CloseVariable()
    //
    virtual int closeVariable(int fd);

    //! \copydoc ReadRegion()
    //
    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region);

    virtual int readRegion(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (-1); }

    //! \copydoc ReadRegionBlock()
    //
    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, float *region);

    virtual int readRegionBlock(int fd, const vector<size_t> &min, const vector<size_t> &max, int *region) { return (-1); }

    //! \copydoc VariableExists()
    //
    virtual bool variableExists(size_t ts, string varname, int reflevel = 0, int lod = 0) const;

private:
    std::map<string, DerivedVar *>      _vars;
    std::map<string, DerivedDataVar *>  _dataVars;
    std::map<string, DerivedCoordVar *> _coordVars;
    std::map<string, Mesh>              _meshes;

    DerivedVar *     _getVar(string name) const;
    DerivedDataVar * _getDataVar(string name) const;
    DerivedCoordVar *_getCoordVar(string name) const;

private:
    DC::FileTable _fileTable;
};
};    // namespace VAPoR

#endif
