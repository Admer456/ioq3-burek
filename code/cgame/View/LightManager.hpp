#pragma once

class Light final
{
public:
	Light();
	Light( Vector origin, Vector colour, float intensity, float duration );

	void	Update();
	void	Render();

	// Bookkeeping
	bool	active{ false };

	Vector	lightOrigin{ Vector::Zero };
	Vector	lightColour{ Vector::Zero };
	float	lightIntensity{};
	float	lightStart{};
	float	lightDuration{};
};

class LightManager final
{
public:
	LightManager();
	~LightManager();

	static constexpr size_t MaxLights = 64U;

	void	AddLight( const Light& light );
	void	Update();
	void	Render();

private:
	Light	lights[MaxLights];

};
