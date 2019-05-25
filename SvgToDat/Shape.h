#ifndef _Shape_h_
#define _Shape_h_

#include "Point2D.h"
#include <clipper.hpp>

class Shape
{
public:
	Shape(void) : m_nColorNumber(16) {}
	Shape(const Polygons2D &vecPolygons, int nColorNumber) : m_vecPolygons(vecPolygons), m_nColorNumber(nColorNumber) {}
	Shape(const clipper::Polygons &vecPolygons, int nColorNumber) : m_nColorNumber(nColorNumber)
	{
		SetPolygons(vecPolygons);
	}
	~Shape(void) {}
	const Polygons2D &GetPolygons(void) const { return m_vecPolygons; }
	void SetPolygons(const Polygons2D &vecPolygons) { m_vecPolygons = vecPolygons; }
	void SetPolygons(const clipper::Polygons &vecPolygons)
	{
		m_vecPolygons.clear();
		m_vecPolygons.reserve(vecPolygons.size());
		for (size_t i = 0; i < vecPolygons.size(); i++)
		{
			const clipper::Polygon &vecPoly = vecPolygons[i];

			m_vecPolygons.push_back(Polygon2D(vecPoly.begin(), vecPoly.end()));
		}
	}
	int GetColorNumber(void) const { return m_nColorNumber; }
	void SetColorNumber(int nColorNumber) { m_nColorNumber = nColorNumber; }
	Polygon2D &GetTriangles(void) { return m_vecTriangles; }
protected:
	Polygons2D m_vecPolygons;
	int m_nColorNumber;
	Polygon2D m_vecTriangles;
};

typedef std::vector<Shape> Shapes;

#endif // _Shape_h_
