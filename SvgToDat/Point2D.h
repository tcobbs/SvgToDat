#ifndef _Point2D_h_
#define _Point2D_h_

#include <clipper.hpp>

class Point2D
{
public:
	Point2D(void) : x(0.0), y(0.0) {}
	Point2D(double x, double y) : x(x), y(y) {}
	Point2D(const clipper::IntPoint &ip) : x(ip.X / 100000.0), y(ip.Y / 100000.0) {}
	operator clipper::IntPoint() const
	{
		return clipper::IntPoint((clipper::long64)(x * 100000.0), (clipper::long64)(y * 100000.0));
	}
	~Point2D(void) {}
	Point2D operator+(const Point2D &other) const
	{
		return Point2D(x + other.x, y + other.y);
	}
	Point2D operator-(const Point2D &other) const
	{
		return Point2D(x - other.x, y - other.y);
	}
	Point2D operator*(double dValue) const
	{
		return Point2D(x * dValue, y * dValue);
	}
	Point2D operator/(double dValue) const
	{
		return Point2D(x / dValue, y / dValue);
	}
	Point2D &operator+=(const Point2D &other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	Point2D &operator-=(const Point2D &other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}
	Point2D &operator*=(double dValue)
	{
		x *= dValue;
		y *= dValue;
		return *this;
	}
	Point2D &operator/=(double dValue)
	{
		x /= dValue;
		y /= dValue;
		return *this;
	}
	bool operator==(const Point2D &other) const
	{
		return x == other.x && y == other.y;
	}
	bool operator!=(const Point2D &other) const
	{
		return x != other.x || y != other.y;
	}
	double x, y;
};

typedef std::vector<Point2D> Polygon2D;
typedef std::vector<Polygon2D> Polygons2D;

#endif // _Point2D_h_
