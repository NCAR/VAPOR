//************************************************************************
//									*
//			 Copyright (C)  2004				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Renderer.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September 2004
//
//	Description:	Implements the Renderer class.
//		A pure virtual class that is implemented for each renderer.
//		Methods are called by the Visualizer class as needed.
//
#include <cfloat>
#include <climits>
#include <limits>
#include <iomanip>
#include <sstream>

#include <vapor/glutil.h> // Must be included first!!!
#include <vapor/Renderer.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/GetAppPath.h>

#include <vapor/ViewpointParams.h>

#ifdef Darwin
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

using namespace VAPoR;
const int Renderer::_imgWid = 256;
const int Renderer::_imgHgt = 256;

Renderer::Renderer(
    const ParamsMgr *pm, string winName, string dataSetName, string paramsType,
    string classType, string instName, DataMgr *dataMgr) : RendererBase(pm, winName, dataSetName, paramsType, classType, instName, dataMgr) {
    //Establish the data sources for the rendering:
    //

    _colorbarTexture = 0;
    _timestep = 0;
    _textObject = NULL;

    vector<string> fpath;
    fpath.push_back("fonts");
    _fontFile = GetAppPath("VAPOR", "share", fpath);
    _fontFile = _fontFile + "//arimo.ttf";
}

RendererBase::RendererBase(
    const ParamsMgr *pm, string winName, string dataSetName, string paramsType,
    string classType, string instName, DataMgr *dataMgr) {
    //Establish the data sources for the rendering:
    //
    _paramsMgr = pm;
    _winName = winName;
    _dataSetName = dataSetName;
    _paramsType = paramsType;
    _classType = classType;
    _instName = instName;
    _dataMgr = dataMgr;

    _shaderMgr = NULL;
    _glInitialized = false;
}
// Destructor
RendererBase::~RendererBase() {
}
// Destructor
Renderer::~Renderer() {
    if (_colorbarTexture)
        delete _colorbarTexture;
}

int RendererBase::initializeGL(ShaderMgr *sm) {
    _shaderMgr = sm;
    int rc = _initializeGL();
    if (rc < 0) {
        return (rc);
    }

    vector<int> status;
    bool ok = oglStatusOK(status);
    if (!ok) {
        SetErrMsg("OpenGL error : %s", oglGetErrMsg(status).c_str());
        return (-1);
    }

    _glInitialized = true;

    return (0);
}

double Renderer::_getDefaultZ(
    DataMgr *dataMgr, size_t ts) const {

    vector<double> minExts;
    vector<double> maxExts;

    bool status = DataMgrUtils::GetExtents(dataMgr, ts, "", minExts, maxExts);
    assert(status);

    return (minExts.size() == 3 ? minExts[2] : 0.0);
}

int Renderer::paintGL() {
    const RenderParams *rParams = GetActiveParams();

    if (!rParams->IsEnabled())
        return (0);

    _timestep = rParams->GetCurrentTimestep();

    vector<double> translate = rParams->GetTransform()->GetTranslations();
    vector<double> rotate = rParams->GetTransform()->GetRotations();
    vector<double> scale = rParams->GetTransform()->GetScales();
    vector<double> origin = rParams->GetTransform()->GetOrigin();
    assert(translate.size() == 3);
    assert(rotate.size() == 3);
    assert(scale.size() == 3);
    assert(origin.size() == 3);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(-origin[0], -origin[1], -origin[2]);
    glScalef(scale[0], scale[1], scale[2]);
    glRotatef(rotate[0], 1, 0, 0);
    glRotatef(rotate[1], 0, 1, 0);
    glRotatef(rotate[2], 0, 0, 1);
    glTranslatef(origin[0], origin[1], origin[2]);

    glTranslatef(translate[0], translate[1], translate[2]);

    int rc = _paintGL();

    glPopMatrix();

    if (rc < 0) {
        return (-1);
    }

    vector<int> status;
    bool ok = oglStatusOK(status);
    if (!ok) {
        SetErrMsg("OpenGL error : %s", oglGetErrMsg(status).c_str());
        return (-1);
    }
    return (0);
}

void Renderer::EnableClipToBox() const {

    GLdouble x0Plane[] = {1.0, 0.0, 0.0, 0.0};
    GLdouble x1Plane[] = {-1.0, 0.0, 0.0, 0.0};
    GLdouble y0Plane[] = {0.0, 1.0, 0.0, 0.0};
    GLdouble y1Plane[] = {0.0, -1.0, 0.0, 0.0};
    GLdouble z0Plane[] = {0.0, 0.0, 1.0, 0.0};
    GLdouble z1Plane[] = {0.0, 0.0, -1.0, 0.0}; //z largest

    const RenderParams *rParams = GetActiveParams();
    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);
    assert(minExts.size() == maxExts.size());
    assert(minExts.size() > 0 && minExts.size() < 4);

    int orientation = rParams->GetBox()->GetOrientation();

    if (minExts.size() == 3 || orientation != 0) {
        x0Plane[3] = -minExts[0];
        x1Plane[3] = maxExts[0];
        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, x0Plane);
        glEnable(GL_CLIP_PLANE1);
        glClipPlane(GL_CLIP_PLANE1, x1Plane);
    }

    if (minExts.size() == 3 || orientation != 1) {
        y0Plane[3] = -minExts[1];
        y1Plane[3] = maxExts[1];
        glEnable(GL_CLIP_PLANE2);
        glClipPlane(GL_CLIP_PLANE2, y0Plane);
        glEnable(GL_CLIP_PLANE3);
        glClipPlane(GL_CLIP_PLANE3, y1Plane);
    }

    if (minExts.size() == 3 || orientation != 2) {
        z0Plane[3] = -minExts[2];
        z1Plane[3] = maxExts[2];
        glEnable(GL_CLIP_PLANE4);
        glClipPlane(GL_CLIP_PLANE4, z0Plane);
        glEnable(GL_CLIP_PLANE5);
        glClipPlane(GL_CLIP_PLANE5, z1Plane);
    }
    cout << endl;
}

void Renderer::DisableClippingPlanes() {
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
    glDisable(GL_CLIP_PLANE4);
    glDisable(GL_CLIP_PLANE5);
}

#ifdef DEAD
void Renderer::buildLocal2DTransform(int dataOrientation, float a[2], float b[2], float *constVal, int mappedDims[3]) {

    mappedDims[2] = dataOrientation;
    mappedDims[0] = (dataOrientation == 0) ? 1 : 0; // x or y
    mappedDims[1] = (dataOrientation < 2) ? 2 : 1;  // z or y
    const RenderParams *rParams = GetActiveParams();

    const vector<double> &exts = rParams->GetBox()->GetLocalExtents();
    *constVal = exts[dataOrientation];
    //constant terms go to middle
    b[0] = 0.5 * (exts[mappedDims[0]] + exts[3 + mappedDims[0]]);
    b[1] = 0.5 * (exts[mappedDims[1]] + exts[3 + mappedDims[1]]);
    //linear terms send -1,1 to box min,max
    a[0] = b[0] - exts[mappedDims[0]];
    a[1] = b[1] - exts[mappedDims[1]];
}

void Renderer::getLocalContainingRegion(float regMin[3], float regMax[3]) {
    //Determine the smallest axis-aligned cube that contains the rotated box local coordinates.  This is
    //obtained by mapping all 8 corners into the space.

    double transformMatrix[12];
    //Set up to transform from probe (coords [-1,1]) into volume:
    GetActiveParams()->GetBox()->buildLocalCoordTransform(transformMatrix, 0.f, -1);
    const double *sizes = _dataStatus->getFullSizes();

    //Calculate the normal vector to the probe plane:
    double zdir[3] = {0.f, 0.f, 1.f};
    double normEnd[3]; //This will be the unit normal
    double normBeg[3];
    double zeroVec[3] = {0.f, 0.f, 0.f};
    vtransform(zdir, transformMatrix, normEnd);
    vtransform(zeroVec, transformMatrix, normBeg);
    vsub(normEnd, normBeg, normEnd);
    vnormal(normEnd);

    //Start by initializing extents, and variables that will be min,max
    for (int i = 0; i < 3; i++) {
        regMin[i] = FLT_MAX;
        regMax[i] = -FLT_MAX;
    }

    for (int corner = 0; corner < 8; corner++) {
        int intCoord[3];
        double startVec[3], resultVec[3];
        intCoord[0] = corner % 2;
        intCoord[1] = (corner / 2) % 2;
        intCoord[2] = (corner / 4) % 2;
        for (int i = 0; i < 3; i++)
            startVec[i] = -1.f + (float)(2.f * intCoord[i]);
        // calculate the mapping of this corner,
        vtransform(startVec, transformMatrix, resultVec);
        // force mapped corner to lie in the local extents
        //and then force box to contain the corner:
        for (int i = 0; i < 3; i++) {
            //force to lie in domain
            if (resultVec[i] < 0.)
                resultVec[i] = 0.;
            if (resultVec[i] > sizes[i])
                resultVec[i] = sizes[i];

            if (resultVec[i] < regMin[i])
                regMin[i] = resultVec[i];
            if (resultVec[i] > regMax[i])
                regMax[i] = resultVec[i];
        }
    }
    return;
}
#endif

int Renderer::makeColorbarTexture() {

    if (_colorbarTexture)
        delete _colorbarTexture;

    RenderParams *rParams = GetActiveParams();
    ColorbarPbase *cbpb = rParams->GetColorbarPbase();
    if (!cbpb)
        return -1;

    MapperFunction *mf = rParams->GetMapperFunc(rParams->GetVariableName());
    if (!mf)
        return -1;

    _colorbarTexture = new unsigned char[_imgWid * _imgHgt * 3];

    //Fill with background color
    vector<double> bgc = cbpb->GetBackgroundColor();
    unsigned char bgr = (unsigned char)(bgc[0] * 255);
    unsigned char bgg = (unsigned char)(bgc[1] * 255);
    unsigned char bgb = (unsigned char)(bgc[2] * 255);
    for (int i = 0; i < _imgWid; i++) {
        for (int j = 0; j < _imgHgt; j++) {

            _colorbarTexture[3 * (j + _imgHgt * i)] = bgr;
            _colorbarTexture[1 + 3 * (j + _imgHgt * i)] = bgg;
            _colorbarTexture[2 + 3 * (j + _imgHgt * i)] = bgb;
        }
    }

    //determine foreground color:
    unsigned char fgr = 255 - bgr;
    unsigned char fgg = 255 - bgg;
    unsigned char fgb = 255 - bgb;

    vector<double> colorbarSize = cbpb->GetSize();

    // determine horizontal and vertical line widths in pixels (.02 times image size?)
    //int lineWidth = (int)(0.02*Min(_imgHgt, _imgWid)+0.5);
    int lineWidth = 2;

    //Draw top and bottom
    for (int i = 0; i < _imgWid; i++) {
        for (int j = 0; j < lineWidth; j++) {
            _colorbarTexture[3 * (i + _imgWid * j)] = fgr;
            _colorbarTexture[1 + 3 * (i + _imgWid * j)] = fgg;
            _colorbarTexture[2 + 3 * (i + _imgWid * j)] = fgb;
            _colorbarTexture[3 * (i + _imgWid * (_imgHgt - j - 1))] = fgr;
            _colorbarTexture[1 + 3 * (i + _imgWid * (_imgHgt - j - 1))] = fgg;
            _colorbarTexture[2 + 3 * (i + _imgWid * (_imgHgt - j - 1))] = fgb;
        }
    }
    //sides:
    for (int j = 0; j < _imgHgt; j++) {
        for (int i = 0; i < lineWidth; i++) {
            _colorbarTexture[3 * (i + _imgWid * j)] = fgr;
            _colorbarTexture[1 + 3 * (i + _imgWid * j)] = fgg;
            _colorbarTexture[2 + 3 * (i + _imgWid * j)] = fgb;
            _colorbarTexture[3 * (_imgWid - i - 1 + _imgWid * j)] = fgr;
            _colorbarTexture[1 + 3 * (_imgWid - i - 1 + _imgWid * j)] = fgg;
            _colorbarTexture[2 + 3 * (_imgWid - i - 1 + _imgWid * j)] = fgb;
        }
    }
    //Draw tics
    int numtics = cbpb->GetNumTicks();
    for (int tic = 0; tic < numtics; tic++) {
        int ticPos = tic * (_imgHgt / numtics) + (_imgHgt / (2 * numtics));
        //Draw a horizontal line from .37 to .45 width
        //for (int i = (int)(_imgWid*.35); i<= (int)( _imgWid*.45); i++){
        int i, j;
        for (i = (int)(_imgWid * .37); i <= (int)(_imgWid * .45); i++) {
            for (j = ticPos - lineWidth / 2; j <= ticPos + lineWidth / 2; j++) {
                _colorbarTexture[3 * (i + _imgWid * j)] = fgr;
                _colorbarTexture[1 + 3 * (i + _imgWid * j)] = fgg;
                _colorbarTexture[2 + 3 * (i + _imgWid * j)] = fgb;
            }
        }
    }

    //Draw colors
    //With no tics, use the whole scale
    if (numtics == 0)
        numtics = 1000;
    double A = (mf->getMaxMapValue() - mf->getMinMapValue()) * (double)(numtics) /
               ((double)(1. - numtics) * (double)_imgHgt);
    double B = mf->getMaxMapValue() - A * (double)_imgHgt * .5 / (double)(numtics);

    //for (int line = _imgHgt-2*lineWidth; line>=2*lineWidth; line--){
    for (int line = _imgHgt - 2 * lineWidth - 5; line > 2 * lineWidth + 5; line--) {
        float ycoord = A * (float)line + B;

        float rgb[3];
        unsigned char rgbByte;
        mf->rgbValue(ycoord, rgb);

        for (int col = 2 * lineWidth; col < (int)(_imgWid * .35); col++) {
            for (int k = 0; k < 3; k++) {
                rgbByte = (unsigned char)(255 * rgb[k]);
                _colorbarTexture[k + 3 * (col + _imgWid * line)] = rgbByte;
            }
        }
    }
    return 0;
}

void Renderer::renderColorbar() {

    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];

    float whitecolor[4] = {1., 1., 1., 1.f};
    const RenderParams *rParams = GetActiveParams();
    ColorbarPbase *cbpb = rParams->GetColorbarPbase();
    if (!cbpb || !cbpb->IsEnabled())
        return;
    if (makeColorbarTexture())
        return;

    glColor4fv(whitecolor);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //Disable z-buffer compare, always overwrite:
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_TEXTURE_2D);

    //create a polygon appropriately positioned in the scene.  It's inside the unit cube--
    glTexImage2D(GL_TEXTURE_2D, 0, 3, _imgWid, _imgHgt, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 _colorbarTexture);

    vector<double> cornerPosn = cbpb->GetCornerPosition();
    vector<double> cbsize = cbpb->GetSize();

    //Note that GL reverses y coordinates
    float llx = 2. * cornerPosn[0] - 1.;
    float lly = 2. * (cornerPosn[1] + cbsize[1]) - 1.;
    float urx = 2. * (cornerPosn[0] + cbsize[0]) - 1.;
    float ury = 2. * cornerPosn[1] - 1.;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(llx, lly, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(llx, ury, 0.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(urx, ury, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(urx, lly, 0.0f);

    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Draw numeric text annotation
    //
    renderColorbarText(cbpb, fbWidth, fbHeight, llx, lly, urx, ury);

    glDepthFunc(GL_LESS);
}

void Renderer::renderColorbarText(ColorbarPbase *cbpb,
                                  float fbWidth, float fbHeight,
                                  float llx, float lly, float urx, float ury) {

    RenderParams *rParams = GetActiveParams();
    MapperFunction *mf = rParams->GetMapperFunc(rParams->GetVariableName());
    float numEntries = mf->getNumEntries();

    vector<double> bgc = cbpb->GetBackgroundColor();

    //determine text color; the compliment of the background color:
    //
    float fgr = 1.f - bgc[0];
    float fgg = 1.f - bgc[1];
    float fgb = 1.f - bgc[2];

    float txtColor[] = {fgr, fgg, fgb, 1.};
    float bgColor[] = {(float)bgc[0], (float)bgc[1], (float)bgc[2], 0.};
    int precision = (int)cbpb->GetNumDigits();
    float dummy[] = {0., 0., 0.}; // Dummy coordinates.  We won't know the correct
                                  // coords until we know image size.

    // Corners in texture coordinates, to be derived later
    //
    float Trx, Tlx, Tly, Tuy;

    float maxWidth = 0;
    int numtics = cbpb->GetNumTicks();
    int fontSize = cbpb->GetFontSize();
    for (int tic = 0; tic < numtics; tic++) {
        // Find numeric data value associated with our current text label
        //
        int mapIndex = (1.f / ((float)numtics * 2.f) + (float)tic / (float)numtics) * numEntries;
        float textValue = mf->mapIndexToFloat(mapIndex);
        stringstream ss;
        ss << fixed << setprecision(precision) << textValue;
        string textString = ss.str();

        // Generate our text object so we can use proper width/height
        // values when calculating offsets
        //
        if (_textObject != NULL) {
            delete _textObject;
            _textObject = NULL;
        }
        _textObject = new TextObject();
        //_textObject->Initialize("/Users/pearse/Downloads/pacifico/Pacifico.ttf",
        _textObject->Initialize(_fontFile,
                                textString, fontSize, dummy, 0, txtColor, bgColor);
        float texWidth = _textObject->getWidth();
        float texHeight = _textObject->getHeight();

        // llx and lly are in visualizer coordinates between -1 and 1
        // TextRenderer takes pixel coordinates. Trx and Tuy are the
        // TextRenderer coords of the colorbar in the pixel coord system.
        //
        Trx = (urx + 1) * fbWidth / 2.f;  // right
        Tlx = (llx + 1) * fbWidth / 2.f;  // left
        Tly = (lly + 1) * fbHeight / 2.f; // lower
        Tuy = (ury + 1) * fbHeight / 2.f; // upper

        // Start at lower left corner
        //
        float Tx = Tlx;
        float Ty = Tuy;

        // Calculate Y offset to align with tick marks
        //
        Ty -= texHeight / 2.f; // Center the text vertically, at the bottom of the colorbar
        float ticOffset = (Tly - Tuy) * 1.f / (float)numtics;
        Ty += ticOffset / 2.f; // Initial offset from bottom of colorbar
        Ty += ticOffset * tic; // Offset applied per tick mark

        // Calculate X offset to allign with tick marks
        //
        float cbWidth = Trx - Tlx;
        float rightShift = cbWidth * .5;
        Tx += rightShift;

        // Now that we know the x offset, check if the colorbar box is big enough
        // to contain the text, and resize the colorbar if not
        //
        if (texWidth > maxWidth) {
            maxWidth = texWidth;
        }
        vector<double> size = cbpb->GetSize();
        // size[0]/2 is the space we have to place text into
        //
        if (size[0] / 2.f < maxWidth / fbWidth) {
            size[0] = 2 * maxWidth / fbWidth;
            cbpb->SetSize(size);
        }

        float inCoords[] = {Tx, Ty, 0};

        _textObject->drawMe(inCoords);
    }

    // Render colorbar title, if any
    //
    string title = cbpb->GetTitle();
    if (title != "") {
        if (_textObject != NULL) {
            delete _textObject;
            _textObject = NULL;
        }
        txtColor[0] = bgColor[0];
        txtColor[1] = bgColor[1];
        txtColor[2] = bgColor[2];
        txtColor[3] = 1.f;
        _textObject = new TextObject();
        _textObject->Initialize("/Users/pearse/Downloads/pacifico/Pacifico.ttf",
                                title, 20, dummy, 0, txtColor, bgColor);
        Tuy -= _textObject->getHeight();
        float coords[] = {Tlx, Tuy, 0.f};
        _textObject->drawMe(coords);
    }

    if (_textObject != NULL) {
        delete _textObject;
        _textObject = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
//
// RendererFactory Class
//
/////////////////////////////////////////////////////////////////////////

RendererFactory *RendererFactory::Instance() {
    static RendererFactory instance;
    return &instance;
}

void RendererFactory::RegisterFactoryFunction(
    string myName, string myParamsName,
    function<Renderer *(
        const ParamsMgr *, string, string, string, string, DataMgr *)>
        classFactoryFunction) {

    // register the class factory function
    _factoryFunctionRegistry[myName] = classFactoryFunction;
    _factoryMapRegistry[myName] = myParamsName;
}

Renderer *RendererFactory::CreateInstance(
    const ParamsMgr *pm, string winName, string dataSetName,
    string classType, string instName, DataMgr *dataMgr) {
    Renderer *instance = NULL;

    // find classType in the registry and call factory method.
    //
    auto it = _factoryFunctionRegistry.find(classType);
    if (it != _factoryFunctionRegistry.end()) {
        instance = it->second(
            pm, winName, dataSetName, classType, instName, dataMgr);
    }

    if (instance != NULL)
        return instance;
    else
        return NULL;
}

string RendererFactory::GetRenderClassFromParamsClass(
    string paramsClass) const {

    map<string, string>::const_iterator itr;
    for (itr = _factoryMapRegistry.begin(); itr != _factoryMapRegistry.end(); ++itr) {
        if (itr->second == paramsClass)
            return (itr->first);
    }
    return ("");
}

string RendererFactory::GetParamsClassFromRenderClass(
    string renderClass) const {

    map<string, string>::const_iterator itr;
    for (itr = _factoryMapRegistry.begin(); itr != _factoryMapRegistry.end(); ++itr) {
        if (itr->first == renderClass)
            return (itr->second);
    }
    return ("");
}

vector<string> RendererFactory::GetFactoryNames() const {
    vector<string> names;

    map<string, function<Renderer *(
                    const ParamsMgr *, string, string, string, string, DataMgr *)>>::const_iterator itr;

    for (
        itr = _factoryFunctionRegistry.begin();
        itr != _factoryFunctionRegistry.end();
        ++itr) {
        names.push_back(itr->first);
    }
    return (names);
}

RendererFactory::RendererFactory() {}
RendererFactory::RendererFactory(const RendererFactory &) {}
RendererFactory &RendererFactory::operator=(const RendererFactory &) { return *this; }
