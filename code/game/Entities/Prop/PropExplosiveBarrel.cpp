#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "Entities/Func/FuncBreakable.hpp"
#include "PropExplosiveBarrel.hpp"

using namespace Entities;

DefineEntityClass( "prop_ebarrel", PropExplosiveBarrel, BaseEntity );

void PropExplosiveBarrel::Spawn()
{
	BaseEntity::Spawn();

	SetModel( "models/props/barrel_explo.iqm" );

	GetShared()->mins << Vector( -16, -16, 0 );
	GetShared()->maxs << Vector( 16, 16, 48 );

	GetShared()->contents = CONTENTS_SOLID;

	gameImports->LinkEntity( this );

	health = 25;
}

void PropExplosiveBarrel::Precache()
{
	gibModels[0] = gameWorld->PrecacheModel( "models/gibs/metal1.iqm" );
	gibModels[1] = gameWorld->PrecacheModel( "models/gibs/metal2.iqm" );
	gibModels[2] = gameWorld->PrecacheModel( "models/gibs/metal3.iqm" );

	explosionSound = gameWorld->PrecacheSound( "sound/debris/explode1.wav" );
}

void PropExplosiveBarrel::Use( IEntity* activator, IEntity* caller, float value )
{
	if ( !GetState()->modelindex )
		return;

	TakeDamage( activator, caller, DamageFlags::Bullet, 100 );
}

void PropExplosiveBarrel::TakeDamage( IEntity* attacker, IEntity* inflictor, int damageFlags, float damage )
{
	if ( damageFlags & DamageFlags::Punch )
	{
		return;
	}

	health -= damage;

	if ( health <= 0 )
	{
		activator = static_cast<BaseEntity*>( attacker );
		Explode();
	}
}

void PropExplosiveBarrel::Explode()
{
	gameImports->UnlinkEntity( this );
	GetState()->modelindex = 0;
	GetState()->solid = 0;
	GetShared()->contents = 0;

	RadiusDamage( 220, 300 );

	// Explosion
	{
		EventData ed;
		ed.id = CE_Explosion;

		ed.fparm = 200.0f;
		ed.parm = 0;
		ed.parm2 = 0;
		ed.vparm = Vector( 0, 0, 1 );
		ed.sound = explosionSound;

		gameWorld->EmitComplexEvent( GetOrigin(), Vector::Zero, ed );
	}

	// Metal gib shower
	{
		EventData ed;
		ed.id = CE_GibSpan;

		ed.parm = 15;
		ed.parm2 = 3;
		ed.parm3 = 0;
		ed.fparm = 90.0f;
		ed.fparm2 = 0.0f;
		ed.fparm3 = -500.0f;

		ed.vparm = GetCurrentOrigin() + Vector( -16, -16, 16 );
		ed.vparm2 = GetCurrentOrigin() + Vector( 16, 16, 48 );

		ed.model = gibModels[0];
		ed.parm4 = Material_Metal;

		gameWorld->EmitComplexEvent( GetOrigin(), Vector::Zero, ed );
	}
}

void PropExplosiveBarrel::RadiusDamage( float damageAmount, float radius )
{
	int ents[GameWorld::MaxEntities];
	int entNum{ 0 };
	trace_t tr;
	IEntity* ent{ nullptr };

	Vector mins = GetCurrentOrigin() - Vector( radius, radius, radius );
	Vector maxs = GetCurrentOrigin() + Vector( radius, radius, radius );

	entNum = gameImports->EntitiesInBox( mins, maxs, ents, GameWorld::MaxEntities );

	for ( int i = 0; i < entNum; i++ )
	{
		ent = gEntities[ents[i]];
		if ( nullptr == ent )
			continue;

		// Calculate the entity's position
		// This should work for origin-less brush ents and point ents!
		Vector pos = ent->GetAverageOrigin() + ent->GetCurrentOrigin();
		float length = (GetOrigin() - pos).Length();

		// If it's too far away, don't hurt
		if ( length > radius )
			continue;

		// The explosion will damage most 
		// efficiently if you're in its very centre
		if ( length < 1.0f )
			length = 1.0f;

		// Calculate linear damage
		float damage = damageAmount * (1.0f - (length / radius));

		// If it's a point entity, check if it's obstructed by some cover
		if ( !ent->GetShared()->bmodel )
		{
			// TODO: determine *how much* the entity is in cover by randomly 
			// spreading multiple tracelines and averaging their results
			gameImports->Trace( &tr, GetCurrentOrigin(), Vector::Zero, Vector::Zero, pos, ent->GetEntityIndex(), MASK_SHOT );

			// Decrease the damage by 20x if behind cover
			if ( tr.fraction != 1.0f )
			{
				damage *= 0.05f;
			}
		}

		// Bang
		ent->TakeDamage( activator, this, 0, damage );
	}
}
