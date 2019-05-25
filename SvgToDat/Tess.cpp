#include "Tess.h"
#ifdef WIN32
#include <windows.h>
#endif // WIN32
#include <GL/glu.h>

Tess::Tess(void)
{
}

Tess::~Tess(void)
{
}

void Tess::AddContour(GLUtesselator *pTess, const Polygon2D &vecPolygon)
{
	gluTessBeginContour(pTess);
	for (size_t i = 0; i < vecPolygon.size(); ++i)
	{
		m_lsPoints.push_back(vecPolygon[i]);
		Point3D &sPoint = m_lsPoints.back();
		gluTessVertex(pTess, (GLdouble *)&sPoint, &sPoint);
	}
	gluTessEndContour(pTess);
}

void Tess::Tesselate(const Polygons2D &vecIn, Polygon2D &vecOut)
{
	GLUtesselator *pTess = gluNewTess();

	m_eCurMode = GL_INVALID_ENUM;
	m_vecCurPoints.clear();
	m_lsPoints.clear();
	m_vecTriangles.clear();

	gluTessCallback(pTess, GLU_TESS_VERTEX_DATA, (GLvoid(CALLBACK *)())(&Tess::StaticVertexCallback));
	gluTessCallback(pTess, GLU_TESS_BEGIN_DATA, (GLvoid(CALLBACK *)())(&Tess::StaticBeginCallback));
	gluTessCallback(pTess, GLU_TESS_END_DATA, (GLvoid(CALLBACK *)())(&Tess::StaticEndCallback));
	gluTessCallback(pTess, GLU_TESS_COMBINE_DATA, (GLvoid(CALLBACK *)())(&Tess::StaticCombineCallback));

	gluTessProperty(pTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

	gluTessBeginPolygon(pTess, this);
	for (size_t i = 0; i < vecIn.size(); ++i)
	{
		AddContour(pTess, vecIn[i]);
	}
	gluTessEndPolygon(pTess);

	gluDeleteTess(pTess);
	vecOut.reserve(m_vecTriangles.size());
	for (size_t i = 0; i < m_vecTriangles.size(); ++i)
	{
		vecOut.push_back(m_vecTriangles[i]);
	}
}

void Tess::StaticVertexCallback(GLvoid *pVertex, void *pUserData)
{
	((Tess *)pUserData)->VertexCallback(pVertex);
}

void Tess::StaticBeginCallback(GLenum eType, void *pUserData)
{
	((Tess *)pUserData)->BeginCallback(eType);
}

void Tess::StaticEndCallback(void *pUserData)
{
	((Tess *)pUserData)->EndCallback();
}

void Tess::StaticCombineCallback(GLdouble arCoords[3], void *pVertexData[4], GLfloat arWeight[4], void **ppOutData, void *pUserData)
{
	((Tess *)pUserData)->CombineCallback(arCoords, pVertexData, arWeight, ppOutData);
}

void Tess::VertexCallback(GLvoid *pVertex)
{
	m_vecCurPoints.push_back(*(Point3D *)pVertex);
}

void Tess::BeginCallback(GLenum eType)
{
	m_eCurMode = eType;
}

void Tess::AddTriangle(const Point3D &p0, const Point3D &p1, const Point3D &p2)
{
	if (p0 != p1 && p0 != p2 && p1 != p2)
	{
		Point3D oNormal = (p0 - p2) * (p0 - p1);

		if (oNormal.LengthSquared() > 0)
		{
			m_vecTriangles.push_back(p0);
			m_vecTriangles.push_back(p1);
			m_vecTriangles.push_back(p2);
		}
	}
}

void Tess::AddTriangles(void)
{
	for (size_t i = 0; i + 2 < m_vecCurPoints.size(); i += 3)
	{
		AddTriangle(m_vecCurPoints[i], m_vecCurPoints[i + 1], m_vecCurPoints[i + 2]);
	}
}

void Tess::AddTriangleStrip(void)
{
	for (size_t i = 0; i + 2 < m_vecCurPoints.size(); ++i)
	{
		if (i % 2 == 0)
		{
			AddTriangle(m_vecCurPoints[i], m_vecCurPoints[i + 1], m_vecCurPoints[i + 2]);
		}
		else
		{
			AddTriangle(m_vecCurPoints[i], m_vecCurPoints[i + 2], m_vecCurPoints[i + 1]);
		}
	}
}

void Tess::AddTriangleFan(void)
{
	for (size_t i = 1; i + 1 < m_vecCurPoints.size(); ++i)
	{
		AddTriangle(m_vecCurPoints[0], m_vecCurPoints[i], m_vecCurPoints[i + 1]);
	}
}

void Tess::EndCallback(void)
{
	switch (m_eCurMode)
	{
	case GL_TRIANGLES:
		AddTriangles();
		break;
	case GL_TRIANGLE_STRIP:
		AddTriangleStrip();
		break;
	case GL_TRIANGLE_FAN:
		AddTriangleFan();
		break;
	default:
		throw "Unsupported mode for tesselation";
	}
	m_vecCurPoints.clear();
}

// Note: pVertexData dn arWeight are only needed for color blending; we don't have color.
void Tess::CombineCallback(GLdouble arCoords[3], void * /*pVertexData*/[4], GLfloat /*arWeight*/[4], void **ppOutData)
{
	Point3D sPoint(arCoords[0], arCoords[1], arCoords[2]);
	m_lsPoints.push_back(sPoint);
	*ppOutData = &m_lsPoints.back();
}
