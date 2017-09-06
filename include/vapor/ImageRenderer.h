#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

#include <GL/glew.h>

#ifdef Darwin
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include <vapor/TwoDRenderer.h>
#include <vapor/Visualizer.h>
#include <vapor/RenderParams.h>
#include <vapor/ShaderMgr.h>
#include <vapor/DataMgr.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/GeoImage.h>
#include <vapor/StructuredGrid.h>
#include <vapor/utils.h>

namespace VAPoR {

class RENDER_API ImageRenderer : public TwoDRenderer {
public:
    // ImageRenderer(Visualizer *w, RenderParams* rp, ShaderMgr *sm);
    ImageRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr);

    virtual ~ImageRenderer();

    static std::string GetClassType() { return ("ImageRenderer"); }

    /*static Renderer* CreateInstance( Visualizer* v, RenderParams* rp, ShaderMgr *sm)
  {
    return new ImageRenderer(v,rp,sm);
  }*/

protected:
    int GetMesh(DataMgr *dataMgr, GLfloat **verts, GLfloat **normals, GLsizei &width, GLsizei &height, GLuint **indices, GLsizei &nindices, bool &structuredMesh);

    const GLvoid *GetTexture(DataMgr *dataMgr, GLsizei &width, GLsizei &height, GLint &internalFormat, GLenum &format, GLenum &type, size_t &texelSize, bool &gridAligned);

    virtual GLuint GetAttribIndex() const { return 0; }

private:
    GeoImage *     _geoImage;
    unsigned char *_twoDTex;
    string         _cacheImgFileName;
    vector<double> _cacheTimes;
    vector<double> _pcsExtentsData;
    double         _pcsExtentsImg[4];
    string         _proj4StringImg;
    GLsizei        _texWidth;
    GLsizei        _texHeight;
    size_t         _cacheTimestep;
    int            _cacheRefLevel;
    int            _cacheLod;
    vector<double> _cacheBoxExtents;
    size_t         _cacheTimestepTex;
    string         _cacheHgtVar;
    int            _cacheGeoreferenced;
    vector<double> _cacheBoxExtentsTex;
    SmartBuf       _sb_verts;
    SmartBuf       _sb_normals;
    SmartBuf       _sb_indices;
    GLsizei        _vertsWidth;
    GLsizei        _vertsHeight;

    unsigned char *_getTexture(DataMgr *dataMgr);

    bool _gridStateDirty() const;

    void _gridStateClear();

    void _gridStateSet();

    bool _imageStateDirty(const vector<double> &times) const;

    void _imageStateSet(const vector<double> &times);

    void _imageStateClear();

    bool _texStateDirty(DataMgr *dataMgr) const;

    void _texStateSet(DataMgr *dataMgr);

    void _texStateClear();

    int _reinit(string path, vector<double> times);

    unsigned char *_getImage(GeoImage *geoimage, size_t ts, string proj4StringData, vector<double> pcsExtentsDataVec, double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
                             GLsizei &width, GLsizei &height) const;

    int _getMeshDisplacedGeo(DataMgr *dataMgr, StructuredGrid *hgtGrid, GLsizei width, GLsizei height, double defaultZ);

    // Compute _verts  for displayed, non-georeferenced image
    //
    int _getMeshDisplacedNoGeo(DataMgr *dataMgr, StructuredGrid *hgtGrid, GLsizei width, GLsizei height, const vector<double> &minExt, const vector<double> &maxExt);

    int _getMeshDisplaced(DataMgr *dataMgr, GLsizei width, GLsizei height, const vector<double> &minBox, const vector<double> &maxBox);

    int _getMeshPlane(const vector<double> &minBox, const vector<double> &maxBox);

    // Get the selected horizontal ROI in PCS data coordinates
    //
    vector<double> _getPCSExtentsData() const;

    // Transform verts from absolute to local coordinates
    //
    void _transformToLocal(size_t width, size_t height, const vector<double> &scaleFac) const;
};
};    // namespace VAPoR

#endif    // TWODRENDERER_H
