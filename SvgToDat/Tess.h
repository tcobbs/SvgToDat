#ifndef _Tess_h_
#define _Tess_h_

#include "Point2D.h"
#include "Point3D.h"

class GLUtesselator;

class Tess
{
public:
	Tess(void);
	~Tess(void);
	void Tesselate(const Polygons2D &vecIn, Polygon2D &vecOut);
protected:
	static void _stdcall StaticVertexCallback(void *pVertex, void *pUserData);
	static void _stdcall StaticBeginCallback(unsigned eType, void *pUserData);
	static void _stdcall StaticEndCallback(void *pUserData);
	static void _stdcall StaticCombineCallback(double arCoords[3], void *pVertexData[4], float arWeight[4], void **ppOutData, void *pUserData);

	void VertexCallback(void *vertex);
	void BeginCallback(unsigned eType);
	void EndCallback(void);
	void CombineCallback(double arCoords[3], void *pVertexData[4], float arWeight[4], void **ppOutData);

	void AddContour(GLUtesselator *pTess, const Polygon2D &vecPolygon);
	void AddTriangleStrip(void);
	void AddTriangleFan(void);
	void AddTriangles(void);
	void AddTriangle(const Point3D &p0, const Point3D &p1, const Point3D &p2);

	Point3DList m_lsPoints;
	Polygon3D m_vecCurPoints;
	Polygon3D m_vecTriangles;
	unsigned m_eCurMode;
};
#endif // _Tess_h_

