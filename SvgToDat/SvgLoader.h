#ifndef _SvgLoader_h_
#define _SvgLoader_h_

#include "VectorDrawing.h"
#include <tinyxml.h>
#include <map>
#include <clipper.hpp>
#include "Shape.h"

class SvgLoader : public VectorDrawing
{
public:
	SvgLoader(void);
	~SvgLoader(void);
	virtual bool Load(const std::string &strFilename);
	void WriteDat(const std::string &strFilename);
protected:
	enum ENodeType
	{
		eNTUnknown,
		eNTSwitch,
		eNTGroup,
		eNTPolygon,
		eNTPath,
		eNTRect,
		eNTCircle,
	};
	typedef std::map<std::string, ENodeType> NodeTypeMap;
	typedef std::vector<double> DoubleVector;

	bool ParseNode(TiXmlElement *pNode);
	bool ParseChildren(TiXmlElement *pNode);
	bool ParseSwitch(TiXmlElement *pNode);
	bool ParseGroup(TiXmlElement *pNode);
	bool ParsePolygon(TiXmlElement *pNode);
	bool ParsePath(TiXmlElement *pNode);
	bool ParseRect(TiXmlElement *pNode);
	bool ParseCircle(TiXmlElement *pNode);
	bool ParsePathData(const char *pszData, clipper::Polygons &vecPolygons, clipper::Polygons &vecPolyLines);
	void SetShapeColor(Shape &oShape, TiXmlElement *pNode);
	void AddShape(const clipper::Polygons &vecPolygons, TiXmlElement *pNode);
	void AddShape(const clipper::Polygon &vecPolygon, TiXmlElement *pNode);

	void ApplyDifferences(void);
	void Tesselate(void);

	void BuildClipperPolygons(const Polygons2D &vecSrc, clipper::Polygons &vecDst);

	static bool ParseValue(const char *&pszData, Point2D &sPoint);
	static bool ParseValue(const char *&pszData, double &dValue);
	static bool IsWhiteSpace(char cValue);
	static bool IsDoubleDigit(char cValue, char cPrev);
	static void SkipWhiteSpace(const char *&pszData);
	static void OffsetIfNeeded(char cCmd, const Point2D &sLastPoint, Point2D &sPoint);
	static void OffsetIfNeeded(char cCmd, const double &dLastValue, double &dValue);

	static void GenerateCurveTo(const Point2D &sStart, const Polygon2D &vecIn, Polygon2D &vecOut);
	static void GenerateQuadCurveTo(const Point2D &sStart, const Polygon2D &vecIn, Polygon2D &vecOut);
	static void GenerateArcTo(const Point2D &sStart, const DoubleVector &vecIn, Polygon2D &vecOut);

	static ENodeType GetNodeType(const std::string &strValue);
	static bool HaveValue(const char *pszValue);

	static NodeTypeMap sm_mapNodeTypes;

	Shapes m_vecShapes;

private:
	template <typename _T> static bool ParseValues(const char *&pszData, std::vector<_T> &vecValues);
	template <typename _T> static void OffsetIfNeeded(char cCmd, const _T &lastValue, std::vector<_T> &vecValues);
};

#endif // _SvgLoader_h_
