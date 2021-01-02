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

	GetState()->eType = ET_MOVER;
	bobIntensity = spawnArgs->GetFloat( "bobIntensity", 64.0f );

	trajectory_t* tr = &GetState()->pos;
	tr->trType = TR_SINE;
	tr->trTime = 0;
	tr->trDuration = 10000;

	GetOrigin().CopyToArray( tr->trBase );
	SetCurrentOrigin( GetOrigin() );
	SetOrigin( Vector::Zero );
	Vector( 0, 0, bobIntensity ).CopyToArray( tr->trDelta );

	gameImports->LinkEntity( this );
	//GetState()->clipFlags |= ClipFlag_ManualAbsoluteBox;

	for ( int i = 0; i < 3; i++ )
	{
		GetShared()->absmax[i] += bobIntensity * 2.0f;
		GetShared()->absmin[i] -= bobIntensity * 2.0f;
	}
}

void FuncBobbing::Think()
{
	BaseMover::Think();
}
