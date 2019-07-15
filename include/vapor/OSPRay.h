#pragma once

#include <ospray/ospray.h>
#include <vapor/common.h>

extern RENDER_API OSPError OSPInitStatus;
extern RENDER_API const char *OSPInitStatusMessage;

bool RENDER_API OSPRayInitialized();
