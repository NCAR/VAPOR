/**
 * Class Name: QSinglePoint
 *
 * Purpose: Represent and manupulate a single point in 2D, 3D, or 4D space.
 */

class QSinglePoint : public : QWidget {
public:
    /// Constructor
    QSinglePoint(QWidget *parent = 0);

    /// Destructor
    ~QSinglePoint();

    /// Set Dimensionality
    ///   Note: Valid values are 2, 3, and 4.
    void SetDimensionality(int dim);

    /// Set Spatial Extents
    ///   Note: extents.size() == _dimensionality * 2
    void SetSpatialExtents(std::vector<double> &min, std::vector<double> &max);

    /// Get Current Point Coordinates
    ///   Note: point.size() == _dimensionality
    void GetCurrentPoint(std::vector<double> &point);

signals:
    void PointChanged();

private:
    int                       _dimensionality;
    std::vector<QSlider *>    _sliders;      /// _sliders.size() == _dimensionality
    std::vector<QLineEdits *> _lineEdits;    /// _lineEdits.size() == _dimensionality
};
