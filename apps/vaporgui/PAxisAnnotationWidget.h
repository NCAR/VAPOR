#pragma once

#include "PWidget.h"
#include <vector>

class V3DInput;
class V3DIntInput;
class VGroup;

namespace VAPoR {
class ControlExec;
}

class PAxisAnnotationWidget : public PWidget {
    VAPoR::ControlExec *_controlExec;
    VGroup *            _group;
    V3DIntInput *       _numTics;
    V3DInput *          _size;
    V3DInput *          _min;
    V3DInput *          _max;
    V3DInput *          _origin;

public:
    PAxisAnnotationWidget(VAPoR::ControlExec *controlExec);

protected:
    void updateGUI() const override;

private:
    void _numTicsChanged(const std::vector<int> xyz);
    void _sizeChanged(const std::vector<double> xyz);
    void _minChanged(const std::vector<double> xyz);
    void _maxChanged(const std::vector<double> xyz);
    void _originChanged(const std::vector<double> xyz);

    std::vector<double> _getDomainExtents() const;
    void                _convertPCSToLonLat(double &xCoord, double &yCoord) const;
    void                _convertLonLatToPCS(double &xCoord, double &yCoord) const;
    void                _scaleNormalizedCoordsToWorld(std::vector<double> &coords) const;
    void                _scaleWorldCoordsToNormalized(std::vector<double> &coords) const;
};
