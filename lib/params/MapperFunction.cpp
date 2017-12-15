//************************************************************************
//									*
//		     Copyright (C)  2005				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		MapperFunction.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		August 2005
//
//	Description:	Implements the MapperFunction class
//		Provides separate mapping of color and opacity with separate domain
//		bounds.
//
//
#ifdef WIN32
    #pragma warning(disable : 4251 4100)
#endif
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <algorithm>
#include <vapor/MapperFunction.h>
#include <vapor/ColorMap.h>
#include <vapor/XmlNode.h>
#include <vapor/ParamsBase.h>

using namespace VAPoR;
using namespace Wasp;

const string MapperFunction::_dataBoundsTag = "DataBounds";
const string MapperFunction::_opacityCompositionTag = "OpacityComposition";
const string MapperFunction::_opacityScaleTag = "OpacityScale";
const string MapperFunction::_opacityMapsTag = "OpacityMaps";
const string MapperFunction::_opacityMapTag = "OpacityMap";
const string MapperFunction::_autoUpdateHistoTag = "AutoUpdateHisto";
const string MapperFunction::_secondaryVarMapperTag = "SecondaryVarMapper";

//
// Register class with object factory!!!
//
static ParamsRegistrar<MapperFunction> registrar(MapperFunction::GetClassType());

//----------------------------------------------------------------------------
// Constructor for empty, default Mapper function
//----------------------------------------------------------------------------

MapperFunction::MapperFunction(ParamsBase::StateSave *ssave) : ParamsBase(ssave, MapperFunction::GetClassType()), _numEntries(256)
{
    m_colorMap = NULL;
    m_opacityMaps = NULL;

    setOpacityScale(1.0);
    setOpacityComposition(ADDITION);

    m_colorMap = new ColorMap(ssave);
    m_colorMap->SetParent(this);

    // Create a Params container for multiple opacity maps
    //
    m_opacityMaps = new ParamsContainer(ssave, _opacityMapsTag);
    m_opacityMaps->SetParent(this);

    // Populate container with a single default opacity map
    //
    OpacityMap opacityMap(ssave);

    m_opacityMaps->Insert(&opacityMap, _make_omap_name(0));

    setMinMaxMapValue(1., -1.);
}

MapperFunction::MapperFunction(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node), _numEntries(256)
{
    m_colorMap = NULL;
    m_opacityMaps = NULL;

    if (node->HasChild(ColorMap::GetClassType())) {
        m_colorMap = new ColorMap(ssave, node->GetChild(ColorMap::GetClassType()));
    } else {
        // Node doesn't contain a colormap
        //
        m_colorMap = new ColorMap(ssave);
        m_colorMap->SetParent(this);
    }

    if (node->HasChild(_opacityMapsTag)) {
        m_opacityMaps = new ParamsContainer(ssave, node->GetChild(_opacityMapsTag));
    } else {
        // Node doesn't contain a opacity map
        //
        m_opacityMaps = new ParamsContainer(ssave, _opacityMapsTag);
        m_opacityMaps->SetParent(this);

        // Populate container with a single default opacity map
        //
        OpacityMap opacityMap(ssave);

        m_opacityMaps->Insert(&opacityMap, _make_omap_name(0));
    }
}

MapperFunction::MapperFunction(const MapperFunction &rhs) : ParamsBase(rhs), _numEntries(256)
{
    m_colorMap = NULL;
    m_opacityMaps = NULL;

    m_colorMap = new ColorMap(*(rhs.m_colorMap));
    m_colorMap->SetParent(this);

    m_opacityMaps = new ParamsContainer(*(rhs.m_opacityMaps));
    m_opacityMaps->SetParent(this);
}

MapperFunction &MapperFunction::operator=(const MapperFunction &rhs)
{
    if (m_colorMap) delete m_colorMap;

    if (m_opacityMaps) delete m_opacityMaps;

    ParamsBase::operator=(rhs);

    m_colorMap = NULL;
    m_opacityMaps = NULL;

    m_colorMap = new ColorMap(rhs._ssave, rhs.m_colorMap->GetNode());
    m_colorMap->SetParent(this);

    m_opacityMaps = new ParamsContainer(rhs._ssave, rhs.m_opacityMaps->GetNode());
    m_opacityMaps->SetParent(this);

    return (*this);
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
MapperFunction::~MapperFunction()
{
    MyBase::SetDiagMsg("MapperFunction::~MapperFunction()");

    if (m_colorMap) delete m_colorMap;

    if (m_opacityMaps) delete m_opacityMaps;
}

//----------------------------------------------------------------------------
// Create a mapper function by parsing a file.
//----------------------------------------------------------------------------
int MapperFunction::LoadFromFile(string path, vector<double> defaultDataBounds)
{
    XmlParser xmlparser;

    XmlNode *node = new XmlNode();

    int rc = xmlparser.LoadFromFile(node, path);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to read file %s : %M", path.c_str());
        return (-1);
    }

    // Create a new MapperFunction from the XmlNode tree
    //
    XmlNode *       parent = this->GetNode()->GetParent();
    MapperFunction *newTF = new MapperFunction(_ssave, node);

    if (newTF->getMinMapValue() == 0 && newTF->getMaxMapValue() == 0) { newTF->setMinMaxMapValue(defaultDataBounds[0], defaultDataBounds[1]); }

    // Assign (copy) new TF to this object
    //
    *this = *newTF;
    this->GetNode()->SetParent(parent);

    // Delete the new tree
    //
    delete newTF;

    return (0);
}

//----------------------------------------------------------------------------
// Save the mapper function to a file.
//----------------------------------------------------------------------------
int MapperFunction::SaveToFile(string path)
{
    ofstream out(path);
    if (!out) {
        MyBase::SetErrMsg("Failed to open file %s : %M", path.c_str());
        return (-1);
    }

    XmlNode *node = GetNode();

    out << *node;

    if (out.bad()) {
        MyBase::SetErrMsg("Failed to write file %s : %M", path.c_str());
        return (-1);
    }
    out.close();

    return (0);
}

//----------------------------------------------------------------------------
// Calculate the opacity given the data value
//----------------------------------------------------------------------------
float MapperFunction::getOpacityValueData(float value) const
{
    int count = 0;

    //
    // Using the square of the opacity scaling factor gives
    // better control over low opacity values.
    // But this correction will be made in the GUI
    //
    float opacScale = getOpacityScale();

    float opacity = 0.0;

    if (getOpacityComposition() == MULTIPLICATION) { opacity = 1.0; }

    for (int i = 0; i < getNumOpacityMaps(); i++) {
        OpacityMap *omap = GetOpacityMap(i);

        if (omap->IsEnabled() && omap->inDataBounds(value)) {
            if (getOpacityComposition() == ADDITION) {
                opacity += omap->opacityData(value);
            } else    // _compType == MULTIPLICATION
            {
                opacity *= omap->opacityData(value);
            }

            count++;

            if (opacity * opacScale > 1.0) { return 1.0; }
        }
    }

    if (count) return opacity * opacScale;

    return 0.0;
}

//----------------------------------------------------------------------------
// Calculate the color given the data value
//----------------------------------------------------------------------------
void MapperFunction::hsvValue(float value, float *h, float *s, float *v) const
{
    ColorMap *cmap = GetColorMap();
    if (cmap) {
        ColorMap::Color color = cmap->color(value);

        *h = color.hue();
        *s = color.sat();
        *v = color.val();
    }
}

//----------------------------------------------------------------------------
// Populate at a RGBA lookup table
//----------------------------------------------------------------------------
void MapperFunction::makeLut(float *clut) const
{
    float step = (getMaxMapValue() - getMinMapValue()) / (_numEntries - 1);

    for (int i = 0; i < _numEntries; i++) {
        float v = getMinMapValue() + i * step;

        ColorMap *cmap = GetColorMap();
        cmap->color(v).toRGB(&clut[4 * i]);
        clut[4 * i + 3] = getOpacityValueData(v);
    }
}

//! Set both minimum and maximum mapping (histo) values
//! \param[in] val1 minimum value
//! \param[in] val2 maximum value
void MapperFunction::setMinMaxMapValue(float val1, float val2)
{
    if (val1 > val2) val2 = val1;

    vector<double> bnds;
    bnds.push_back(val1);
    bnds.push_back(val2);

    SetValueDoubleVec(_dataBoundsTag, "Set min max map value", bnds);

    // Need to set the bounds of color and opacity maps
    //
    ColorMap *cmap = GetColorMap();
    if (cmap) cmap->SetDataBounds(bnds);

    for (int i = 0; i < getNumOpacityMaps(); i++) { GetOpacityMap(i)->SetDataBounds(bnds); }
}

vector<double> MapperFunction::getMinMaxMapValue() const
{
    vector<double> defaultv(2, 0.0);

    vector<double> bounds = GetValueDoubleVec(_dataBoundsTag, defaultv);
    if (bounds.size() != 2) bounds = defaultv;
    return (bounds);
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
OpacityMap *MapperFunction::createOpacityMap(OpacityMap::Type type)
{
    int index = m_opacityMaps->Size();

    OpacityMap opacityMap(_ssave);
    opacityMap.SetType(type);

    m_opacityMaps->Insert(&opacityMap, _make_omap_name(index));

    return ((OpacityMap *)m_opacityMaps->GetParams(_make_omap_name(index)));
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void MapperFunction::DeleteOpacityMap(const OpacityMap *omap) { m_opacityMaps->Remove(omap); }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
OpacityMap *MapperFunction::GetOpacityMap(int index) const
{
    if (index >= m_opacityMaps->Size()) return (NULL);

    return ((OpacityMap *)m_opacityMaps->GetParams(_make_omap_name(index)));
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Reset to starting values
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Static utility function to map and quantize a float
//----------------------------------------------------------------------------
int MapperFunction::mapPosition(float x, float minValue, float maxValue, int hSize)
{
    double psn = (0.5 + ((((double)x - (double)minValue) * hSize) / ((double)maxValue - (double)minValue)));
    // constrain to integer size limit
    if (psn < -1000000000.f) psn = -1000000000.f;
    if (psn > 1000000000.f) psn = 1000000000.f;
    return (int)psn;
}

//----------------------------------------------------------------------------
// hsv-rgb Conversion function.  inputs and outputs between 0 and 1
// copied (with corrections) from Hearn/Baker
//----------------------------------------------------------------------------
void MapperFunction::hsvToRgb(float *hsv, float *rgb)
{
    if (hsv[1] == 0.f) {    // grey
        rgb[0] = rgb[1] = rgb[2] = hsv[2];
        return;
    }

    int sector = (int)(hsv[0] * 6.f);

    float sectCrd = hsv[0] * 6.f - (float)sector;
    if (sector == 6) sector = 0;
    float a = hsv[2] * (1.f - hsv[1]);
    float b = hsv[2] * (1.f - sectCrd * hsv[1]);
    float c = hsv[2] * (1.f - (hsv[1] * (1.f - sectCrd)));

    switch (sector) {
    case (0):
        // red to green, r>g
        rgb[0] = hsv[2];
        rgb[1] = c;
        rgb[2] = a;
        break;
    case (1):    // red to green, g>r
        rgb[1] = hsv[2];
        rgb[2] = a;
        rgb[0] = b;
        break;
    case (2):    // green to blue, gr>bl
        rgb[0] = a;
        rgb[1] = hsv[2];
        rgb[2] = c;
        break;
    case (3):    // green to blue, gr<bl
        rgb[0] = a;
        rgb[2] = hsv[2];
        rgb[1] = b;
        break;
    case (4):    // blue to red, bl>red
        rgb[1] = a;
        rgb[2] = hsv[2];
        rgb[0] = c;
        break;
    case (5):    // blue to red, bl<red
        rgb[1] = a;
        rgb[0] = hsv[2];
        rgb[2] = b;
        break;
    default: assert(0);
    }
    return;
}

//----------------------------------------------------------------------------
// rgb-hsv Conversion function.  inputs and outputs between 0 and 1
// copied (with corrections) from Hearn/Baker
//----------------------------------------------------------------------------
void MapperFunction::rgbToHsv(float *rgb, float *hsv)
{
    // value is max (r,g,b)
    float maxval = Max(rgb[0], Max(rgb[1], rgb[2]));
    float minval = Min(rgb[0], Min(rgb[1], rgb[2]));
    float delta = maxval - minval;    // chrominance
    hsv[2] = maxval;
    if (maxval != 0.f)
        hsv[1] = delta / maxval;
    else
        hsv[1] = 0.f;
    if (hsv[1] == 0.f)
        hsv[0] = 0.f;    // no hue!
    else {
        if (rgb[0] == maxval) {
            hsv[0] = (rgb[1] - rgb[2]) / delta;
            if (hsv[0] < 0.f) hsv[0] += 6.f;
        } else if (rgb[1] == maxval) {
            hsv[0] = 2.f + (rgb[2] - rgb[0]) / delta;
        } else {
            hsv[0] = 4.f + (rgb[0] - rgb[1]) / delta;
        }
        hsv[0] /= 6.f;    // Put between 0 and 1
    }
    return;
}

//----------------------------------------------------------------------------
// Set the opacity function to 1.
//----------------------------------------------------------------------------
void MapperFunction::setOpaque()
{
    for (int i = 0; i < getNumOpacityMaps(); i++) { GetOpacityMap(i)->setOpaque(); }
}
bool MapperFunction::isOpaque() const
{
    for (int i = 0; i < getNumOpacityMaps(); i++) {
        if (GetOpacityMap(i)->isOpaque()) return false;
    }
    return true;
}

string MapperFunction::_make_omap_name(int index) const
{
    stringstream ss;

    ss << MapperFunction::_opacityMapTag;
    ss << "_";
    ss << index;
    return (ss.str());
}
