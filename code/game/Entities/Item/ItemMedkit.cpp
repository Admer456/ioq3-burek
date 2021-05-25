#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "ItemMedkit.hpp"

using namespace Entities;

DefineEntityClass( "item_medkit", ItemMedkit, BaseEntity );

void ItemMedkit::Spawn()
{
	BaseEntity::Precache();

	SetTouch( &ItemMedkit::HealingTouch );
	SetUse( &ItemMedkit::HealingUse );

	isFood = true;
	int itemType = spawnArgs->GetInt( "type", Standard );

	switch ( itemType )
	{
	case Standard: SetModel( "models/props/medkit.iqm" ); isFood = false; break;
	case Burek: SetModel( "models/props/burek.iqm" ); break;
	case Cevapi: SetModel( "models/props/cevapi.iqm" ); break;
	default: Remove(); return;
	}

	GetShared()->maxs << Vector( 16, 16, 16 );
	GetShared()->mins << Vector( -16, -16, 0 );
	gameImports->LinkEntity( this );

	GetShared()->contents = CONTENTS_NOTTEAM1 | CONTENTS_TRIGGER;
}

void ItemMedkit::Precache()
{
	HealingSound = gameWorld->PrecacheSound( "sound/items/heal.wav" );
	EatingSound = gameWorld->PrecacheSound( "sound/items/eat.wav" );
}

void ItemMedkit::HealingTouch( IEntity* other, trace_t* trace )
{
	if ( !other->IsClass( BasePlayer::ClassInfo ) )
		return;

	BaseEntity* ent = static_cast<BaseEntity*>( other );
	int& playerHealth = ent->health;

	if ( playerHealth == 100 )
		return;

	playerHealth += HealAmount;
	if ( playerHealth > 100 )
		playerHealth = 100;

	gameWorld
		->CreateTempEntity( GetOrigin(), EV_GENERAL_SOUND )
		->GetState()->eventParm = isFood ? EatingSound : HealingSound;

	SetModel( nullptr );
	SetTouch( nullptr );
	gameImports->UnlinkEntity( this );
}

void ItemMedkit::HealingUse( IEntity* activator, IEntity* caller, float value )
{
	HealingTouch( activator, nullptr );
}
