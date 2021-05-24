#pragma once

#include <iostream>
#include <vapor/MyBase.h>
#include <vapor/utils.h>
#include <vapor/DC.h>

namespace VAPoR {

class BOVCollection : public Wasp::MyBase {
public:
    BOVCollection();
    int Initialize(const std::vector<std::string> &paths);

    std::string              GetDataFile() const;
    std::string              GetDataVariableName() const;
    std::string              GetTimeDimension() const;
    std::vector<size_t>      GetDataSize() const;
    std::vector<std::string> GetSpatialDimensions() const;
    DC::XType                GetDataFormat() const;
    std::vector<float>       GetBrickOrigin() const;
    std::vector<float>       GetBrickSize() const;
    std::string              GetDataEndian() const;

    template<typename T> int ReadRegion(std::string, T *region);

private:
    std::string              _time;
    std::string              _dataFile;
    std::vector<size_t>      _dataSize;
    DC::XType                _dataFormat;
    std::string              _variable;
    std::string              _dataEndian;
    std::string              _centering;
    std::vector<float>       _brickOrigin;
    std::vector<float>       _brickSize;
    int                      _byteOffset;
    bool                     _divideBrick;
    std::vector<int>         _dataBricklets;
    int                      _dataComponents;

    std::vector<std::string> _spatialDimensions;
    std::string              _timeDimension;

    template<typename T> int _readMetadata(const std::string &token, std::string &line, T &value, bool verbose = true);
    template<> int           _readMetadata<DC::XType>(const std::string &token, std::string &line, DC::XType &value, bool verbose);

    template<typename T> int _readMetadata(const std::string &token, std::string &line, std::vector<T> &value, bool verbose = true);

    std::string _findValue(std::string &line) const;

    static const std::string _timeToken;
    static const std::string _dataFileToken;
    static const std::string _dataSizeToken;
    static const std::string _formatToken;
    static const std::string _variableToken;
    static const std::string _endianToken;
    static const std::string _centeringToken;
    static const std::string _originToken;
    static const std::string _sizeToken;
    static const std::string _offsetToken;
    static const std::string _divideBrickToken;
    static const std::string _dataBrickletsToken;
    static const std::string _dataComponentsToken;
};
}    // namespace VAPoR
