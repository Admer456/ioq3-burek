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
// Weapon_Pistol::Precache
// ===================
void Weapon_Pistol::Precache()
{
	BaseWeapon::Precache();

	BulletHoleDecals[0] = gameWorld->PrecacheMaterial( "decals/bullethole1" );
	BulletHoleDecals[1] = gameWorld->PrecacheMaterial( "decals/bullethole2" );
	BulletHoleDecals[2] = gameWorld->PrecacheMaterial( "decals/bullethole3" );

	BulletHoleDecals[3] = gameWorld->PrecacheMaterial( "decals/bullethole_glass1" );
	BulletHoleDecals[4] = gameWorld->PrecacheMaterial( "decals/bullethole_glass2" );
	BulletHoleDecals[5] = gameWorld->PrecacheMaterial( "decals/bullethole_glass3" );

	LaserSounds[LaserOn] = gameWorld->PrecacheSound( "sound/weapons/laseron.wav" );
	LaserSounds[LaserOff] = gameWorld->PrecacheSound( "sound/weapons/laseroff.wav" );
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
		nextPrimary = level.time * 0.001f + 0.1f;
	}
	else
	{
		ammoMag--;
		Shoot();
		nextPrimary = level.time * 0.001f;
	}

	nextSecondary = nextIdle = nextReload = nextPrimary;
}

// ===================
// Weapon_Pistol::SecondaryAttack
// ===================
void Weapon_Pistol::SecondaryAttack()
{
	if ( level.time * 0.001f < nextSecondary )
		return;

	playerState_t& ps = player->GetClient()->ps;
	IEntity* temp = gameWorld->CreateTempEntity( ps.origin, EV_GENERAL_SOUND );
	
	if ( ps.weaponFlags & PlayerWeaponFlags::PistolLaser )
	{	// It's about to be turned off
		temp->GetState()->eventParm = LaserSounds[LaserOff];
	}
	else
	{	// It's about to be turned on
		temp->GetState()->eventParm = LaserSounds[LaserOn];
	}

	temp->GetShared()->svFlags |= SVF_NOTSINGLECLIENT;
	temp->GetShared()->singleClient = ps.clientNum;

	ps.weaponFlags ^= PlayerWeaponFlags::PistolLaser;

	nextSecondary = level.time * 0.001f + 0.5f;
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

	// 15 == 13
	if ( need != have )
	{
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
			delay = 2.66f; // the tactical reload lasts shorter
		else
			delay = 2.66f;

		nextReload = nextPrimary = nextSecondary = nextIdle = level.time * 0.001f + delay;
	}
}

// ===================
// Weapon_Pistol::Shoot
// ===================
void Weapon_Pistol::Shoot()
{
	trace_t tr;
	Vector randomVector = Vector( crandom(), crandom(), crandom() ) * 0.01f;
	Vector forward;
	Vector::AngleVectors( player->GetClientViewAngle(), &forward, nullptr, nullptr );

	Vector start, end;
	start = player->GetViewOrigin();
	end = start + (forward + randomVector) * 4096.0f;

	gameImports->Trace( &tr, start, nullptr, nullptr, end, player->GetEntityIndex(), MASK_SHOT );

	engine->Print( va( "tr.entityNum = %i; tr.fraction = %1.3f; ENTITYNUM_WORLD = %i\n", tr.entityNum, tr.fraction, ENTITYNUM_WORLD ) );

	if ( tr.entityNum == ENTITYNUM_NONE || tr.fraction == 1.0f )
		return;

	IEntity* ent = gEntities[tr.entityNum];
	if ( nullptr == ent )
		return;

	float bulletDamage = 40.0f;

	// Damage the entity if applicable
	ent->TakeDamage( player, player, 0, bulletDamage );

	// Add a bullet hole if it's a brush or worldspawn
	if ( (ent->GetState()->solid == SOLID_BMODEL) || (ent->GetEntityIndex() == ENTITYNUM_WORLD) )
	{
		bool isGlass = (tr.surfaceFlags & SURF_GLASS) ? true : false;

		// Offset the bullet hole position slightly away from the surface, 0.25u is enough
		Vector bulletHoleOrigin = Vector( tr.endpos ) - forward * 0.25f;

		if ( ~tr.surfaceFlags & SURF_SKY )
		{
			AddBulletHole( tr.plane.normal, forward, bulletHoleOrigin, isGlass );
		}
	}

	// Try going through the wall if thin enough
	testThroughWallCounter = 0;
	TestThroughWall( &tr, (forward + randomVector), bulletDamage );
}

// ===================
// Weapon_Pistol::TestThroughWall
// ===================
void Weapon_Pistol::TestThroughWall( trace_t* tr, Vector direction, float damage )
{
	if ( testThroughWallCounter > 8 || damage <= 0.0f )
	{
		return;
	}

	Vector entryPoint = tr->endpos;
	Vector exitPoint = entryPoint + direction * 12.0f;

	// Trace in reverse from the imaginary exit point
	gameImports->Trace( tr, exitPoint, nullptr, nullptr, entryPoint, player->GetEntityIndex(), MASK_SHOT );

	// Stop right there, the wall is too thick
	if ( tr->startsolid )
	{
		return;
	}

	testThroughWallCounter++;

	// Calculate the depth of the wall by fraction
	float depth = 12.0f - (tr->fraction * 12.0f);

	// Reduce damage
	damage /= (1.0f + depth * 0.125f);

	// Calculate the actual exit point and add a bullet hole there
	Vector realExitPoint = Vector( tr->endpos ) + direction * 0.25f;
	AddBulletHole( tr->plane.normal, direction, realExitPoint - direction * 0.25f, (tr->surfaceFlags & SURF_GLASS) );

	// Trace from the actual exit point
	gameImports->Trace( tr, realExitPoint, nullptr, nullptr, realExitPoint + direction * 4096.0f, player->GetEntityIndex(), MASK_SHOT );

	IEntity* ent = gEntities[tr->entityNum];
	if ( nullptr == ent || tr->fraction == 1.0f )
	{
		return;
	}

	// Add a bullet hole if it's a brush or worldspawn
	if ( (ent->GetState()->solid == SOLID_BMODEL) || (ent->GetEntityIndex() == ENTITYNUM_WORLD) )
	{
		bool isGlass = (tr->surfaceFlags & SURF_GLASS) ? true : false;

		// Offset the bullet hole position slightly away from the surface, 0.25u is enough
		Vector bulletHoleOrigin = Vector( tr->endpos ) - direction * 0.25f;

		if ( ~tr->surfaceFlags & SURF_SKY )
		{
			AddBulletHole( tr->plane.normal, direction, bulletHoleOrigin, isGlass );
		}
	}

	ent->TakeDamage( player, player, 0, damage );
	TestThroughWall( tr, direction, damage );
}

void Weapon_Pistol::AddBulletHole( Vector planeNormal, Vector bulletDirection, Vector origin, bool glass )
{
	// Bullet hole decal
	{
		EventData ed;
		ed.id = CE_Decal;

		// This could've been a one-liner, but it'd be even more cryptic to read
		// Ahem: ed.model = BulletHoleDecals[(rand() % 3) + (glass ? 3 : 0)];
		if ( glass )
		{
			ed.model = BulletHoleDecals[3 + rand() % 3];
		}
		else
		{
			ed.model = BulletHoleDecals[rand() % 3];
		}

		ed.vparm = planeNormal.Normalized();
		ed.fparm = random() * 360.0f;
		ed.fparm2 = glass ? 24.0f : 6.0f;

		gameWorld->EmitComplexEvent( origin, Vector::Zero, ed );
	}

	// Smoke from the bullet hole
	if ( !glass )
	{
		EventData ed;
		ed.id = CE_SmokePuff;
		
		ed.vparm = -1.0f * bulletDirection;
		ed.fparm = 40.0f;
		ed.fparm2 = 0.6f;

		gameWorld->EmitComplexEvent( origin - bulletDirection * 1.5f, Vector::Zero, ed );
	}
}
