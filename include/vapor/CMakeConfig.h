#ifndef CMAKECONFIG_H
#define CMAKECONFIG_H

#include <string>

#define CMakeConfigStringType std::string

extern const int MAJOR;
extern const int MINOR;
extern const int MICRO;

extern const CMakeConfigStringType VERSION_RC;
extern const CMakeConfigStringType VERSION_DATE;
extern const CMakeConfigStringType VERSION_COMMIT;
extern const CMakeConfigStringType VERSION_STRING;
extern const CMakeConfigStringType VERSION_STRING_FULL;

extern const CMakeConfigStringType BUILD_TYPE;
extern const CMakeConfigStringType SOURCE_DIR;
extern const CMakeConfigStringType THIRD_PARTY_DIR;

extern const CMakeConfigStringType PYTHON_VERSION;
extern const CMakeConfigStringType PYTHON_DIR;
extern const CMakeConfigStringType PYTHON_PATH;

#endif
