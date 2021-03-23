#pragma once

class VegetationInstance
{
public:
	VegetationInstance();
	VegetationInstance( const qhandle_t& model, 
						const Vector& position, 
						const Vector& angles, 
						const float& scale = 1.0f, 
						const float& distance = 1024.0f, 
						const float& wind = 0.0f );

	void		Clear();
	void		Render();

	bool		active{ false };

private:
	Vector		instancePosition{ Vector::Zero };
	Vector		instancePVSPosition{ Vector::Zero };
	Vector		instanceAxis[3];
	float		maxDistance{ 1024.0f };
	float		windFactor{ 0.0f };
	refEntity_t re;
};
