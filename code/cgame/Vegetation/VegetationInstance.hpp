#pragma once

class VegetationInstance
{
public:
	VegetationInstance();
	VegetationInstance( const qhandle_t& model, 
						const Vector& position, 
						const Vector& angles, 
						const float& scale = 1.0f, 
						const float& distance = 4096.0f, 
						const float& wind = 0.0f );

	void		Clear();
	void		Render();
	void		PositionOnGround();

	static float CalculateWindPhase( const Vector& position );

	bool		active{ false };

private:
	Vector		instancePosition{ Vector::Zero };
	Vector		instancePVSPosition{ Vector::Identity };
	float		instanceScale{ 1.0f };
	Vector		instanceAxis[3];
	float		maxDistance{ 2048.0f };
	float		windFactor{ 0.0f };
	float		windPhase{ 0.0f };
	refEntity_t re;
};
