#include "cg_local.hpp"
#include "LightManager.hpp"

Light::Light()
{
	active = false;
}

Light::Light( Vector origin, Vector colour, float intensity, float duration )
{
	lightOrigin = origin;
	lightColour = colour;
	lightIntensity = intensity;
	lightStart = GetClient()->Time();
	lightDuration = duration;

	active = false;
}

void Light::Update()
{
	if ( !active )
		return;

	float time = GetClient()->Time();
	float deathTime = lightStart + lightDuration;

	// Negative light duration = infinite
	if ( lightDuration < 0 )
		return;

	if ( time >= deathTime )
		active = false;
}

void Light::Render()
{
	if ( !active )
		return;

	float time = GetClient()->Time();
	float fraction = (time - lightStart) / lightDuration;

	trap_R_AddAdditiveLightToScene( lightOrigin, lightIntensity * fraction, lightColour.x, lightColour.y, lightColour.z );
}

LightManager::LightManager()
{

}

LightManager::~LightManager()
{

}

void LightManager::AddLight( const Light& light )
{
	for ( Light& l : lights )
	{
		if ( !l.active )
		{
			l = light;
			l.active = true;
		}
	}
}

void LightManager::Update()
{
	for ( auto& l : lights )
	{
		l.Update();
	}
}

void LightManager::Render()
{
	for ( auto& l : lights )
	{
		l.Render();
	}
}
