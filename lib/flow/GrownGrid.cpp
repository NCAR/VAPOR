#include "GrownGrid.h"

using VAPoR::GrownGrid;

// Constructor
GrownGrid::GrownGrid( const VAPoR::Grid* gp, VAPoR::DataMgr* mp, float z)
            : _grid2d( gp ), _dataMgr( mp ), _defaultZ( z )
{}
    
// Destructor
GrownGrid::~GrownGrid()
{
    if( _grid2d && _dataMgr )
    {
        _dataMgr->UnlockGrid( _grid2d );
        delete _grid2d;
    }
}
    

float GrownGrid::GetDefaultZ() const
{
    return _defaultZ;
}
    
float GrownGrid::GetValue(const std::vector <double> &coords) const
{
    return _grid2d->GetValue( coords );
}

std::string GrownGrid::GetType() const
{
    std::string type( "GrownGrid" );
    return type;
}

float GrownGrid::GetMissingValue() const
{
    return _grid2d->GetMissingValue();
}

void GrownGrid::GetUserExtentsHelper( double minu[3], double maxu[3]) const
{
    _grid2d->GetUserExtents( minu, maxu );

	if (_grid2d->GetGeometryDim() < 3) {
		minu[2] = _defaultZ;
		maxu[2] = _defaultZ;
	}
}
    
bool GrownGrid::InsideGrid(const double coords[3]) const
{
    // Note that we don't use defaultZ to decide if a position is inside of 
    // a grid or not.
    return ( _grid2d->InsideGrid( coords ) );
}

    
//
// Start meaningless functions!
//
float GrownGrid::GetValueNearestNeighbor( const double *) const
{
    return 0.0f;
}
    
float GrownGrid::GetValueLinear( const double *) const 
{
    return 0.0f;
}
std::vector<size_t> GrownGrid::GetCoordDimensions(size_t) const
{
    std::vector<size_t> tmp;
    return tmp;
}
    

size_t GrownGrid::GetGeometryDim() const 
{
    return 0;
}
    

const std::vector<size_t>& GrownGrid::GetNodeDimensions() const 
{
    return( GetDimensions() );
}


const std::vector<size_t>& GrownGrid::GetCellDimensions() const 
{
    return( GetDimensions() );
}
    
bool GrownGrid::GetIndicesCell( const double coords[3],
     size_t indices[3]) const 
{
    return false;
}
    
bool GrownGrid::GetCellNodes( const size_t cindices[], size_t nodes[], int &n) const 
{
    return false;
}
    
bool GrownGrid::GetCellNeighbors( const std::vector <size_t> &cindices,
     std::vector <std::vector <size_t> > &cells) const 
{
    return false;
}
    
bool GrownGrid::GetNodeCells( const std::vector <size_t> &indices,
     std::vector <std::vector <size_t> > &cells) const 
{
    return false;
}
    
size_t GrownGrid::GetMaxVertexPerFace() const 
{
    return 0;
}
    
size_t GrownGrid::GetMaxVertexPerCell() const 
{
    return 0;
}
    
VAPoR::Grid::ConstCoordItr GrownGrid::ConstCoordBegin() const
{
    return VAPoR::Grid::ConstCoordItr();
}
    
VAPoR::Grid::ConstCoordItr GrownGrid::ConstCoordEnd() const
{
    return VAPoR::Grid::ConstCoordItr();
}
