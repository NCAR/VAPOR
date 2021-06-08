#pragma once

#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/utils.h>
#include <vapor/DC.h>

namespace VAPoR {

class BOVCollection : public Wasp::MyBase {
public:
    enum parseCodes { ERROR = -1, NOT_FOUND = 0, FOUND = 1 };

    BOVCollection();
    int Initialize(const std::vector<std::string> &paths);

    std::string              GetDataFile() const;
    std::string              GetDataVariableName() const;
    std::string              GetTimeDimension() const;
    double                   GetUserTime() const;
    std::vector<double>      GetUserTimes() const;
    std::vector<size_t>      GetDataSize() const;
    std::vector<std::string> GetSpatialDimensions() const;
    DC::XType                GetDataFormat() const;
    std::vector<double>      GetBrickOrigin() const;
    std::vector<double>      GetBrickSize() const;
    std::string              GetDataEndian() const;

    template<class T> int ReadRegion(std::string varname, size_t ts, const std::vector<size_t> &min, const std::vector<size_t> &max, T region);

private:
    double                   _time;
    std::vector<double>      _times;
    std::string              _dataFile;
    std::vector<std::string> _dataFiles;
    std::vector<size_t>      _gridSize;
    DC::XType                _dataFormat;
    std::string              _variable;
    std::vector<std::string> _variables;
    std::string              _dataEndian;
    std::string              _centering;
    std::vector<double>      _brickOrigin;
    std::vector<double>      _brickSize;
    size_t                   _byteOffset;
    bool                     _divideBrick;
    std::vector<size_t>      _dataBricklets;
    int                      _dataComponents;

    // _dataFiles allows us to access the data files via the following:
    //      std::string file = _dataFiles[var][timeStep]
    //
    std::map<std::string, std::map<size_t, std::string>> _dataFileMap;
    std::vector<std::string> _spatialDimensions;
    std::string              _timeDimension;

    bool _gridSizeAssigned;
    bool _formatAssigned;
    bool _brickOriginAssigned;
    bool _brickSizeAssigned;
    bool _dataEndianAssigned;

    int _parseHeader(std::ifstream &header);

    template<typename T> int _findToken(const std::string &token, std::string &line, T &value, bool verbose = true);
    template<typename T> int _findToken(const std::string &token, std::string &line, std::vector<T> &value, bool verbose = true);

    void _findTokenValue(std::string &line) const;

    size_t _sizeOfFormat(DC::XType) const;
    void   _swapBytes(void *vptr, size_t size, size_t n) const;

    int _invalidDimensionError(std::string token) const;
    int _invalidFormatError(std::string token) const;
    int _failureToReadError(std::string token) const;
    int _inconsistentValueError(std::string token) const;
    int _invalidValueError(std::string token) const;
    int _missingValueError(std::string token) const;

    static const std::string _timeToken;
    static const std::string _dataFileToken;
    static const std::string _gridSizeToken;
    static const std::string _formatToken;
    static const std::string _variableToken;
    static const std::string _endianToken;
    static const std::string _centeringToken;
    static const std::string _originToken;
    static const std::string _brickSizeToken;
    static const std::string _offsetToken;
    static const std::string _divideBrickToken;
    static const std::string _dataBrickletsToken;
    static const std::string _dataComponentsToken;

    static const double              _defaultTime;
    static const std::string         _defaultFile;
    static const DC::XType           _defaultFormat;
    static const std::string         _defaultVar;
    static const std::string         _defaultEndian;
    static const std::string         _defaultCentering;
    static const size_t              _defaultOffset;
    static const size_t              _defaultComponents;
    static const bool                _defaultDivBrick;
    static const std::vector<double> _defaultOrigin;
    static const std::vector<double> _defaultBrickSize;
    static const std::vector<size_t> _defaultGridSize;
    static const std::vector<size_t> _defaultBricklets;

    static const std::string _xDim;
    static const std::string _yDim;
    static const std::string _zDim;
    static const std::string _timeDim;

    static const std::string _byteFormatString;
    static const std::string _shortFormatString;
    static const std::string _intFormatString;
    static const std::string _floatFormatString;
    static const std::string _doubleFormatString;
};

// Make gcc happy by moving template specialization outside of the class body
//
template<> int BOVCollection::_findToken<DC::XType>(const std::string &token, std::string &line, DC::XType &value, bool verbose);
}    // namespace VAPoR
