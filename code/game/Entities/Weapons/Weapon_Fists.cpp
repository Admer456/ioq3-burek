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

// ===================
// Weapon_Fists::GetWeaponInfo
// ===================
BaseWeapon::WeaponInfo Weapon_Fists::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.worldModel = "models/hands/v_hands.iqm";
	wi.weaponID = WeaponID_Fists;

	return wi;
}

// ===================
// Weapon_Fists::Precache
// ===================
void Weapon_Fists::Precache()
{
	BaseWeapon::Precache();

	PunchSounds[0] = gameWorld->PrecacheSound( "sound/weapons/fist1.wav" );
	PunchSounds[1] = gameWorld->PrecacheSound( "sound/weapons/fist2.wav" );
	PunchSounds[2] = gameWorld->PrecacheSound( "sound/weapons/fist3.wav" );
	PunchSounds[3] = gameWorld->PrecacheSound( "sound/weapons/fistblock1.wav" );
	PunchSounds[4] = gameWorld->PrecacheSound( "sound/weapons/fistblock2.wav" );
	PunchSounds[5] = gameWorld->PrecacheSound( "sound/weapons/fistblock3.wav" );
}

// ===================
// Weapon_Fists::PrimaryAttack
// ===================
void Weapon_Fists::PrimaryAttack()
{
	if ( level.time * 0.001f < nextPrimary )
		return;

	Swing( 10.0f );
	nextPrimary = nextSecondary = level.time * 0.001f + 0.4f;
}

// ===================
// Weapon_Fists::SecondaryAttack
// ===================
void Weapon_Fists::SecondaryAttack()
{
	if ( level.time * 0.001f < nextSecondary )
		return;

	Swing( 30.0f );
	nextPrimary = nextSecondary = level.time * 0.001f + 1.0f;
}

// ===================
// Weapon_Fists::Swing
// ===================
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

	IEntity* temp = gameWorld->CreateTempEntity( tr.endpos, EV_GENERAL_SOUND );

	if ( tr.entityNum >= ENTITYNUM_MAX_NORMAL )
	{
		temp->GetState()->eventParm = PunchSounds[3 + rand() % 3];
		return;
	}

	temp->GetState()->eventParm = PunchSounds[rand() % 3];

	IEntity* ent = gEntities[tr.entityNum];
	if ( nullptr == ent )
		return;

	ent->TakeDamage( player, player, DamageFlags::Punch, damage );
}
