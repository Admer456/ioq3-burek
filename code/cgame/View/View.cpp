#include "cg_local.hpp"
#include "View.hpp"

Vector ClientView::ViewShake::CalculateShake() const
{
	Vector v;
	float time = GetClient()->Time();
	
	float delta = (time - timeStarted) / duration;
	Vector amplitude = direction * delta;

	v = amplitude * sin( time );

	return v;
}

void ClientView::ViewShake::Update()
{
	if ( !active )
		return;

	if ( GetClient()->Time() > (timeStarted + duration) )
	{
		active = false;
		return;
	}
}

void ClientView::CalculateWeaponPosition( Vector& outOrigin, Vector& outAngles )
{

}

void ClientView::AddShake( float frequency, float duration, Vector direction )
{
	for ( ViewShake& vs : shakes )
	{
		if ( vs.active )
			continue;

		vs.active = true;
		vs.frequency = frequency;
		vs.duration = duration;
		vs.direction = direction;
		vs.timeStarted = GetClient()->Time();
	}
}
