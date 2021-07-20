#pragma once

#include <array>
#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/utils.h>
#include <vapor/DC.h>

namespace VAPoR {

class BOVCollection : public Wasp::MyBase {
public:
    enum class parseCodes { PARSE_ERROR = -1, NOT_FOUND = 0, FOUND = 1 };

    BOVCollection();
    int Initialize(const std::vector<std::string> &paths);

    std::vector<std::string>   GetDataVariableNames() const;
    std::string                GetTimeDimension() const;
    std::vector<float>         GetUserTimes() const;
    float                      GetUserTime(size_t ts) const { return GetUserTimes()[ts]; };
    std::array<size_t, 3>      GetDataSize() const;
    std::array<std::string, 3> GetSpatialDimensions() const;
    DC::XType                  GetDataFormat() const;
    std::array<double, 3>      GetBrickOrigin() const;
    std::array<double, 3>      GetBrickSize() const;
    std::string                GetDataEndian() const;

    template<class T> int ReadRegion(std::string varname, size_t ts, const std::vector<size_t> &min, const std::vector<size_t> &max, T region);

private:
    float                    _time;
    std::vector<float>       _times;
    std::string              _dataFile;
    std::vector<std::string> _dataFiles;
    std::array<size_t, 3>    _gridSize;
    DC::XType                _dataFormat;
    std::string              _variable;
    std::vector<std::string> _variables;
    std::string              _dataEndian;
    std::string              _centering;
    std::array<double, 3>    _brickOrigin;
    std::array<double, 3>    _brickSize;
    size_t                   _byteOffset;
    bool                     _divideBrick;
    std::array<size_t, 3>    _dataBricklets;
    int                      _dataComponents;

    // Placeholder variables to store values read from BOV descriptor files.
    // These values must be consistent among BOV files, and are validated before
    // assigning to "actual" values such as _gridSize, declaired above.
    std::array<size_t, 3> _tmpGridSize;
    DC::XType             _tmpDataFormat;
    std::string           _tmpDataEndian;
    std::array<double, 3> _tmpBrickOrigin;
    std::array<double, 3> _tmpBrickSize;
    size_t                _tmpByteOffset;

    bool _gridSizeAssigned;
    bool _formatAssigned;
    bool _brickOriginAssigned;
    bool _brickSizeAssigned;
    bool _dataEndianAssigned;
    bool _byteOffsetAssigned;

    // _dataFileMap allows us to access binary data files with a
    // varname/timestep pair
    std::map<std::string, std::map<float, std::string>> _dataFileMap;

    std::array<std::string, 3> _spatialDimensions;
    int                        _validateParsedValues();
    std::string                _timeDimension;

    int  _parseHeader(std::ifstream &header);
    void _populateDataFileMap();

    template<typename T> int _findToken(const std::string &token, std::string &line, T &value, bool verbose = false);
    template<typename T> int _findToken(const std::string &token, std::string &line, std::array<T, 3> &value, bool verbose = false);

    void _findTokenValue(std::string &line) const;

    int  _sizeOfFormat(DC::XType) const;
    void _swapBytes(void *vptr, size_t size, size_t n) const;

    int _invalidDimensionError(std::string token) const;
    int _invalidFormatError(std::string token) const;
    int _failureToReadError(std::string token) const;
    int _inconsistentValueError(std::string token) const;
    int _invalidValueError(std::string token) const;
    int _missingValueError(std::string token) const;

    static const std::string TIME_TOKEN;
    static const std::string DATA_FILE_TOKEN;
    static const std::string GRID_SIZE_TOKEN;
    static const std::string FORMAT_TOKEN;
    static const std::string VARIABLE_TOKEN;
    static const std::string ENDIAN_TOKEN;
    static const std::string CENTERING_TOKEN;
    static const std::string ORIGIN_TOKEN;
    static const std::string BRICK_SIZE_TOKEN;
    static const std::string OFFSET_TOKEN;
    static const std::string DIVIDE_BRICK_TOKEN;
    static const std::string DATA_BRICKLETS_TOKEN;
    static const std::string DATA_COMPONENTS_TOKEN;

    static const double                _defaultTime;
    static const std::string           _defaultFile;
    static const DC::XType             _defaultFormat;
    static const std::string           _defaultVar;
    static const std::string           _defaultEndian;
    static const std::string           _defaultCentering;
    static const size_t                _defaultByteOffset;
    static const size_t                _defaultComponents;
    static const bool                  _defaultDivBrick;
    static const std::array<double, 3> _defaultOrigin;
    static const std::array<double, 3> _defaultBrickSize;
    static const std::array<size_t, 3> _defaultGridSize;
    static const std::array<size_t, 3> _defaultBricklets;

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
