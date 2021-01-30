#include "cg_local.hpp"
#include "View.hpp"

// ===================
// ViewShake::CalculateShake
// ===================
Vector ClientView::ViewShake::CalculateShake() const
{
	if ( !active )
		return Vector::Zero;

	Vector v;
	float time = GetClient()->Time();
	
	// This'll give us a value between 1.0 and 0.0
	float delta = 1.0f - ((time - timeStarted) / duration);
	Vector amplitude = direction * delta;

	// Shaking goes either in sinusoidal or cosinusoidal fashion,
	// *from* the point when the shaking started
	v = amplitude * sin( (time - timeStarted) * frequency * (M_PI*2.0f) );

	return v;
}

// ===================
// ViewShake::Update
// ===================
void ClientView::ViewShake::Update()
{
	if ( !active )
		return;

	if ( GetClient()->Time() >= (timeStarted + duration) )
	{
		active = false;
		return;
	}
}

// ===================
// ClientView::ctor
// ===================
ClientView::ClientView()
{
	memset( shakes, 0, sizeof( shakes ) );
}

// ===================
// ClientView::CalculateViewTransform
// ===================
void ClientView::CalculateViewTransform( Vector& outOrigin, Vector& outAngles )
{
	outOrigin = cg.refdef.vieworg;
	outAngles = cg.refdefViewAngles;

	Vector forward, right, up;
	Vector::AngleVectors( outAngles, &forward, &right, &up );
	Vector velocity = cg.predictedPlayerState.velocity;

	static float speed = 0.0f;
	
	float targetSpeed = velocity.Length2D() / 150.0f;
	float time = GetClient()->Time();
	bool air = cg.predictedPlayerState.groundEntityNum == ENTITYNUM_NONE;

	if ( air )
		targetSpeed = 0.0f;

	speed = speed * 0.9f + targetSpeed * 0.1f;

	float upBob = speed;
	float sideBob = upBob * 1.5f;

	upBob *= sin( time * 16.0f );
	sideBob *= sin( time * 8.0f );

	for ( ViewShake& vs : shakes )
		vs.Update();

	Vector shake = CalculateShakeAverage();

	outOrigin += up * upBob;
	outOrigin += right * sideBob * 0.5f;
	outOrigin += shake;

	currentViewOrigin = outOrigin;
	currentViewAngles = outAngles;
}

// ===================
// ClientView::CalculateWeaponTransform
// ===================
void ClientView::CalculateWeaponTransform( Vector& outOrigin, Vector& outAngles )
{
	outOrigin = cg.refdef.vieworg;
	outAngles = cg.refdefViewAngles;

	Vector forward, right, up;
	Vector::AngleVectors( outAngles, &forward, &right, &up );
	Vector velocity = cg.predictedPlayerState.velocity;
	
	static float speed = 0.0f;
	static float airAngle = 0.0f;
	static short air = 0;

	static float dotForward = 0.0f;
	static float dotRight = 0.0f;
	static float dotUp = 0.0f;

	float targetSpeed = velocity.Length2D() / 350.0f;
	float time = GetClient()->Time();
	float airOffset = 0.0f;

	Vector dotVector( forward * velocity, right * velocity, up * velocity );
	dotVector /= 320.0f;
	if ( dotVector.Length() > 1.0f )
		dotVector.Normalize();

	bool targetAir = cg.predictedPlayerState.groundEntityNum == ENTITYNUM_NONE;

	// This is ugly :(
	// Pls fix in BUREKTech 0.2
	usercmd_t uc;
	trap_GetUserCmd( trap_GetCurrentCmdNumber(), &uc );

	if ( targetAir )
	{
		air += cg.frametime;
		if ( uc.upmove > 0 )
			air = 255;
	}
	else
	{
		air -= cg.frametime;
	}

	air = MIN( MAX( 0, air ), 255 );

	if ( air >= 128 )
	{
		targetSpeed = 0.2f;
		airAngle += cg.frametime * 0.001f * 10.0f;
	}
	else
	{
		airAngle -= cg.frametime * 0.001f * 10.0f;
	}

	if ( airAngle > M_PI * 1.25f )
		airAngle = M_PI * 1.25f;
	else if ( airAngle < 0.0f )
		airAngle = 0;

	airOffset = sin( airAngle ) * -5.0f;

	// Lerping static floats and stuff
	speed = speed * 0.9f + targetSpeed * 0.1f;
	dotForward = dotForward * 0.95f + dotVector.x * 0.05f;
	dotRight = dotRight * 0.95f + dotVector.y * 0.05f;
	dotUp = dotUp * 0.95f + dotVector.z * 0.05f;

	float upBob = speed * 0.25f;
	float sideBob = speed * 0.5f;

	upBob *= sin( time * 16.0f );
	sideBob *= sin( time * 8.0f );

	Vector shake = CalculateShakeAverage();

	outOrigin += up * upBob;
	outOrigin += right * sideBob;
	outOrigin.z += airOffset * 0.1f;

	outOrigin += forward * dotForward;
	outOrigin += right * dotRight;
	outOrigin += up * -dotUp;

	outOrigin += shake * -0.1f;

	outOrigin.z -= fabs( cg.refdefViewAngles[PITCH] ) / 90.0f;

	outAngles.x -= airOffset;

	currentWeaponOrigin = outOrigin;
	currentWeaponAngles = outAngles;
}

// ===================
// ClientView::AddShake
// ===================
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
		break;
	}
}

// ===================
// ClientView::CalculateShakeAverage
// ===================
Vector ClientView::CalculateShakeAverage() const
{
	Vector shake = Vector::Zero;

	for ( const ViewShake& vs : shakes )
	{
		if ( !vs.active )
			continue;

		shake += vs.CalculateShake();
	}

	return shake;
}
