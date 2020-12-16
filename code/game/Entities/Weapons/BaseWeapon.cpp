#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"

#include "BaseWeapon.hpp"

using namespace Entities;

DefineEntityClass( "weapon_test", BaseWeapon, BaseQuakeEntity );

Entities::BaseWeapon::WeaponInfo BaseWeapon::GetWeaponInfo()
{
	WeaponInfo info;

	info.worldModel = "models/weapons/w_test.iqm";
	info.viewModel = "models/weapons/v_test.iqm";

	return info;
}

void BaseWeapon::Spawn()
{
	BaseQuakeEntity::Spawn();

	SetTouch( &BaseWeapon::WeaponTouch );

	const char* model = GetWeaponInfo().worldModel;
	int modelIndex = G_ModelIndex( const_cast<char*>(model) );

	GetState()->modelindex = modelIndex;
	Vector( 16, 16, 16 ).CopyToArray( GetShared()->maxs );
	Vector( -16, -16, 0 ).CopyToArray( GetShared()->mins );
	gameImports->LinkEntity( this );

	shared.r.contents |= CONTENTS_TRIGGER;
}

void BaseWeapon::WeaponTouch( IEntity* other, trace_t* trace )
{
	engine->Print( "BaseWeapon::WeaponTouch\n" );

	// Only players can pick up weapons by touching them
	if ( !other->IsClass( BasePlayer::ClassInfo ) )
		return;


	GetState()->modelindex = 0;
	SetTouch( nullptr );
	gameImports->UnlinkEntity( this );

	player = static_cast<BasePlayer*>( other );
	//player->AddWeapon( this );
}

BasePlayer* BaseWeapon::GetPlayer()
{
	return player;
}
