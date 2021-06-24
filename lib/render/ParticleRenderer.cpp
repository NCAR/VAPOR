//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ParticleRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Implementation of ParticleRenderer
//

#include <sstream>
#include <string>
#include <iterator>

#include <vapor/glutil.h>    // Must be included first!!!

#include <vapor/Renderer.h>
#include <vapor/ParticleRenderer.h>
#include <vapor/ParticleParams.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/errorcodes.h>
#include <vapor/ControlExecutive.h>
#include "vapor/ShaderManager.h"
#include "vapor/debug.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "vapor/GLManager.h"
#include <vapor/LegacyGL.h>

using namespace VAPoR;

#pragma pack(push, 4)
// struct ParticleRenderer::VertexData {
//    float x, y, z;
//    float v;
//};
#pragma pack(pop)

static RendererRegistrar<ParticleRenderer> registrar(ParticleRenderer::GetClassType(), ParticleParams::GetClassType());

ParticleRenderer::ParticleRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, ParticleParams::GetClassType(), ParticleRenderer::GetClassType(), instName, dataMgr)
{
}

ParticleRenderer::~ParticleRenderer() {}

int ParticleRenderer::_paintGL(bool)
{
    int rc = 0;
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);

    auto p = GetActiveParams();

    MapperFunction *mf = p->GetMapperFunc(p->GetVariableName());
    float           LUT[256 * 4];
    mf->makeLut(LUT);
    float mapMin = mf->getMinMapValue();
    float mapMax = mf->getMaxMapValue();
    float mapDif = mapMax - mapMin;

    vector<double> minExt, maxExt;
    p->GetBox()->GetExtents(minExt, maxExt);
#define PD3(v) printf("%s = %f, %f, %f\n", #v, v[0], v[1], v[2])
    string varName = p->GetVariableName();
    Grid * grid = _dataMgr->GetVariable(p->GetCurrentTimestep(), varName, p->GetRefinementLevel(), p->GetCompressionLevel(), minExt, maxExt, true);
    if (!grid) return -1;

    vector<long> values;

    size_t         stride = max(1L, p->GetValueLong(ParticleParams::StrideTag, 1));
    bool           showDir = p->GetValueLong(ParticleParams::ShowDirectionTag, 0);
    float          dirScale = p->GetValueDouble(ParticleParams::DirectionScaleTag, 1);
    vector<Grid *> vecGrids;
    if (showDir) {
        vector<string> vecNames = p->GetFieldVariableNames();
        vector<string> mainVarCoords;
        _dataMgr->GetVarCoordVars(varName, true, mainVarCoords);

        for (auto var : vecNames) {
            vector<string> varCoords;
            _dataMgr->GetVarCoordVars(var, true, varCoords);

            if (mainVarCoords != varCoords) {
                if (grid) {
                    _dataMgr->UnlockGrid(grid);
                    delete grid;
                }
                for (auto g : vecGrids) {
                    _dataMgr->UnlockGrid(g);
                    delete g;
                }
                SetErrMsg("Variable \"%s\" on different grid from main variable", var.c_str());
                return -1;
            }

            Grid *ng = _dataMgr->GetVariable(p->GetCurrentTimestep(), var, p->GetRefinementLevel(), p->GetCompressionLevel(), minExt, maxExt, true);
            if (!ng) {
                if (grid) {
                    _dataMgr->UnlockGrid(grid);
                    delete grid;
                }
                for (auto g : vecGrids) {
                    _dataMgr->UnlockGrid(g);
                    delete g;
                }
                SetErrMsg("Cannot read var \"%s\"", var.c_str());
                return -1;
            }

            vecGrids.push_back(ng);
        }
    }

    auto   dims = grid->GetDimensions();
    size_t nCoords = 1;
    for (const auto d : dims) nCoords *= d;

    auto *lgl = _glManager->legacy;
    lgl->Color3f(1, 1, 1);
    lgl->Begin(showDir ? GL_LINES : GL_POINTS);

    long                        renderedParticles = 0;
    auto                        node = grid->ConstNodeBegin(minExt, maxExt);
    auto                        nodeEnd = grid->ConstNodeEnd();
    vector<double>              coordsBuf(3);
    vector<Grid::ConstIterator> dirs;
    for (auto g : vecGrids) dirs.push_back(g->cbegin(minExt, maxExt));
    for (size_t i = 0; node != nodeEnd; ++node, ++i) {
        if (i % stride) {
            if (showDir)
                for (auto &it : dirs) ++it;
            continue;
        }

        const float value = grid->GetValueAtIndex(*node);
        grid->GetUserCoordinates(*node, coordsBuf);
        const glm::vec3 p(coordsBuf[0], coordsBuf[1], coordsBuf[2]);

        lgl->Color4fv(&LUT[4 * glm::clamp((int)(255 * (value - mapMin) / mapDif), 0, 255)]);
        lgl->Vertex3fv((float *)&p);
        renderedParticles++;

        if (showDir) {
            const glm::vec3 n(*(dirs[0]), *(dirs[1]), *(dirs[2]));
            const glm::vec3 p2 = p + n * dirScale;
            lgl->Vertex3fv((float *)&p2);

            for (auto &it : dirs) ++it;
        }
    }
    lgl->End();

    //    printf("Rendered %li particles\n", renderedParticles);

    _dataMgr->UnlockGrid(grid);
    delete grid;
    for (auto g : vecGrids) {
        _dataMgr->UnlockGrid(g);
        delete g;
    }
    return rc;
}

int ParticleRenderer::_initializeGL() { return 0; }
