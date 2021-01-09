#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../Base/BaseMover.hpp"

#include "FuncRotating.hpp"

using namespace Entities;

DefineEntityClass( "func_rotating", FuncRotating, BaseMover );

void FuncRotating::Spawn()
{
	BaseMover::Spawn();

	shared.s.apos.trType = TR_LINEAR;

	speed = spawnArgs->GetFloat( "speed", 15.0f );
	rotationDir = spawnArgs->GetVector( "dir", Vector( 0,1,0 ) );

	(rotationDir * speed).CopyToArray( GetState()->apos.trDelta );
}
