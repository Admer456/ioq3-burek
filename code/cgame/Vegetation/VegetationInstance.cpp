#include "cg_local.hpp"
#include "VegetationInstance.hpp"
#include "VegetationSystem.hpp"

VegetationInstance::VegetationInstance()
{
	instanceAxis[0] = Vector( 1, 0, 0 );
	instanceAxis[1] = Vector( 0, 1, 0 );
	instanceAxis[2] = Vector( 0, 0, 1 );

	memset( &re, 0, sizeof( re ) );
}

VegetationInstance::VegetationInstance(
	const qhandle_t& model,
	const Vector& position,
	const Vector& angles,
	const float& scale,
	const float& distance,
	const float& wind )
{
	instancePosition = position;
	instancePVSPosition = Vector::Identity;
	Vector::AngleVectors( angles, &instanceAxis[0], &instanceAxis[1], &instanceAxis[2] );
	instanceAxis[1] = instanceAxis[1] * -1;
	for ( Vector& v : instanceAxis )
		v *= scale;

	instanceScale = scale;

	maxDistance = distance * scale;
	windFactor = wind;

	memset( &re, 0, sizeof( re ) );

	re.reType = RT_MODEL;
	re.origin << instancePosition;
	re.axis[0] << instanceAxis[0];
	re.axis[1] << instanceAxis[1];
	re.axis[2] << instanceAxis[2];
	re.hModel = model;

	if ( scale != 1.0f )
	{
		re.nonNormalizedAxes = true;
		windFactor /= scale; // Decrease wind sway the bigger you get
	}

	active = true;

	windPhase = VegetationInstance::CalculateWindPhase( position );
}

void VegetationInstance::Clear()
{
	active = false;
}

void VegetationInstance::Render()
{
	if ( instancePVSPosition == Vector::Identity )
		PositionOnGround();

	// If too far away, don't render
	float distanceFromView = (GetClient()->GetView()->GetViewOrigin() - instancePVSPosition).Length();
	if ( distanceFromView > maxDistance )
		return;

	// Check if in PVS
	if ( !trap_R_inPVS( VegetationSystem::playerEyePosition, instancePVSPosition ) )
		return;

	// If close enough, do a swaying animation
	if ( distanceFromView < (maxDistance * 0.5f) )
	{
		float cycle = GetClient()->Time() * 0.5f * (1.0f / instanceScale) + (windPhase * M_PI);
		float wind = windFactor * 0.0005f;

		// Some gentle swaying for leaves
		re.axis[0][2] = instanceAxis[0].z + (sin( cycle * 21.3452f ) - sin( cycle * 17.2f )) * sin( cycle * 1.651f ) * wind * 0.65f;
		re.axis[1][2] = instanceAxis[0].z + (cos( cycle * 22.6654f ) - cos( cycle * 18.4f )) * cos( cycle * 1.652f ) * wind * 0.65f;

		// Some subtle, tiny shaking
		re.axis[0][2] += sin( cycle * 51.53f ) * wind * 0.07f;
		re.axis[1][2] += cos( cycle * 53.24f ) * wind * 0.07f;

		// Oscillation of the upper pivot, moves tree trunks too, so keep it slow and low!
		re.axis[2][0] = instanceAxis[2].x + sin( cycle * 0.5f ) * wind;
		re.axis[2][1] = instanceAxis[2].y + cos( cycle * 0.5f ) * wind * 0.5f;
	}

	// Render it!
	trap_R_AddRefEntityToScene( &re );
}

void VegetationInstance::PositionOnGround()
{
	Vector start = instancePosition;
	Vector end = start - Vector( 0, 0, 4096 );
	trace_t tr;

	CG_Trace( &tr, start, Vector::Zero, Vector::Zero, end, -1, MASK_SOLID );

	// Stuck somewhere, delet
	if ( tr.startsolid )
	{
		Clear();
		return;
	}

	instancePosition = Vector( tr.endpos ) - Vector( 0, 0, 8 );
	// Raise the "VIS" position a bit above ground, because many trees can be partially inside the ground
	instancePVSPosition = instancePosition + Vector( 0, 0, 64 );

	re.origin << instancePosition;
	re.lightingOrigin << instancePVSPosition;
}

float VegetationInstance::CalculateWindPhase( const Vector& position )
{
	float x, y, phase, r;
	x = position.x * 0.1f;
	y = position.y * 0.1f;
	r = random();

	if ( r < 0.333f )
	{
		phase = r;
	}
	else// Nice wavy pattern, try this out in GeoGebra!
	{	// z = sin(x+sin(0.5 y)) sin(y+sin(0.5 x))*0.5+0.5
		phase = sinf( x + sinf( y * 0.5f ) ) * sinf( y + sinf( x * 0.5f ) ) * 0.5f + 0.5f;
	}

	return phase;
}
