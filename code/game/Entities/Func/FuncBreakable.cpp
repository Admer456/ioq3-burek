#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"

#include "FuncBreakable.hpp"

using namespace Entities;

void FuncBreakable::Spawn()
{
	BaseQuakeEntity::Spawn();


}

void FuncBreakable::Precache()
{
	materialType = spawnArgs->GetInt( "material", Material_Glass );

	gibModels[0] = gameWorld->PrecacheModel( BreakGibs[materialType] );
	gibModels[1] = gameWorld->PrecacheModel( BreakGibs[materialType+1] );
	gibModels[2] = gameWorld->PrecacheModel( BreakGibs[materialType+2] );

	sounds[0] = gameWorld->PrecacheSound( BreakSounds[materialType] );
	sounds[1] = gameWorld->PrecacheSound( BreakSounds[materialType+1] );
}

void FuncBreakable::Use( IEntity* activator, IEntity* caller, float value )
{

}

void FuncBreakable::TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage )
{

}

void FuncBreakable::Break()
{

}
