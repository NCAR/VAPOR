#include <memory>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <float.h>
#include "vapor/VAssert.h"
#include "vapor/utils.h"
#include "vapor/FileUtils.h"
#include <cstdio>
#include <climits>
#include <cmath>

#include <vapor/BOVCollection.h>

using namespace VAPoR;

const std::string BOVCollection::TIME_TOKEN = "TIME";
const std::string BOVCollection::DATA_FILE_TOKEN = "DATA_FILE";
const std::string BOVCollection::GRID_SIZE_TOKEN = "DATA_SIZE";
const std::string BOVCollection::FORMAT_TOKEN = "DATA_FORMAT";
const std::string BOVCollection::VARIABLE_TOKEN = "VARIABLE";
const std::string BOVCollection::ORIGIN_TOKEN = "BRICK_ORIGIN";
const std::string BOVCollection::BRICK_SIZE_TOKEN = "BRICK_SIZE";
const std::string BOVCollection::OFFSET_TOKEN = "BYTE_OFFSET";

// These tokens are parsed, but not used in ReadRegion() logic
const std::string BOVCollection::ENDIAN_TOKEN = "DATA_ENDIAN";
const std::string BOVCollection::CENTERING_TOKEN = "CENTERING";
const std::string BOVCollection::DIVIDE_BRICK_TOKEN = "DIVIDE_BRICK";
const std::string BOVCollection::DATA_BRICKLETS_TOKEN = "DATA_BRICKLETS";
const std::string BOVCollection::DATA_COMPONENTS_TOKEN = "DATA_COMPONENTS";

const std::array<double, 3> BOVCollection::_defaultOrigin = {0., 0., 0.};
const std::array<double, 3> BOVCollection::_defaultBrickSize = {1., 1., 1.};
const std::array<size_t, 3> BOVCollection::_defaultGridSize = {0, 0, 0};
const DC::XType             BOVCollection::_defaultFormat = DC::XType::INVALID;
const std::string           BOVCollection::_defaultFile = "";
const std::string           BOVCollection::_defaultVar = "brickVar";
const double                BOVCollection::_defaultTime = FLT_MIN;
const size_t                BOVCollection::_defaultByteOffset = 0;

// Currently unused in ReadRegion() logic
const std::string           BOVCollection::_defaultEndian = "LITTLE";
const std::string           BOVCollection::_defaultCentering = "ZONAL";
const bool                  BOVCollection::_defaultDivBrick = false;
const std::array<size_t, 3> BOVCollection::_defaultBricklets = {0, 0, 0};
const size_t                BOVCollection::_defaultComponents = 1;

const std::string BOVCollection::_xDim = "x";
const std::string BOVCollection::_yDim = "y";
const std::string BOVCollection::_zDim = "z";
const std::string BOVCollection::_timeDim = "t";

const std::string BOVCollection::_byteFormatString = "BYTE";
const std::string BOVCollection::_shortFormatString = "SHORT";
const std::string BOVCollection::_intFormatString = "INT";
const std::string BOVCollection::_floatFormatString = "FLOAT";
const std::string BOVCollection::_doubleFormatString = "DOUBLE";

BOVCollection::BOVCollection()
: _time(_defaultTime), _dataFile(_defaultFile), _dataFormat(_defaultFormat), _variable(_defaultVar), _byteOffset(_defaultByteOffset), _divideBrick(_defaultDivBrick), _dataEndian(_defaultEndian),
  _centering(_defaultCentering), _dataComponents(_defaultComponents), _tmpDataFormat(_defaultFormat), _tmpByteOffset(_defaultByteOffset), _gridSizeAssigned(false), _formatAssigned(false),
  _brickOriginAssigned(false), _brickSizeAssigned(false), _byteOffsetAssigned(false), _timeDimension(_timeDim)
{
    // Note: the following variables are unused in the ReadRegion() logic
    // _defaultEndian
    // _defaultCentering
    // _defaultDivBrick
    // _defaultBricklets
    // _defaultComponents

    _dataFiles.clear();
    _times.clear();
    _gridSize = _defaultGridSize;
    _tmpGridSize = _defaultGridSize;
    _brickOrigin = _defaultOrigin;
    _tmpBrickOrigin = _defaultOrigin;
    _brickSize = _defaultBrickSize;
    _tmpBrickSize = _defaultBrickSize;
    _dataBricklets = _defaultBricklets;
    _spatialDimensions = {_xDim, _yDim, _zDim};
}

int BOVCollection::Initialize(const std::vector<std::string> &paths)
{
    VAssert(paths.size() > 0);

    int           rc;
    std::ifstream header;
    for (int i = 0; i < paths.size(); i++) {
        _dataFile = _defaultFile;

        // Save the path to the BOV header so we can add it
        // to data files given with a relative path
        _currentFilePath = Wasp::FileUtils::Dirname(paths[i]);

        header.open(paths[i]);
        if (header.is_open()) {
            rc = _parseHeader(header);
            if (rc < 0) {
                SetErrMsg(("Error parsing BOV file " + paths[0]).c_str());
                return -1;
            }

            // If _dataFile is not an absolute path, prepend with the BOV header's path
            if (!Wasp::FileUtils::IsPathAbsolute(_dataFile)) {
                auto dirAndPath = {_currentFilePath, _dataFile};
                _dataFile = Wasp::FileUtils::JoinPaths(dirAndPath);
            }

            rc = _validateParsedValues();
            if (rc < 0) {
                SetErrMsg("Validating BOV tokens failed");
                return -1;
            }

            // Ensure we have the required tokens in the header
            //
            if (_dataFile == _defaultFile) { return _missingValueError(DATA_FILE_TOKEN); }
            if (_dataFormat == _defaultFormat) { return _missingValueError(FORMAT_TOKEN); }
            if (_gridSize == _defaultGridSize) { return _missingValueError(GRID_SIZE_TOKEN); }
            if (_time == _defaultTime) { return _missingValueError(TIME_TOKEN); }

            rc = _populateDataFileMap();
            if (rc < 0) {
                SetErrMsg("Problem indexing data files.");
                return -1;
            }
        } else {
            SetErrMsg(("Failed to open BOV file " + paths[0]).c_str());
            return -1;
        }
        header.close();
    }

    return 0;
}

int BOVCollection::_parseHeader(std::ifstream &header)
{
    int         rc;
    std::string line;
    std::string dataFile;
    while (getline(header, line)) {
        // The _dataFile, _gridSize, and _dataFormat variables are all required to process
        // the BOV.  Try to find them, and report errors if we can't.
        //
        rc = _findToken(DATA_FILE_TOKEN, line, dataFile);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _failureToReadError(DATA_FILE_TOKEN);
        else if (rc == (int)parseCodes::FOUND) {
            _dataFile = dataFile;
            continue;
        }

        double time;
        rc = _findToken(TIME_TOKEN, line, time);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _failureToReadError(TIME_TOKEN);
        else if (rc == (int)parseCodes::FOUND) {
            _time = time;
            continue;
        }

        std::string variable;
        rc = _findToken(VARIABLE_TOKEN, line, variable);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _invalidValueError(VARIABLE_TOKEN);
        else if (rc == (int)parseCodes::FOUND) {
            _variable = variable;
            continue;
        }

        rc = _findToken(GRID_SIZE_TOKEN, line, _tmpGridSize);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _failureToReadError(GRID_SIZE_TOKEN);
        else if (rc == (int)parseCodes::FOUND)
            continue;

        rc = _findToken(FORMAT_TOKEN, line, _tmpDataFormat);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _failureToReadError(FORMAT_TOKEN);
        else if (rc == (int)parseCodes::FOUND)
            continue;

        // Optional tokens.  If their values are invalid, SetErrMsg, and return -1.
        //
        rc = _findToken(ORIGIN_TOKEN, line, _tmpBrickOrigin);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _invalidValueError(ORIGIN_TOKEN);
        else if (rc == (int)parseCodes::FOUND)
            continue;

        rc = _findToken(BRICK_SIZE_TOKEN, line, _tmpBrickSize);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _invalidValueError(BRICK_SIZE_TOKEN);
        else if (rc == (int)parseCodes::FOUND)
            continue;

        rc = _findToken(OFFSET_TOKEN, line, _tmpByteOffset);
        if (rc == (int)parseCodes::PARSE_ERROR)
            return _invalidValueError(OFFSET_TOKEN);
        else if (rc == (int)parseCodes::FOUND)
            continue;

        // All other variables are currently unused.
        //
        _findToken(ENDIAN_TOKEN, line, _dataEndian);
        _findToken(CENTERING_TOKEN, line, _centering);
        _findToken(DIVIDE_BRICK_TOKEN, line, _divideBrick);
        _findToken(DATA_BRICKLETS_TOKEN, line, _dataBricklets);
        _findToken(DATA_COMPONENTS_TOKEN, line, _dataComponents);
    }
    return 0;
}

int BOVCollection::_validateParsedValues()
{
    // Validate grid dimensions
    if (_tmpGridSize[0] < 1 || _tmpGridSize[1] < 1 || _tmpGridSize[2] < 1)
        return _invalidDimensionError(GRID_SIZE_TOKEN);
    else if (_tmpGridSize != _gridSize && _gridSizeAssigned == true)
        return _inconsistentValueError(GRID_SIZE_TOKEN);
    else {
        _gridSize = _tmpGridSize;
        _gridSizeAssigned = true;
    }


    // Validate data format
    if (_tmpDataFormat == DC::INVALID)
        return _invalidFormatError(FORMAT_TOKEN);
    else if (_tmpDataFormat != _dataFormat && _formatAssigned == true) {
        return _inconsistentValueError(FORMAT_TOKEN);
    } else {
        _dataFormat = _tmpDataFormat;
        _formatAssigned = true;
    }

    // Validate brick origin
    if (_tmpBrickOrigin != _brickOrigin && _brickOriginAssigned == true)
        return _inconsistentValueError(ORIGIN_TOKEN);
    else {
        _brickOrigin = _tmpBrickOrigin;
        _brickOriginAssigned = true;
    }

    // Validate brick size
    if (_tmpBrickSize != _brickSize && _brickSizeAssigned == true)
        return _inconsistentValueError(BRICK_SIZE_TOKEN);
    else {
        _brickSize = _tmpBrickSize;
        _brickSizeAssigned = true;
    }

    // Validate byte offest
    if (_tmpByteOffset != _byteOffset && _byteOffsetAssigned == true)
        return _inconsistentValueError(OFFSET_TOKEN);
    else {
        _byteOffset = _tmpByteOffset;
        _byteOffsetAssigned = true;
    }

    if (_variable.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-") != std::string::npos) return _invalidVarNameError();

    return 0;
}

int BOVCollection::_populateDataFileMap()
{
    if (_dataFileMap[_variable].count(_time)) {
        SetErrMsg("Duplicate time entries found in BOV files.  Each file must uniquely describe one variable, at one timestep.");
        return -1;
    }

    _variables.push_back(_variable);

    if (std::find(_times.begin(), _times.end(), _time) == _times.end()) _times.push_back(_time);

    std::sort(_times.begin(), _times.end());

    _dataFileMap[_variable][_time] = _dataFile;
    return 0;
}

int BOVCollection::_invalidVarNameError() const
{
    SetErrMsg("Invalid variable name.  (Must be alphanumeric)");
    return -1;
}


int BOVCollection::_invalidFileSizeError(size_t numElements) const
{
    SetErrMsg(("Data file " + _dataFile + ", which has " + to_string(numElements)
               + " values, does not match the size of the the data and offset "
                 "specified in BOV header.")
                  .c_str());
    return -1;
}

int BOVCollection::_invalidFileError() const
{
    SetErrMsg(("Reading " + _dataFile + " failed with error: " + strerror(errno)).c_str());
    return -1;
}

int BOVCollection::_missingValueError(const std::string &token) const
{
    SetErrMsg(("BOV file must contain token: " + token).c_str());
    return -1;
}

int BOVCollection::_invalidDimensionError(const std::string &token) const
{
    SetErrMsg((token + " must have all dimensions > 1").c_str());
    return -1;
}

int BOVCollection::_invalidFormatError(const std::string &token) const
{
    std::string message = token + " must be either INT, FLOAT, or DOUBLE.";
    SetErrMsg(message.c_str());
    return -1;
}

int BOVCollection::_failureToReadError(const std::string &token) const
{
    SetErrMsg(("Failure reading BOV token: " + token).c_str());
    return -1;
}

int BOVCollection::_inconsistentValueError(const std::string &token) const
{
    SetErrMsg((token + " must be consistent in all BOV files").c_str());
    return -1;
}

int BOVCollection::_invalidValueError(const std::string &token) const
{
    SetErrMsg(("Invalid value for token: " + token).c_str());
    return -1;
}

std::vector<std::string> BOVCollection::GetDataVariableNames() const { return _variables; }

std::array<std::string, 3> BOVCollection::GetSpatialDimensions() const { return _spatialDimensions; }

std::string BOVCollection::GetTimeDimension() const { return _timeDimension; }

std::vector<float> BOVCollection::GetUserTimes() const { return _times; }

std::array<size_t, 3> BOVCollection::GetDataSize() const { return _gridSize; }

DC::XType BOVCollection::GetDataFormat() const { return _dataFormat; }

std::array<double, 3> BOVCollection::GetBrickOrigin() const { return _brickOrigin; }

std::array<double, 3> BOVCollection::GetBrickSize() const { return _brickSize; }

// Template specialization for reading data of type DC::XType
template<> int BOVCollection::_findToken<DC::XType>(const std::string &token, std::string &line, DC::XType &value, bool verbose)
{
    // Skip comments
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '#') {
            line.erase(line.begin() + i, line.end());
            break;
        }
    }

    if (line.length() == 0) return (int)parseCodes::NOT_FOUND;

    while (line.length() > 0) {
        if (line[line.length() - 1] == ' ')    // If last char is a space, pop it
            line.pop_back();
        else
            break;
    }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        std::string format = line;
        _findTokenValue(format);
        if (format == _intFormatString)
            value = DC::INT32;
        else if (format == _floatFormatString)
            value = DC::FLOAT;
        else if (format == _doubleFormatString)
            value = DC::DOUBLE;
        else {
            value = DC::INVALID;
            _invalidFormatError(token);
            return (int)parseCodes::PARSE_ERROR;
        }

        if (verbose) { std::cout << std::setw(20) << token << " " << value << std::endl; }
        return (int)parseCodes::FOUND;
    }
    return (int)parseCodes::NOT_FOUND;
}

// Template specialization for reading data of types bool or string
template<typename T> int BOVCollection::_findToken(const std::string &token, std::string &line, T &value, bool verbose)
{
    // Skip comments
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '#') {
            line.erase(line.begin() + i, line.end());
            break;
        }
    }

    if (line.length() == 0) return (int)parseCodes::NOT_FOUND;

    while (line.length() > 0) {
        if (line[line.length() - 1] == ' ')    // If last char is a space, pop it
            line.pop_back();
        else
            break;
    }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        _findTokenValue(line);
        stringstream ss(line);
        if (std::is_same<T, bool>::value) {
            ss >> std::boolalpha >> value;
        } else {
            ss >> value;
        }

        if (verbose) { std::cout << std::setw(20) << token << " " << value << std::endl; }

        if (ss.fail()) { return (int)parseCodes::PARSE_ERROR; }
        // If there is more than one value, throw error
        if (ss.eof() == false) return (int)parseCodes::PARSE_ERROR;

        return (int)parseCodes::FOUND;
    }
    return (int)parseCodes::NOT_FOUND;
}

// Template specialization for reading data of type std::array<size_t, 3> or std::array<float, 3>
template<typename T> int BOVCollection::_findToken(const std::string &token, std::string &line, std::array<T, 3> &value, bool verbose)
{
    // Skip comments
    for (size_t i = 0; i < line.length(); i++) {
        if (line[i] == '#') {
            line.erase(line.begin() + i, line.end());
            break;
        }
    }

    if (line.length() == 0) return (int)parseCodes::NOT_FOUND;

    while (line.length() > 0) {
        if (line[line.length() - 1] == ' ')    // If last char is a space, pop it
            line.pop_back();
        else
            break;
    }

    size_t pos = line.find(token);
    if (pos != std::string::npos) {    // We found the token
        T lineValue;
        _findTokenValue(line);
        std::stringstream lineStream(line);

        for (int i = 0; i < value.size(); i++) {
            lineStream >> lineValue;
            if (lineStream.fail()) return (int)parseCodes::PARSE_ERROR;

            value[i] = lineValue;
        }

        // If there are more than 3 values, throw error
        if (!lineStream.eof()) return (int)parseCodes::PARSE_ERROR;

        if (verbose) {
            std::cout << std::setw(20) << token << " ";
            for (int i = 0; i < value.size(); i++) std::cout << value[i] << " ";
            std::cout << std::endl;
        }
        return (int)parseCodes::FOUND;
    }
    return (int)parseCodes::NOT_FOUND;
}

void BOVCollection::_findTokenValue(std::string &line) const
{
    std::string delimiter = ": ";

    size_t      pos = 0;
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        line.erase(0, pos + delimiter.length());
    }
}

int BOVCollection::_sizeOfFormat(DC::XType type) const
{
    switch (type) {
    case DC::XType::INT32: return 4;
    case DC::XType::FLOAT: return 4;
    case DC::XType::DOUBLE: return 8;
    default: return -1;
    }
}

template<class T> int BOVCollection::ReadRegion(std::string varname, size_t ts, const std::vector<size_t> &min, const std::vector<size_t> &max, T region)
{
    float       time = _times[ts];
    std::string dataFile = _dataFileMap[varname][time];

    FILE *fp = fopen(dataFile.c_str(), "rb");
    if (!fp) {
        if (dataFile == "")
            SetErrMsg("No data file associated with variable '%s' at timestep %d", varname.c_str(), time);
        else
            SetErrMsg("Invalid file: %s : %M", dataFile.c_str());
        return -1;
    }

    int formatSize = _sizeOfFormat(_dataFormat);
    if (formatSize < 0) {
        SetErrMsg("Unspecified data format");
        fclose(fp);
        return -1;
    }

    // Read a "pencil" of data along the X axis, one row at a time
    size_t count = max[0] - min[0] + 1;
    // Note: allocate buffer once and reuse for many times, so repeated allocation is avoided.
    std::vector<unsigned char> vReadBuffer(count * formatSize);
    unsigned char *            readBuffer = vReadBuffer.data();

    for (size_t k = min[2]; k <= max[2]; k++) {
        size_t zOffset = _gridSize[0] * _gridSize[1] * k;
        for (size_t j = min[1]; j <= max[1]; j++) {
            size_t xOffset = min[0];
            size_t yOffset = _gridSize[0] * j;
            size_t offset = formatSize * (xOffset + yOffset + zOffset) + _byteOffset;

            int rc = fseek(fp, offset, SEEK_SET);
            if (rc != 0) {
                MyBase::SetErrMsg("Unable to seek on file: %M");
                return -1;
            }

            size_t fread_rc = fread(readBuffer, formatSize, count, fp);
            if (fread_rc != count) {
                if (ferror(fp) != 0) {
                    MyBase::SetErrMsg("Error reading input file: %M");
                } else {
                    MyBase::SetErrMsg("Short read on input file: %M");
                }
                fclose(fp);
                return -1;
            }

            if (_dataFormat == DC::XType::INT32) {
                int *castBuffer = (int *)readBuffer;
                for (int i = 0; i < count; i++) { *region++ = (typename std::remove_pointer<T>::type)castBuffer[i]; }
            } else if (_dataFormat == DC::XType::FLOAT) {
                float *castBuffer = (float *)readBuffer;
                for (int i = 0; i < count; i++) { *region++ = (typename std::remove_pointer<T>::type)castBuffer[i]; }
            } else if (_dataFormat == DC::XType::DOUBLE) {
                double *castBuffer = (double *)readBuffer;
                for (int i = 0; i < count; i++) { *region++ = (typename std::remove_pointer<T>::type)castBuffer[i]; }
            }
        }
    }

    fclose(fp);

    return 0;
}

// ReadRegion can only be used with int* float* and double*
template int BOVCollection::ReadRegion<int *>(std::string varname, size_t ts, const std::vector<size_t> &, const std::vector<size_t> &, int *);
template int BOVCollection::ReadRegion<float *>(std::string varname, size_t ts, const std::vector<size_t> &, const std::vector<size_t> &, float *);
template int BOVCollection::ReadRegion<double *>(std::string varname, size_t ts, const std::vector<size_t> &, const std::vector<size_t> &, double *);
