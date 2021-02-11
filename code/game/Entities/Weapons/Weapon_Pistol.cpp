#include "Game/g_local.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "../BaseEntity.hpp"
#include "../BasePlayer.hpp"
#include "BaseWeapon.hpp"

#include "Weapon_Pistol.hpp"

using namespace Entities;

DefineEntityClass( "weapon_pistol", Weapon_Pistol, BaseWeapon );

// ===================
// Weapon_Pistol::GetWeaponInfo
// ===================
BaseWeapon::WeaponInfo Weapon_Pistol::GetWeaponInfo()
{
	WeaponInfo wi;

	wi.viewModel = "models/weapons/v_pistol.iqm";
	wi.worldModel = "models/weapons/w_pistol.iqm";
	wi.weaponID = WeaponID_Pistol;

	return wi;
}

// ===================
// Weapon_Pistol::Idle
// ===================
void Weapon_Pistol::Idle()
{
	if ( !(player->GetClient()->interactionButtons & Interaction_PrimaryAttack) )
		fireHeld = false;
}

// ===================
// Weapon_Pistol::PrimaryAttack
// ===================
void Weapon_Pistol::PrimaryAttack()
{
	if ( level.time * 0.001f < nextPrimary )
		return;

	if ( fireHeld )
		return;

	fireHeld = true;

	int& ammoMag = player->GetClient()->ps.ammoMag[WeaponID_Pistol];

	if ( ammoMag <= 0 )
	{
		ammoMag = 0; // just in case it somehow becomes negative
		return;
	}

	ammoMag--;

	Shoot();

	nextPrimary = nextSecondary = nextIdle = nextReload = level.time * 0.001f;
}

// ===================
// Weapon_Pistol::Reload
// ===================
void Weapon_Pistol::Reload()
{
	if ( level.time * 0.001f < nextReload )
		return;

	int need = 15;
	int& have = player->GetClient()->ps.ammoMag[WeaponID_Pistol];
	int& reserve = player->GetClient()->ps.ammo[WeaponID_Pistol];
	float delay;

	// 15 == 15
	if ( need == have )
		return;

	// if I have 13, then I need 2
	need = need - have;

	// if we have 1 in the ammo reserve, but we need 2, then we can't take more than 1
	if ( reserve < need )
		need = reserve;

	// take 2 from the ammo reserve
	reserve -= need;
	have += need;

	// tactical reload
	if ( have )
		delay = 2.8347f; // the tactical reload lasts shorter
	else
		delay = 2.8347f;
		
	nextReload = nextPrimary = nextSecondary = nextIdle = level.time * 0.001f + delay;
}

// ===================
// Weapon_Pistol::Shoot
// ===================
void Weapon_Pistol::Shoot()
{
	trace_t tr;
	Vector randomVector = Vector( crandom(), crandom(), crandom() ) * 0.03f;
	Vector forward;
	Vector::AngleVectors( player->GetClientViewAngle(), &forward, nullptr, nullptr );

	Vector start, end;
	start = player->GetViewOrigin();
	end = start + (forward + randomVector) * 4096.0f;

	gameImports->Trace( &tr, start, nullptr, nullptr, end, player->GetEntityIndex(), MASK_SHOT );

	if ( tr.fraction == 1.0f )
		return;

	Vector hitPoint = tr.endpos;
	EventData ed;
	ed.id = CE_Explosion;
	ed.fparm = 8.0f;
	ed.parm2 = 1;
	ed.vparm = Vector::Zero;
	gameWorld->EmitComplexEvent( hitPoint, Vector::Zero, ed );

	if ( tr.entityNum >= ENTITYNUM_MAX_NORMAL )
		return;

	IEntity* ent = gEntities[tr.entityNum];
	if ( nullptr == ent )
		return;

	ent->TakeDamage( this, this, 0, 40.0f );
}
