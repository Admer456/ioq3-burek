#pragma once

class ClientView final
{
	struct ViewShake
	{
		Vector	CalculateShake() const;
		void	Update();

		float	frequency;
		float	duration;
		Vector	direction;
		float	timeStarted;
		bool	active;
	};

	constexpr static size_t MaxViewShakes = 32U;

public:
	ClientView();

	void		CalculateWeaponPosition( Vector& outOrigin, Vector& outAngles );
	void		AddShake( float frequency, float duration, Vector direction );

private:
	ViewShake	shakes[MaxViewShakes];
};
