#include "Vector.hpp"

const Vector Vector::Zero = Vector( 0, 0, 0 );

const Vector& Vector::Normalized() const
{
	float length = Length();
	if ( length == 0.0f )
	{
		// Approximate infinity, I guess?
		length = 0.00000001f;
	}

	return *this / length;
}

const Vector& Vector::ToAngles( bool flipPitch = false ) const
{
	float forward;
	float yaw, pitch;

	if ( y == 0 && x == 0 )
	{
		yaw = 0;
		if ( z > 0 )
		{
			pitch = 90;
		}

		else
		{
			pitch = 270;
		}
	}

	else
	{
		if ( x )
		{
			yaw = (atan2( y, x ) * 180 / M_PI);
		}

		else if ( y > 0 )
		{
			yaw = 90;
		}

		else
		{
			yaw = 270;
		}

		if ( yaw < 0 )
		{
			yaw += 360;
		}

		forward = sqrt( x * x + y * y );
		pitch = (atan2( z, forward ) * 180 / M_PI);

		if ( pitch < 0 )
		{
			pitch += 360;
		}
	}

	return Vector( flipPitch ? pitch : -pitch, yaw, 0 );
}

inline const Vector& Vector::RotatedAboutAxis( const Vector& axis, float angle ) const
{
	Vector cosV = cos( angle ) * *this;
	Vector K = (1.0f - cos( angle )) * (axis * *this) * axis;
	Vector cross = sin( angle ) * CrossProduct( axis );

	return cosV + K + cross;
}


inline const Vector& Vector::ProjectedOnPlane( const Vector& normal )
{
	float inverseDenominator = 1.0f / (normal * normal);
	float dot = (normal * *this) * inverseDenominator;
	Vector inverseNormal = normal * inverseDenominator;

	return *this - (dot * inverseNormal);
}

const Vector& Vector::FromAngles( float pitch, float yaw, float radius = 1.0f )
{
	pitch = pitch * (M_PI * 2 / 360);
	yaw = yaw * (M_PI * 2 / 360);

	return Vector(
		cos( yaw ) * sin( pitch ) * radius,
		sin( yaw ) * sin( pitch ) * radius,
		cos( pitch ) * radius
	);
}

void Vector::AngleVectors( const Vector& angles, Vector* forward, Vector* right, Vector* up )
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = angles.y * (M_PI * 2 / 360);
	sy = sin( angle );
	cy = cos( angle );

	angle = angles.x * (M_PI * 2 / 360);
	sp = sin( angle );
	cp = cos( angle );

	angle = angles.z * (M_PI * 2 / 360);
	sr = sin( angle );
	cr = cos( angle );

	if ( forward )
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if ( right )
	{
		right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
		right->y = (-1 * sr * sp * sy + -1 * cr * cy);
		right->z = -1 * sr * cp;
	}

	if ( up )
	{
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}
