#pragma once

class Angle {
private:
	double radians = 0.0f;
private:
	GENERATE_SERIALIZE(radians)

public:
	Angle() {}
	explicit Angle(double _rad) :radians(_rad) {}
	Angle(float _deg)
	{
		radians = Math::DegToRad(_deg);
	}

	Angle& operator=(const float& _deg)
	{
		radians = Math::DegToRad(_deg);
		return *this;
	}

	Angle& operator+=(const float& _deg)
	{
		radians += Math::DegToRad(_deg);
		return *this;
	}

	Angle& operator-=(const float& _deg)
	{
		radians -= Math::DegToRad(_deg);
		return *this;
	}

	Angle& operator+(const float& _deg)
	{
		radians += Math::DegToRad(_deg);
		return *this;
	}

	Angle& operator-(const float& _deg)
	{
		radians -= Math::DegToRad(_deg);
		return *this;
	}

	operator float() const
	{
		return Math::RadToDeg(static_cast<float>(radians));
	}

	const double Get() const
	{
		return radians;
	}

	void Set(const double _rad)
	{
		radians = _rad;
	}
};

struct Angle3D
{
	Angle x;
	Angle y;
	Angle z;

	Angle3D& operator=(const Angle3D& _angles)
	{
		this->x = _angles.x;
		this->y = _angles.y;
		this->z = _angles.z;
	}

	Angle3D& operator=(const Vector3& _vec)
	{
		this->x = _vec.x;
		this->y = _vec.y;
		this->z = _vec.z;
	}

	operator Vector3() const
	{
		return{ x,y,z };
	}

private:
	GENERATE_SERIALIZE(x, y, z)
};
