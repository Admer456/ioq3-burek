#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"
#include "BaseWeapon.hpp"

#include "Weapon_Fists.hpp"

using namespace Entities;

DefineEntityClass_NoMapSpawn( Weapon_Fists, BaseWeapon );

BaseWeapon::WeaponInfo Weapon_Fists::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.worldModel = "models/hands/v_hands.iqm";
	wi.weaponID = WeaponID_Fists;

	return wi;
}

void Weapon_Fists::PrimaryAttack()
{
	if ( level.time * 0.001f < nextPrimary )
		return;

	Swing( 10.0f );
	nextPrimary = nextSecondary = level.time * 0.001f + 0.4f;
}

void Weapon_Fists::SecondaryAttack()
{
	if ( level.time * 0.001f < nextSecondary )
		return;

	Swing( 30.0f );
	nextPrimary = nextSecondary = level.time * 0.001f + 1.0f;
}

void Weapon_Fists::Swing( float damage )
{
	trace_t tr;
	Vector forward;
	Vector::AngleVectors( player->GetClientViewAngle(), &forward, nullptr, nullptr );

	Vector start, end;
	start = player->GetViewOrigin();
	end = start + forward * 64.0f;

	gameImports->Trace( &tr, start, nullptr, nullptr, end, player->GetEntityIndex(), MASK_SHOT );

	if ( tr.fraction == 1.0f )
		return;

	if ( tr.entityNum >= ENTITYNUM_MAX_NORMAL )
		return;

	IEntity* ent = gEntities[tr.entityNum];
	if ( nullptr == ent )
		return;

	ent->TakeDamage( this, this, 0, damage );
}
