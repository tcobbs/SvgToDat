#include "SvgLoader.h"
#include "Tess.h"

SvgLoader::NodeTypeMap SvgLoader::sm_mapNodeTypes;

#define CIRCLE_SIZE 16

using namespace clipper;

SvgLoader::SvgLoader(void)
{
}

SvgLoader::~SvgLoader(void)
{
}

void SvgLoader::BuildClipperPolygons(const Polygons2D &vecSrc, Polygons &vecDst)
{
	vecDst.reserve(vecSrc.size());
	for (size_t i = 0; i < vecSrc.size(); i++)
	{
		const Polygon2D &vecPolygon2D = vecSrc[i];

		vecDst.push_back(Polygon(vecPolygon2D.begin(), vecPolygon2D.end()));
	}
}

void SvgLoader::ApplyDifferences(void)
{
	for (size_t i = 0; i + 1 < m_vecShapes.size(); i++)
	{
		Shape &oShape = m_vecShapes[i];
		Clipper c;
		Polygons vecLeft;
		Polygons vecResult;

		BuildClipperPolygons(oShape.GetPolygons(), vecLeft);
		c.AddPolygons(vecLeft, ptSubject);
		for (size_t j = i + 1; j < m_vecShapes.size(); j++)
		{
			Polygons vecRight;

			BuildClipperPolygons(m_vecShapes[j].GetPolygons(), vecRight);
			c.AddPolygons(vecRight, ptClip);
		}
		if (!c.Execute(ctDifference, vecResult, pftNonZero, pftNonZero))
		{
			// I think it will already be clear, but just be sure.
			vecResult.clear();
		}
		oShape.SetPolygons(vecResult);
	}
}

void SvgLoader::Tesselate(void)
{
	Tess oTess;

	for (size_t i = 0; i < m_vecShapes.size(); i++)
	{
		Shape &oShape = m_vecShapes[i];
		Polygon2D &vecPoints = oShape.GetTriangles();
		const Polygons2D &vecPolygons = oShape.GetPolygons();

		oTess.Tesselate(vecPolygons, vecPoints);
	}
}

void SvgLoader::WriteDat(const std::string &strFilename)
{
	FILE *pOutput = fopen(strFilename.c_str(), "w");

	if (pOutput != NULL)
	{
		double dZ = 0.0;
		for (size_t i = 0; i < m_vecShapes.size(); i++)
		{
			Shape &oShape = m_vecShapes[i];
			Polygon2D &vecPoints = oShape.GetTriangles();
			int nColorNumber = oShape.GetColorNumber();

			if (vecPoints.size() >= 3)
			{
				for (size_t j = 0; j + 2 < vecPoints.size(); j += 3)
				{
					const Point2D &p0(vecPoints[j]);
					const Point2D &p1(vecPoints[j + 1]);
					const Point2D &p2(vecPoints[j + 2]);
					if (nColorNumber >= 0x2000000)
					{
						fprintf(pOutput, "3 0x%X %g %g %g %g %g %g %g %g %g\n", nColorNumber, p0.x, p0.y, dZ, p1.x, p1.y, dZ, p2.x, p2.y, dZ);
					}
					else
					{
						fprintf(pOutput, "3 %d %g %g %g %g %g %g %g %g %g\n", nColorNumber, p0.x, p0.y, dZ, p1.x, p1.y, dZ, p2.x, p2.y, dZ);
					}
				}
				//dZ -= 0.1;
				//exit(0);
			}
		}
		fclose(pOutput);
	}
}

bool SvgLoader::Load(const std::string &strFilename)
{
	TiXmlDocument oDoc(strFilename);

	if (oDoc.LoadFile())
	{
		TiXmlElement *pSvg = oDoc.FirstChildElement("svg");

		if (pSvg != NULL)
		{
			if (ParseChildren(pSvg))
			{
				ApplyDifferences();
				Tesselate();
				return true;
			}
		}
	}
	return false;
}

// switch node:
// Process children until you find one that you can handle.  Ignore children
// that you don't understand, as well as all children after the first
// understandable child.
bool SvgLoader::ParseSwitch(TiXmlElement *pNode)
{
	TiXmlElement *pChild;

	for (pChild = pNode->FirstChildElement(); pChild != NULL; pChild = pChild->NextSiblingElement())
	{
		if (ParseNode(pChild))
		{
			return true;
		}
	}
	return false;
}

bool SvgLoader::ParseGroup(TiXmlElement *pNode)
{
	// Groups have a bunch of state that needs to be passed on, but not now.
	return ParseChildren(pNode);
}

// Note: static method
bool SvgLoader::HaveValue(const char *pszValue)
{
	return pszValue != NULL && pszValue[0] != 0;
}

bool SvgLoader::ParsePolygon(TiXmlElement *pNode)
{
	const char *pszPoints = pNode->Attribute("points");
	Polygon2D vecPoints;

	if (ParseValues(pszPoints, vecPoints))
	{
		Polygon vecPolygon(vecPoints.begin(), vecPoints.end());

		AddShape(vecPolygon, pNode);
		return true;
	}
	return false;
}

// Note: static method
bool SvgLoader::ParseValue(const char *&pszData, double &dValue)
{
	size_t i;
	char cPrev = 0;

	for (i = 0; pszData[i]; ++i)
	{
		if (!IsDoubleDigit(pszData[i], cPrev))
		{
			break;
		}
		cPrev = pszData[i];
	}
	if (i > 0)
	{
		std::string strValue(pszData, i);

		if (sscanf(strValue.c_str(), "%lf", &dValue) == 1)
		{
			pszData += i;
			return true;
		}
	}
	return false;
}

// Note: static method
bool SvgLoader::IsDoubleDigit(char cValue, char cPrev)
{
	if (isdigit(cValue))
	{
		return true;
	}
	switch (cValue)
	{
	case 'e':
	case 'E':
	case '.':
		return true;
	case '-':
		return cPrev == 0 || toupper(cPrev) == 'E';
	default:
		return false;
	}
}

// Note: static method
bool SvgLoader::IsWhiteSpace(char cValue)
{
	switch (cValue)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return true;
	default:
		return false;
	}
}

void SvgLoader::SkipWhiteSpace(const char *&pszData)
{
	while (IsWhiteSpace(pszData[0]))
	{
		++pszData;
	}
}

// Note: static method
bool SvgLoader::ParseValue(const char *&pszData, Point2D &sPoint)
{
	const char *pszOrig = pszData;
	if (ParseValue(pszData, sPoint.x))
	{
		SkipWhiteSpace(pszData);
		if (pszData[0] == ',')
		{
			++pszData;
			SkipWhiteSpace(pszData);
		}
		if (ParseValue(pszData, sPoint.y))
		{
			// Can't return the value directly, because we want to
			// reset back if it fails.
			return true;
		}
	}
	pszData = pszOrig;
	return false;
}

// Note: static method
template <typename _T> bool SvgLoader::ParseValues(const char *&pszData, std::vector<_T> &vecValues)
{
	const char *pszOrig = pszData;

	while (pszData[0])
	{
		_T value;

		if (ParseValue(pszData, value))
		{
			vecValues.push_back(value);
		}
		else
		{
			break;
		}
		SkipWhiteSpace(pszData);
		if (pszData[0] == ',')
		{
			++pszData;
			SkipWhiteSpace(pszData);
		}
	}
	if (vecValues.size() > 0)
	{
		return true;
	}
	else
	{
		pszData = pszOrig;
		return false;
	}
}

//// Note: static method
//bool SvgLoader::ParseValues(const char *&pszData, Polygon2D &vecPoints)
//{
//	const char *pszOrig = pszData;
//
//	while (pszData[0])
//	{
//		Point2D sPoint;
//
//		if (ParseValue(pszData, sPoint))
//		{
//			vecPoints.push_back(sPoint);
//		}
//		else
//		{
//			break;
//		}
//		SkipWhiteSpace(pszData);
//		if (pszData[0] == ',')
//		{
//			++pszData;
//			SkipWhiteSpace(pszData);
//		}
//	}
//	if (vecPoints.size() > 0)
//	{
//		return true;
//	}
//	else
//	{
//		pszData = pszOrig;
//		return false;
//	}
//}

// Note: static method
void SvgLoader::OffsetIfNeeded(char cCmd, const Point2D &sLastPoint, Point2D &sPoint)
{
	if (islower(cCmd))
	{
		sPoint += sLastPoint;
	}
}

// Note: static method
template <typename _T> static void SvgLoader::OffsetIfNeeded(char cCmd, const _T &lastValue, std::vector<_T> &vecValues)
{
	for (size_t i = 0; i < vecValues.size(); ++i)
	{
		OffsetIfNeeded(cCmd, lastValue, vecValues[i]);
	}
}

// Note: static method
void SvgLoader::OffsetIfNeeded(char cCmd, const double &dLastValue, double &dValue)
{
	if (islower(cCmd))
	{
		dValue += dLastValue;
	}
}

// Note: static method
void SvgLoader::GenerateCurveTo(const Point2D &sStart, const Polygon2D &vecIn, Polygon2D &vecOut)
{
	Point2D sFrom = sStart;

	for (size_t i = 0; i + 2 < vecIn.size(); i += 3)
	{
		sFrom = vecIn[i + 2];
		vecOut.push_back(sFrom);
	}
}

// Note: static method
void SvgLoader::GenerateQuadCurveTo(const Point2D &sStart, const Polygon2D &vecIn, Polygon2D &vecOut)
{
	Point2D sFrom = sStart;

	for (size_t i = 0; i + 1 < vecIn.size(); i += 2)
	{
		sFrom = vecIn[i + 1];
		vecOut.push_back(sFrom);
	}
}

// Note: static method
void SvgLoader::GenerateArcTo(const Point2D &sStart, const DoubleVector &vecIn, Polygon2D &vecOut)
{
	Point2D sFrom = sStart;

	for (size_t i = 0; i + 6 < vecIn.size(); i += 7)
	{
		sFrom = Point2D(vecIn[i + 5], vecIn[i + 6]);
		vecOut.push_back(sFrom);
	}
}

bool SvgLoader::ParsePathData(const char *pszData, clipper::Polygons &vecPolygons, clipper::Polygons &vecPolyLines)
{
	const char *pszSpot = pszData;
	Point2D sLastPoint;
	Polygon vecPoly;

	while (pszSpot[0])
	{
		SkipWhiteSpace(pszSpot);
		char cCmd = pszSpot[0];
		Polygon2D vecPoints;
		Polygon2D vecInPoints;
		DoubleVector vecValues;
		bool bOffsetDone = false;

		++pszSpot;
		switch (cCmd)
		{
		case 0:		// We're done; don't trigger an error.
			break;
		case 'M':	// moveto (absolute)
		case 'm':	// moveto (relative)
			if (ParseValues(pszSpot, vecPoints))
			{
				if (vecPoly.size() > 0)
				{
					vecPolyLines.push_back(vecPoly);
					vecPoly.clear();
				}
			}
			else
			{
				return false;
			}
			break;
		case 'Z':	// closepath
		case 'z':	// closepath
			if (vecPoly.size() > 0)
			{
				if ((Point2D)vecPoly.front() == (Point2D)vecPoly.back())
				{
					vecPoly.resize(vecPoly.size() - 1);
				}
				vecPolygons.push_back(vecPoly);
				sLastPoint = vecPoly.front();
				vecPoly.clear();
			}
			break;
		case 'L':	// lineto (absolute)
		case 'l':	// lineto (relative)
			if (!ParseValues(pszSpot, vecPoints))
			{
				return false;
			}
			break;
		case 'H':	// horizontal lineto (absolute)
		case 'h':	// horizontal lineto (relative)
			if (ParseValues(pszSpot, vecValues))
			{
				bOffsetDone = true;
				OffsetIfNeeded(cCmd, sLastPoint.x, vecValues);
				for (size_t i = 0; i < vecValues.size(); ++i)
				{
					vecPoints.push_back(Point2D(vecValues[i], sLastPoint.y));
				}
			}
			else
			{
				return false;
			}
			break;
		case 'V':	// vertical lineto (absolute)
		case 'v':	// vertical lineto (relative)
			if (ParseValues(pszSpot, vecValues))
			{
				bOffsetDone = true;
				OffsetIfNeeded(cCmd, sLastPoint.y, vecValues);
				for (size_t i = 0; i < vecValues.size(); ++i)
				{
					vecPoints.push_back(Point2D(sLastPoint.x, vecValues[i]));
				}
			}
			else
			{
				return false;
			}
			break;
		case 'C':	// curveto (absolute)
		case 'c':	// curveto (relative)
			if (ParseValues(pszSpot, vecInPoints))
			{
				if (vecInPoints.size() % 3 != 0)
				{
					return false;
				}
				GenerateCurveTo(sLastPoint, vecInPoints, vecPoints);
			}
			else
			{
				return false;
			}
			break;
		case 'S':	// shorthand/smooth curveto (absolute)
		case 's':	// shorthand/smooth curveto (relative)
			if (ParseValues(pszSpot, vecInPoints))
			{
				if (vecInPoints.size() % 2 != 0)
				{
					return false;
				}
				for (size_t i = 0; i + 1 < vecInPoints.size(); i+= 3)
				{
					vecInPoints.insert(vecInPoints.begin() + i, Point2D());
				}
				GenerateCurveTo(sLastPoint, vecInPoints, vecPoints);
			}
			else
			{
				return false;
			}
			break;
		case 'Q':	// quadratic curveto (absolute)
		case 'q':	// quadratic curveto (relative)
			if (ParseValues(pszSpot, vecInPoints))
			{
				if (vecInPoints.size() % 2 != 0)
				{
					return false;
				}
				GenerateQuadCurveTo(sLastPoint, vecInPoints, vecPoints);
			}
			else
			{
				return false;
			}
			break;
		case 'T':	// shorthand/smooth quadratic curveto (absolute)
		case 't':	// shorthand/smooth quadratic curveto (relative)
			if (ParseValues(pszSpot, vecInPoints))
			{
				for (size_t i = 0; i + 1 < vecInPoints.size(); i+= 2)
				{
					vecInPoints.insert(vecInPoints.begin() + i, Point2D());
				}
				GenerateQuadCurveTo(sLastPoint, vecInPoints, vecPoints);
			}
			else
			{
				return false;
			}
			break;
		case 'A':	// arcto (absolute)
		case 'a':	// arcto (relative)
			if (ParseValues(pszSpot, vecValues))
			{
				if (vecValues.size() % 7 != 0)
				{
					return false;
				}
				GenerateArcTo(sLastPoint, vecValues, vecPoints);
			}
			else
			{
				return false;
			}
			break;
		default:
			return false;
		}
		if (vecPoints.size() > 0)
		{
			if (!bOffsetDone)
			{
				OffsetIfNeeded(cCmd, sLastPoint, vecPoints);
			}
			vecPoly.insert(vecPoly.end(), vecPoints.begin(), vecPoints.end());
			sLastPoint = vecPoints.back();
		}
	}
	return true;
}

void SvgLoader::SetShapeColor(Shape &oShape, TiXmlElement *pNode)
{
	int nFillColorNumber = 16;
	const char *pszFill = pNode->Attribute("fill");
	int nRgb = 0;

	if (pszFill != NULL && pszFill[0] == '#')
	{
		if (sscanf(&pszFill[1], "%x", &nRgb) != 1)
		{
			nRgb = 0;
		}
	}
	nFillColorNumber = 0x2000000 + nRgb;
	oShape.SetColorNumber(nFillColorNumber);
}

void SvgLoader::AddShape(const Polygons &vecPolygons, TiXmlElement *pNode)
{
	m_vecShapes.resize(m_vecShapes.size() + 1);
	Shape &oShape(m_vecShapes.back());
	oShape.SetPolygons(vecPolygons);
	SetShapeColor(oShape, pNode);
}

void SvgLoader::AddShape(const Polygon &vecPolygon, TiXmlElement *pNode)
{
	Polygons vecPolygons;

	vecPolygons.push_back(vecPolygon);
	AddShape(vecPolygons, pNode);
}

bool SvgLoader::ParsePath(TiXmlElement *pNode)
{
	const char *pszData = pNode->Attribute("d");

	if (HaveValue(pszData))
	{
		Polygons vecPolygons;
		Polygons vecPolyLines;

		if (!ParsePathData(pszData, vecPolygons, vecPolyLines))
		{
			return false;
		}
		AddShape(vecPolygons, pNode);
		return true;
	}
	return false;
}

bool SvgLoader::ParseRect(TiXmlElement *pNode)
{
	const char *pszX = pNode->Attribute("x");
	const char *pszY = pNode->Attribute("y");
	const char *pszWidth = pNode->Attribute("width");
	const char *pszHeight = pNode->Attribute("height");

	if (HaveValue(pszX) && HaveValue(pszY) && HaveValue(pszWidth) && HaveValue(pszHeight))
	{
		double x, y, width, height;

		if (sscanf(pszX, "%lf", &x) == 1 &&
			sscanf(pszY, "%lf", &y) == 1 &&
			sscanf(pszWidth, "%lf", &width) == 1 &&
			sscanf(pszHeight, "%lf", &height) == 1)
		{
			Polygon vecPolygon;

			vecPolygon.reserve(4);
			vecPolygon.push_back(Point2D(x, y));
			vecPolygon.push_back(Point2D(x + width, y));
			vecPolygon.push_back(Point2D(x + width, y + height));
			vecPolygon.push_back(Point2D(x, y + height));
			AddShape(vecPolygon, pNode);
			return true;
		}
	}
	return false;
}

bool SvgLoader::ParseCircle(TiXmlElement *pNode)
{
	const char *pszX = pNode->Attribute("cx");
	const char *pszY = pNode->Attribute("cy");
	const char *pszR = pNode->Attribute("r");

	if (HaveValue(pszX) && HaveValue(pszY) && HaveValue(pszR))
	{
		double x, y, r;

		if (sscanf(pszX, "%lf", &x) == 1 &&
			sscanf(pszY, "%lf", &y) == 1 &&
			sscanf(pszR, "%lf", &r) == 1)
		{
			Polygon vecPolygon;

			vecPolygon.reserve(CIRCLE_SIZE);
			for (size_t i = 0; i < CIRCLE_SIZE; i++)
			{
				double dAngle = i * M_PI * 2.0 / CIRCLE_SIZE;
				vecPolygon.push_back(Point2D(x + r * cos(dAngle), y + r * sin(dAngle)));
			}
			AddShape(vecPolygon, pNode);
			return true;
		}
	}
	return false;
}

bool SvgLoader::ParseNode(TiXmlElement *pNode)
{
	switch (GetNodeType(pNode->Value()))
	{
	case eNTSwitch:
		return ParseSwitch(pNode);
	case eNTGroup:
		return ParseGroup(pNode);
	case eNTPolygon:
		return ParsePolygon(pNode);
	case eNTPath:
		return ParsePath(pNode);
	case eNTRect:
		return ParseRect(pNode);
	case eNTCircle:
		return ParseCircle(pNode);
	default:
		return false;
	}
}

bool SvgLoader::ParseChildren(TiXmlElement *pNode)
{
	TiXmlElement *pChild;
	bool bFoundGeometry = false;

	for (pChild = pNode->FirstChildElement(); pChild != NULL; pChild = pChild->NextSiblingElement())
	{
		if (ParseNode(pChild))
		{
			bFoundGeometry = true;
		}
	}
	return bFoundGeometry;
}

// Note: static function
SvgLoader::ENodeType SvgLoader::GetNodeType(const std::string &strValue)
{
	if (sm_mapNodeTypes.size() == 0)
	{
		sm_mapNodeTypes["switch"] = eNTSwitch;
		sm_mapNodeTypes["g"] = eNTGroup;
		sm_mapNodeTypes["polygon"] = eNTPolygon;
		sm_mapNodeTypes["path"] = eNTPath;
		sm_mapNodeTypes["rect"] = eNTRect;
		sm_mapNodeTypes["circle"] = eNTCircle;
	}
	NodeTypeMap::const_iterator it = sm_mapNodeTypes.find(strValue);
	if (it != sm_mapNodeTypes.end())
	{
		return it->second;
	}
	return eNTUnknown;
}
