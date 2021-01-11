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

DefineEntityClass( "func_breakable", FuncBreakable, BaseEntity );

void FuncBreakable::Spawn()
{
	BaseEntity::Spawn();

	// Grab the current model index, so we can
	// use it again if this entity respawns
	currentModel = GetState()->modelindex;
}

void FuncBreakable::Precache()
{
	materialType = spawnArgs->GetInt( "material", Material_Concrete );

	gibModels[0] = gameWorld->PrecacheModel( BreakGibs[materialType*3] );
	gibModels[1] = gameWorld->PrecacheModel( BreakGibs[materialType*3+1] );
	gibModels[2] = gameWorld->PrecacheModel( BreakGibs[materialType*3+2] );

	sounds[0] = gameWorld->PrecacheSound( BreakSounds[materialType*3] );
	sounds[1] = gameWorld->PrecacheSound( BreakSounds[materialType*3+1] );
}

void FuncBreakable::Use( IEntity* activator, IEntity* caller, float value )
{
	Break();
	health = 0;
}

void FuncBreakable::TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage )
{
	if ( spawnFlags & SF_TriggerOnly )
		return;

	health -= damage;

	gibDirection = (GetAverageOrigin() + GetCurrentOrigin()) - inflictor->GetCurrentOrigin();

	/*
	if ( spawnFlags & SF_Twitch )
	{
		SetAngles( GetAngles() + Vector( Maths::FRandom( 0.5, -0.5 ) ) );
	}
	*/

	if ( health <= 0 )
		Break();
}

void FuncBreakable::Break()
{
	SetModel( nullptr );
	gameImports->UnlinkEntity( this );

	EventData ed;

	ed.id = CE_GibSpan;

	ed.parm = spawnArgs->GetInt( "gibs", 30 ); // 30 gibs
	ed.parm2 = 3; // 3 different models
	ed.parm3 = 0; // type 0 = model
	ed.fparm = gibDirection.ToAngles().x; // pitch
	ed.fparm2 = gibDirection.ToAngles().y; // yaw
	ed.fparm3 = spawnArgs->GetFloat( "force", -100.0f ); // force -100.0f = random dir at intensity 100

	ed.vparm = GetCurrentOrigin() + GetMins() + Vector(3,3,3);
	ed.vparm2 = GetCurrentOrigin() + GetMaxs() - Vector(3,3,3);

	ed.model = gibModels[0];

	gameWorld->EmitComplexEvent( GetOrigin(), GetAngles(), ed );
}
