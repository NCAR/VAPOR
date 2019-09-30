//---------------- ----------------------------------------------------------
//   
//                   Copyright (C)  2018
//     University Corporation for Atmospheric Research
//                   All Rights Reserved
//
//----------------------------------------------------------------------------

#ifndef	WIREFRAMERENDERER_H
#define	WIREFRAMERENDERER_H

#include <vapor/glutil.h> // Must be included first!!!

#ifdef Darwin
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <vapor/DataMgr.h>
#include <vapor/utils.h>
#include <vapor/Renderer.h>

namespace VAPoR {

//! \class WireFrameRenderer
//! \brief 
//! \author John Clyne
//! \version 3.0
//! \date June 2018

class RENDER_API WireFrameRenderer : public Renderer 
{
public:

	//! Constructor, must invoke Renderer constructor
	//! \param[in] Visualizer* pointer to the visualizer where this will draw
	//! \param[in] RenderParams* pointer to the ArrowParams describing 
	//! this renderer
	WireFrameRenderer(  const ParamsMgr*    pm, 
			string        winName,     
			string        dataSetName, 
			string        instName,                 
			DataMgr*      dataMgr );

	static string GetClassType() {
		return("WireFrame");
	}

	//! Destructor
	//
	virtual ~WireFrameRenderer();


protected:


	//! \copydoc Renderer::_initializeGL()
	virtual int _initializeGL();

	//! \copydoc Renderer::_paintGL()
	virtual int _paintGL(bool fast);

private:
	GLuint _VAO, _VBO, _EBO;
    unsigned int _nIndices;

	struct VertexData;
	struct {
		string varName;
		string heightVarName;
		size_t ts;
		int level;
		int lod;
		bool useSingleColor;
		std::vector <float> constantColor;
		float constantOpacity;
		std::vector<float> tf_lut;
		std::vector<double> tf_minmax;
		std::vector<double> boxMin, boxMax;

	} _cacheParams;

	class DrawList {
	public:
		DrawList(size_t maxEntries, size_t maxLinesPerVertex) :
			_drawList(
				maxEntries*maxLinesPerVertex, std::numeric_limits<size_t>::max()
			),
			_maxEntries(maxEntries),
			_maxLinesPerVertex(maxLinesPerVertex)
		 {}	

		bool InList(size_t idx0, size_t idx1) {
			VAssert(idx0 < _maxEntries);
			VAssert(idx1 < _maxEntries);

			if (idx1 < idx0) {
				size_t tmp = idx0;
				idx0 = idx1;
				idx1 = tmp;
			}

			for (int i = 0; i<_maxLinesPerVertex; i++) {
				if (_drawList[idx0*_maxLinesPerVertex+i] == idx1) {
					return(true);
				}
				if (_drawList[idx0*_maxLinesPerVertex+i] == std::numeric_limits<size_t>::max()) {
					_drawList[idx0*_maxLinesPerVertex+i] = idx1;
					return(false);
				}
			}
			return(false);
		}
	private:
		vector<size_t> _drawList;
		size_t _maxEntries;
		size_t _maxLinesPerVertex;
	};

	int  _buildCache();
	bool _isCacheDirty() const;
	void _saveCacheParams();
	void _drawCell(
		const size_t *cellNodeIndices,
		int n,
		bool layered,
		const std::vector <size_t> &nodeMap,
		size_t invalidIndex,
		std::vector<unsigned int> &indices,
		DrawList &drawList
	) const;

  void _clearCache() {
	_cacheParams.varName.clear();
  }

};

};

#endif
