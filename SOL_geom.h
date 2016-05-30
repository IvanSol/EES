#ifndef SOL_GEOM
#define SOL_GEOM

#include "SOL_classes.h"
#include "SOL_CONST.h"

class Vect
{
public:
	double x;
	double y;
	Vect(const Pnt & p)
	{
		x = p.x;
		y = p.y;
	}
	Vect(const Pnt & a, const Pnt & b)
	{
		x = b.x - a.x;
		y = b.y - a.y;
	}
	Vect(double _x, double _y)
	{
		x = _x;
		y = _y;
	}
	double len()
	{
		return sqrt((double) x*x + y*y);
	}
};

void vec_norm(Vect & v)
{
	double len = v.len();
	v.x /= len;
	v.y /= len;
}

Vect get_n_vect(double fi)
{
	return Vect(cos(fi), sin(fi));
}

Vect operator/(const Vect & v, const double k)
{
	return Vect(v.x/k, v.y/k);
}

Vect operator*(const Vect & v, const double k)
{
	return Vect(v.x*k, v.y*k);
}

double operator*(const Vect & a, const Vect & b)
{
	return a.x * b.x + a.y * b.y;
}

double Vmul(const Vect & a, const Vect & b)
{
	return a.x * b.y - a.y * b.x;
}

double rho0(const Circ & ib, const Circ & ob, const double & fi)
{
	if (ib.c == ob.c)
		return ob.R - ib.R;
	Vect v2(ib.c, ob.c);
	double d = v2.len();
	vec_norm(v2);
	Vect v1 = get_n_vect(fi);
	double c = v1 * v2;
	double s2 = sqr(Vmul(v1, v2));
	double x1 = d * c + sqrt(sqr(ob.R) - d*d*s2);
	double x2 = d * c - sqrt(sqr(ob.R) - d*d*s2);
	return x1 - ib.R;
}

#endif