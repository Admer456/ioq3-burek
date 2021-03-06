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

// Assumes that X is between 0 and 1
// x = 0 -> y = 0
// x = 0.5 -> y = 1
// x = 1 -> y = 0
float CosWave( float x )
{
	return (-1 * cos( x * 2 * M_PI ) + 1) / 2.0f;
}

// ===================
// ViewPunch::Calculate
// ===================
Vector ClientView::ViewPunch::Calculate() const
{
	if ( !active )
		return Vector::Zero;

	Vector v;
	float time = GetClient()->Time();

	float delta = 1.0f - ((time - timeStarted) / duration);
	float wave = CosWave( delta );
	//float wave = CosWave( delta ) * CosWave( delta );
	return angles * wave;
}

// ===================
// ViewPunch::Calculate
// ===================
void ClientView::ViewPunch::Update()
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

	viewOriginOffset = Vector::Zero;
	viewAnglesOffset = Vector::Zero;

	// Base vars
	Vector forward, right, up;
	Vector::AngleVectors( outAngles, &forward, &right, &up );
	Vector velocity = cg.predictedPlayerState.velocity;
	float time = GetClient()->Time();
	bool air = cg.predictedPlayerState.groundEntityNum == ENTITYNUM_NONE;

	// Static vars
	static float speed = 0.0f;
	static float waterWobble = 0.0f;

	static float deadTime = 0.0f;

	float targetSpeed = velocity.Length2D() / 150.0f;
	float waterWobbleTarget = IsInWater() ? 1.0f : 0.0f;

	if ( air )
		targetSpeed = 0.0f;

	// Interpolations
	speed = speed * 0.9f + targetSpeed * 0.1f;
	waterWobble = waterWobble * 0.9f + waterWobbleTarget * 0.1f;

	float upBob = speed;
	float sideBob = upBob * 1.5f;

	upBob *= sin( time * 16.0f );
	sideBob *= sin( time * 8.0f );

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 && deadTime < 5.0f )
	{
		deadTime += cg.frametime * 0.001f;
	}
	else if ( cg.predictedPlayerState.stats[STAT_HEALTH] > 0 )
	{
		deadTime = 0.0f;
	}

	for ( ViewShake& vs : shakes )
		vs.Update();

	for ( ViewPunch& vp : punches )
		vp.Update();

	Vector shake = CalculateShakeAverage();
	Vector punch = CalculatePunchTotal();

	outOrigin += up * upBob;
	outOrigin += right * sideBob * 0.5f;
	outOrigin += shake;

	outAngles += punch;
	outAngles.z += sin( time * 1.5f ) * waterWobble * 3.0f;
	
	if ( deadTime )
	{
		outAngles.x = -deadTime * 20.0f;
		outAngles.z = deadTime * 2.5f;
	}

	// add view height
	outOrigin.z += cg.predictedPlayerState.viewheight;

	viewOriginOffset = outOrigin - currentViewOrigin;
	viewAnglesOffset = outAngles - currentViewAngles;

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

	const float frameTime = cg.frametime * 0.001f;

	static Vector blendMouseCoords = Vector::Zero;
	Vector mouseCoords = GetClient()->GetMouseXY() * 0.008f;

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

	Vector deltaMouseCoords = mouseCoords - blendMouseCoords;
	float deltaMouseLength = deltaMouseCoords.Length();
	blendMouseCoords += deltaMouseCoords * frameTime * 4.0f;

	Vector shake = CalculateShakeAverage();

	outOrigin += right * blendMouseCoords.x * 0.25f;
	outOrigin += up * -blendMouseCoords.y;

	outOrigin += up * upBob;
	outOrigin += right * sideBob;
	outOrigin.z += airOffset * 0.1f;

	outOrigin += forward * dotForward;
	outOrigin += right * dotRight;
	outOrigin += up * -dotUp;

	outOrigin += shake * -0.1f;

	outOrigin.z -= fabs( cg.refdefViewAngles[PITCH] ) / 90.0f;

	outAngles.x -= airOffset;

	outAngles.z += dotRight * -4.0f;
	outAngles.z += blendMouseCoords.x * 15.0f;

	if ( outAngles.z > 15.0f ) outAngles.z = 15.0f;
	if ( outAngles.z < -15.0f ) outAngles.z = -15.0f;

	weaponOriginOffset = outOrigin - currentWeaponOrigin;
	weaponAnglesOffset = outAngles - currentWeaponAngles;

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

// ===================
// ClientView::AddPunch
// ===================
void ClientView::AddPunch( float duration, Vector angles )
{
	for ( ViewPunch& vp : punches )
	{
		if ( vp.active )
			continue;

		vp.active = true;
		vp.duration = duration;
		vp.angles = angles;
		vp.timeStarted = GetClient()->Time();
		break;
	}
}

// ===================
// ClientView::CalculatePunchTotal
// ===================
Vector ClientView::CalculatePunchTotal() const
{
	Vector punch = Vector::Zero;

	for ( const ViewPunch& vp : punches )
	{
		if ( !vp.active )
			continue;

		punch += vp.Calculate();
	}

	return punch;
}

// ===================
// ClientView::IsInWater
// 
// Todo: check different water levels, maybe...?
// ===================
bool ClientView::IsInWater() const
{
	return CG_PointContents( GetViewOrigin(), cg.predictedPlayerState.clientNum ) & (CONTENTS_WATER | CONTENTS_LAVA | CONTENTS_SLIME);
}
