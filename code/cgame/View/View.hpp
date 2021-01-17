#pragma once

class ClientView final
{
	struct ViewShake final
	{
	public:
		Vector	CalculateShake() const;
		void	Update();

		float	frequency{ 0.0f };
		float	duration{ 0.0f };
		Vector	direction{ Vector::Zero };
		float	timeStarted{ 0.0f };
		bool	active{ false };
	};

	constexpr static size_t MaxViewShakes = 16U;

public:
	ClientView();

	// Calculating the view stuff includes view bobbing etc.
	void		CalculateViewTransform( Vector& outOrigin, Vector& outAngles );
	// Calculating weapon stuff includes weapon bobbing, swaying etc.
	void		CalculateWeaponTransform( Vector& outOrigin, Vector& outAngles );
	// Adds a view shake - every time a quake happens, explosions etc.
	void		AddShake( float frequency, float duration, Vector direction );

private:
	ViewShake	shakes[MaxViewShakes];
};
