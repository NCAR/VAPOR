#pragma once
#include <vapor/MyBase.h>

namespace VAPoR {
class RENDER_API VisualizerGLContextManager {
public:
    virtual void Activate(const string &visualizerName) = 0;
};
}
