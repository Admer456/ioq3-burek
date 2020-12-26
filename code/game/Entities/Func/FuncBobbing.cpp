#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"
#include "Entities/Base/BaseMover.hpp"

#include "FuncBobbing.hpp"

using namespace Entities;

DefineEntityClass( "func_bobbing", FuncBobbing, BaseMover );

void FuncBobbing::Spawn()
{
	BaseMover::Spawn();

	bobIntensity = spawnArgs->GetFloat( "bobIntensity", 16.0f );

	//GetShared()->svFlags &= ~SVF_USE_CURRENT_ORIGIN;

	trajectory_t* tr = &GetState()->pos;

	tr->trType = TR_SINE;
	tr->trTime = level.time;
	tr->trDuration = 1000;

	GetOrigin().CopyToArray( tr->trBase );
	Vector( 0, 0, bobIntensity ).CopyToArray( tr->trDelta );
}

void FuncBobbing::Think()
{
	BaseMover::Think();

	Vector result;
	BG_EvaluateTrajectory( &GetState()->pos, level.time, result );
	SetOrigin( result );
}
