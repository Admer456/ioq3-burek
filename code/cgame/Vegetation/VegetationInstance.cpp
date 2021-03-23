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
	instancePVSPosition = position + Vector( 0, 0, 32 ); // Raise the "VIS" position a bit above ground, because many trees can be partially inside the ground
	Vector::AngleVectors( angles, &instanceAxis[0], &instanceAxis[1], &instanceAxis[2] );
	instanceAxis[1] = instanceAxis[1] * -1;
	for ( Vector& v : instanceAxis )
		v *= scale;

	maxDistance = distance;
	windFactor = wind;

	memset( &re, 0, sizeof( re ) );

	re.reType = RT_MODEL;
	re.origin << instancePosition;
	re.axis[0] << instanceAxis[0];
	re.axis[1] << instanceAxis[1];
	re.axis[2] << instanceAxis[2];
	re.hModel = model;

	if ( scale != 1.0f )
		re.nonNormalizedAxes = true;

	active = true;
}

void VegetationInstance::Clear()
{
	active = false;
}

void VegetationInstance::Render()
{
	// If too far away, don't render
	if ( (GetClient()->GetView()->GetViewOrigin() - instancePVSPosition).Length2D() > maxDistance )
		return;

	// Check if in PVS
	if ( !trap_R_inPVS( VegetationSystem::playerEyePosition, instancePVSPosition ) )
		return;

	// Render it!
	trap_R_AddRefEntityToScene( &re );
}
