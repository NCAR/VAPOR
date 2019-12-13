//
// ====================================
// VGeometry combines two or three VRanges,
// representing a 2D or 3D geometry.
// Note: this class is never supposed to be used beyond 2D and 3D cases.
// ====================================
//
class VGeometry : public QWidget {
    Q_OBJECT

public:
    /* Constructor for 2D or 3D geometries.
       Four floating point values imply a 2D geometry, while Six floating point
       values imply a 3D geometry. All other numbers are illegal. */
    VGeometry(QWidget *parent, int dim, const std::vector<float> &range);

    /* Adjust the dimension and/or value ranges through this function.
       Argument range must contain 4 or 6 values organized in the following order:
       xmin, xmax, ymin, ymax, (zmin, zmax).                                    */
    void SetDimAndRange(int dim, const std::vector<float> &range);
    /* The number of incoming values MUST match the current dimensionality.
       I.e., 4 values for 2D widgets, and 6 values for 3D widgets.       */
    void SetCurrentValues(const std::vector<float> &vals);
    void GetCurrentValues(std::vector<float> &vals) const;

signals:
    void _geometryChanged();

private slots:
    void _respondChanges();

private:
    int          _dim;
    VRange *     _xrange, *_yrange, *_zrange;
    QVBoxLayout *_layout;
};
