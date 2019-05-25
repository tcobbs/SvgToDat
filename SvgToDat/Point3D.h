#ifndef _Point3D_h_
#define _Point3D_h_

#include "Point2D.h"
#include <list>
#include <math.h>

class Point3D
{
public:
	Point3D(void) : x(0.0), y(0.0), z(0.0) {}
	Point3D(double x, double y, double z = 0.0) : x(x), y(y), z(z) {}
	Point3D(const Point2D &sPoint2D, double z = 0.0) : x(sPoint2D.x), y(sPoint2D.y), z(z) {}
	~Point3D(void) {}
	operator Point2D()
	{
		return Point2D(x, y);
	}
	Point3D operator+(const Point3D &other) const
	{
		return Point3D(x + other.x, y + other.y, z + other.z);
	}
	Point3D operator-(const Point3D &other) const
	{
		return Point3D(x - other.x, y - other.y, z - other.z);
	}
	Point3D operator*(double dValue) const
	{
		return Point3D(x * dValue, y * dValue, z * dValue);
	}
	Point3D operator/(double dValue) const
	{
		return Point3D(x / dValue, y / dValue, z / dValue);
	}
	Point3D &operator+=(const Point3D &other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Point3D &operator-=(const Point3D &other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
	Point3D &operator*=(double dValue)
	{
		x *= dValue;
		y *= dValue;
		z *= dValue;
		return *this;
	}
	Point3D &operator/=(double dValue)
	{
		x /= dValue;
		y /= dValue;
		z /= dValue;
		return *this;
	}
	Point3D operator*(const Point3D &other) const
	{
		return Point3D(y * other.z - z * other.y,
			z * other.x - x * other.z,
			x * other.y - y * other.x);
	}
	bool operator==(const Point3D &other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}
	bool operator!=(const Point3D &other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}
	double LengthSquared(void) const
	{
		return x * x + y * y + z * z;
	}
	double Length(void) const
	{
		return sqrt(LengthSquared());
	}
	double x, y, z;
};

typedef std::vector<Point3D> Polygon3D;
typedef std::vector<Polygon3D> Polygons3D;
typedef std::list<Point3D> Point3DList;

#endif // _Point3D_h_
