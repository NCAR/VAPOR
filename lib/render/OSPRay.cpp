#include <vapor/OSPRay.h>

OSPError OSPInitStatus = OSP_UNKNOWN_ERROR;
const char *OSPInitStatusMessage = "";

bool OSPRayInitialized()
{
    return OSPInitStatus == OSP_NO_ERROR;
}
