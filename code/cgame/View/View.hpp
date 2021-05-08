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

	struct ViewPunch final
	{
	public:
		Vector	Calculate() const;
		void	Update();

		float	duration{ 0.0f };
		Vector	angles{ Vector::Zero };
		float	timeStarted{ 0.0f };
		bool	active{ false };
	};

	constexpr static size_t MaxViewShakes = 32U;

public:
	ClientView();

	// Calculating the view stuff includes view bobbing etc.
	void		CalculateViewTransform( Vector& outOrigin, Vector& outAngles );
	// Calculating weapon stuff includes weapon bobbing, swaying etc.
	void		CalculateWeaponTransform( Vector& outOrigin, Vector& outAngles );
	// Adds a view shake - every time a quake happens, explosions etc.
	void		AddShake( float frequency, float duration, Vector direction );
	// Gets the average of all shakes
	Vector		CalculateShakeAverage() const;
	// Adds a view punch - every time a weapon is fired etc.
	void		AddPunch( float duration, Vector angles );
	// Gets the total of all view punches
	Vector		CalculatePunchTotal() const;
	// Is the player's view currently under water?
	bool		IsInWater() const;

	const Vector& GetViewOrigin() const { return currentViewOrigin; }
	const Vector& GetViewAngles() const { return currentViewAngles; }
	const Vector& GetWeaponOrigin() const { return currentWeaponOrigin; }
	const Vector& GetWeaponAngles() const { return currentWeaponAngles; }

	const Vector& GetViewOriginOffset() const { return viewOriginOffset; }
	const Vector& GetViewAnglesOffset() const { return viewAnglesOffset; }
	const Vector& GetWeaponOriginOffset() const { return weaponOriginOffset; }
	const Vector& GetWeaponAnglesOffset() const { return weaponAnglesOffset; }

private:
	ViewShake	shakes[MaxViewShakes];
	ViewPunch	punches[MaxViewShakes];

	Vector		currentViewOrigin;
	Vector		currentViewAngles;
	Vector		currentWeaponOrigin;
	Vector		currentWeaponAngles;

	Vector		viewOriginOffset;
	Vector		viewAnglesOffset;
	Vector		weaponOriginOffset;
	Vector		weaponAnglesOffset;
};
