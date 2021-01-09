#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"
#include "Entities/Base/BaseMover.hpp"
#include "Entities/TestModel.hpp"

#include "FuncBobbing.hpp"

using namespace Entities;

DefineEntityClass( "func_bobbing", FuncBobbing, BaseMover );

void FuncBobbing::Spawn()
{
	BaseMover::Spawn();

	bobIntensity = spawnArgs->GetFloat( "bobIntensity", 64.0f );
	bobDuration = spawnArgs->GetFloat( "bobDuration", 5.0f );
	bobAxis = spawnArgs->GetVector( "bobAxis", Vector(0,0,1) );
	bobPhase = spawnArgs->GetFloat( "bobPhase", 0.0f );

	trajectory_t* tr = &GetState()->pos;
	tr->trType = TR_SINE;
	tr->trTime = 1000 * bobDuration * bobPhase;
	tr->trDuration = 1000 * bobDuration;

	(bobAxis * bobIntensity).CopyToArray( tr->trDelta );

	for ( int i = 0; i < 3; i++ )
	{
		GetShared()->absmax[i] += bobAxis[i] * bobIntensity;
		GetShared()->absmin[i] -= bobAxis[i] * bobIntensity;
	}
}

void FuncBobbing::Think()
{
	BaseMover::Think();
	//SetOrigin( GetCurrentOrigin() );
	//gameImports->LinkEntity( this );
}
