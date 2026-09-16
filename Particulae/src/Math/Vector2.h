#pragma once

#include "Real.h"
#include <cmath>



// Simpele 2D-vector voor positie, snelheid en kracht.

// Blijft bewust een gewone struct met vrije operators om zo verborgen kosten te
// verminderen; dit type wordt duidenden keren per stap gebruikt.

struct Vector2
{
	Real x = 0.0;
	Real y = 0.0;


	Vector2& operator+=(const Vector2& other)
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	Vector2& operator-=(const Vector2& other)
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	Vector2& operator*=(Real scalar)
	{
		x *= scalar;
		y *= scalar;

		return *this;
	}


	Real Length() const
	{
		return std::sqrt(x * x + y * y);
	}


	static Real Dot(const Vector2& a, const Vector2& b)
	{
		return a.x * b.x + a.y * b.y;
	}
};



// Vrije operators die geen aanpassing van de linkerkant nodig hebben.

inline Vector2 operator+(Vector2 a, const Vector2& b)
{
	a += b;
	return a;
}

inline Vector2 operator-(Vector2 a, const Vector2& b)
{
	a -= b;
	return a;
}



// Waarde negatief maken om de richting om te draaien, bijvooreeld bij reactiekrachten.

inline Vector2 operator-(const Vector2& a)
{
	return { -a.x, -a.y };
}



// Operators voor het schalen van de vector.

inline Vector2 operator*(Vector2 vector, Real scalar)
{
	vector *= scalar;
	return vector;
}

inline Vector2 operator*(Real scalar, Vector2 vector)
{
	vector *= scalar;
	return vector;
}

inline Vector2 operator/(Vector2 vector, Real scalar)
{
	vector.x /= scalar;
	vector.y /= scalar;

	return vector;
}