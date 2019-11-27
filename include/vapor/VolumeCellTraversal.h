#pragma once

#include <vapor/VolumeRegular.h>

namespace VAPoR {

//! \class VolumeCellTraversal
//! \ingroup Public_Render
//!
//! \brief Curvilinear grid rendering algorithm
//!
//! \author Stanislaw Jaroszynski
//! \date Feburary, 2019
//!
//! Renders a curvilinear grid by traversing through the cells. The c++ code
//! does the following:
//! 1. Loads scalar data
//! 2. Loads the coordinates
//! 3. Generates a bounding box for every border face
//! 4. Groups bounding boxs recursively into a tree, i.e. a bounding box at level n
//!    would encapsulate the 4 associated bounding boxes at level n-1
//!
//! The glsl code does the following:
//! 1. Find the initial border face that the ray intersects with by traversing
//!    the tree built on the CPU
//! 2. Loop:
//! 3. Find the exit face of the current cell
//! 4. Render the ray segment from the entrance to the exit
//! 5. Set the current exit face to the new entrance face
//! 6. Goto Loop until the ray exits the volume

class VolumeCellTraversal : public VolumeRegular {
public:
    VolumeCellTraversal(GLManager *gl);
    ~VolumeCellTraversal();

    static std::string GetName() { return "Curvilinear"; }
    static Type        GetType() { return Type::DVR; }
    virtual bool       RequiresChunkedRendering() { return true; }

    virtual int            LoadData(const Grid *grid);
    virtual ShaderProgram *GetShader() const;
    virtual void           SetUniforms(const ShaderProgram *shader) const;
    virtual float          GuestimateFastModeSpeedupFactor() const;

private:
    Texture3D      _coordTexture;
    Texture2DArray _minTexture;
    Texture2DArray _maxTexture;
    Texture2D      _BBLevelDimTexture;

    int  _coordDims[3];
    int  _BBLevels;
    bool _useHighPrecisionTriangleRoutine;
    bool _gridHasInvertedCoordinateSystemHandiness;

    bool        _needsHighPrecisionTriangleRoutine(const Grid *grid);
    static bool _need32BitForCoordinates(const Grid *grid);

protected:
    int                 _getHeuristicBBLevels() const;
    virtual std::string _addDefinitionsToShader(std::string shaderName) const;
};

//! \class VolumeCellTraversalIso
//! \ingroup Public_Render
//!
//! \brief Curvilinear grid isosurface rendering algorithm
//!
//! \author Stanislaw Jaroszynski
//! \date Feburary, 2019
//!
//! Renders isosurfaces by ray tracing. This class is the same as the curvilinear DVR
//! except it renders an isosurface

class VolumeCellTraversalIso : public VolumeCellTraversal {
public:
    VolumeCellTraversalIso(GLManager *gl) : VolumeCellTraversal(gl) {}
    static std::string     GetName() { return "Iso Curvilinear"; }
    static Type            GetType() { return Type::Iso; }
    virtual ShaderProgram *GetShader() const;
    virtual void           SetUniforms(const ShaderProgram *shader) const;
};

}    // namespace VAPoR
