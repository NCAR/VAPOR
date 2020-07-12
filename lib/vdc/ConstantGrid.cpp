#include <limits>

#include "vapor/ConstantGrid.h"

using VAPoR::ConstantGrid;


ConstantGrid::ConstantGrid( float v, size_t d )
            : _value( v ), _topologyDim( d )
              
{}
    
    
float ConstantGrid::GetConstantValue() const
{
    return _value;
}
    
float ConstantGrid::GetValue(const double coords[3]) const
{
    return _value;
}
    
float ConstantGrid::GetValueNearestNeighbor( const double coords[3]) const
{
    return _value;
}
    
float ConstantGrid::GetValueLinear( const double coords[3]) const 
{
    return _value;
}

std::string ConstantGrid::GetType() const
{
    std::string type( "ConstantGrid" );
    return type;
}
    
size_t ConstantGrid::GetTopologyDim() const
{
    return _topologyDim;
}

void ConstantGrid::GetUserExtentsHelper( double minu[3], double maxu[3]) const
{
	for (int i=0; i<GetGeometryDim(); i++) {
		minu[i] = std::numeric_limits<double>::min();
		maxu[i] = std::numeric_limits<double>::max();
	}
}
    
bool ConstantGrid::InsideGrid(const double coords[3]) const
{
    return true;
}

std::vector<size_t> ConstantGrid::GetCoordDimensions(size_t) const
{
    std::vector<size_t> tmp;
    return tmp;
}
    

size_t ConstantGrid::GetGeometryDim() const 
{
    return 0;
}
    

const std::vector<size_t>& ConstantGrid::GetNodeDimensions() const 
{
    return( GetDimensions() );
}


const std::vector<size_t>& ConstantGrid::GetCellDimensions() const 
{
    return( GetDimensions() );
}
    
bool ConstantGrid::GetIndicesCell( const double coords[3],
     size_t indices[3]) const 
{
    return false;
}
    
bool ConstantGrid::GetCellNodes( const size_t cindices[], size_t nodes[], int &n) const 
{
    return false;
}
    
bool ConstantGrid::GetCellNeighbors( const std::vector <size_t> &cindices,
     std::vector <std::vector <size_t> > &cells) const 
{
    return false;
}
    
bool ConstantGrid::GetNodeCells( const std::vector <size_t> &indices,
     std::vector <std::vector <size_t> > &cells) const 
{
    return false;
}
    
size_t ConstantGrid::GetMaxVertexPerFace() const 
{
    return 0;
}
    
size_t ConstantGrid::GetMaxVertexPerCell() const 
{
    return 0;
}
    
VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordBegin() const
{
    return VAPoR::Grid::ConstCoordItr();
}
    
VAPoR::Grid::ConstCoordItr ConstantGrid::ConstCoordEnd() const
{
    return VAPoR::Grid::ConstCoordItr();
}
