#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"

#include "BaseWeapon.hpp"

using namespace Entities;

DefineAbstractEntityClass( BaseWeapon, BaseEntity );

void BaseWeapon::Spawn()
{
	BaseEntity::Spawn();

	SetTouch( &BaseWeapon::WeaponTouch );
	SetUse( &BaseWeapon::WeaponUse );

	const char* model = GetWeaponInfo().worldModel;
	int modelIndex = G_ModelIndex( const_cast<char*>(model) );

	GetState()->modelindex = modelIndex;
	Vector( 16, 16, 16 ).CopyToArray( GetShared()->maxs );
	Vector( -16, -16, 0 ).CopyToArray( GetShared()->mins );
	gameImports->LinkEntity( this );

	GetShared()->contents = CONTENTS_NOTTEAM1 | CONTENTS_TRIGGER;

	PickupSound = gameWorld->PrecacheSound( "sound/weapons/pickup.wav" );
}

void BaseWeapon::WeaponTouch( IEntity* other, trace_t* trace )
{
	// Only players can pick up weapons by touching them
	if ( !other->IsClass( BasePlayer::ClassInfo ) )
		return;

	gameWorld
		->CreateTempEntity( GetOrigin(), EV_GENERAL_SOUND )
		->GetState()->eventParm = PickupSound;

	GetState()->modelindex = 0;
	SetTouch( nullptr );
	SetUse( nullptr );
	gameImports->UnlinkEntity( this );

	player = static_cast<BasePlayer*>( other );
	player->AddWeapon( this );
}

void BaseWeapon::WeaponUse( IEntity* activator, IEntity* caller, float value )
{
	WeaponTouch( activator, nullptr );
}

BasePlayer* BaseWeapon::GetPlayer()
{
	return player;
}

void BaseWeapon::SetPlayer( BasePlayer* player )
{
	this->player = player;
}

void BaseWeapon::WeaponFrame()
{
	if ( level.time * 0.001f > nextIdle )
		Idle();
}
