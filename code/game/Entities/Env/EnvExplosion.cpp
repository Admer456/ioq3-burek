#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "EnvExplosion.hpp"

using namespace Entities;

DefineEntityClass( "env_explosion", EnvExplosion, BaseEntity );

void EnvExplosion::Spawn()
{
	BaseEntity::Spawn();

	explosionDamage = spawnArgs->GetFloat( "damage", 200.0f );
	explosionRadius = spawnArgs->GetFloat( "radius", 200.0f );
	spawnFlags = spawnArgs->GetInt( "spawnflags", 0 );
}

void EnvExplosion::Precache()
{
	explosionSound = gameWorld->PrecacheSound( "sound/debris/explode1.wav" );
}

void EnvExplosion::Use( IEntity* activator, IEntity* caller, float value )
{
	this->activator = static_cast<BaseEntity*>( activator );
	Explode();
}

void EnvExplosion::Explode()
{
	// If it's not repeatable and we've already exploded, then don't do it again
	if ( !(spawnFlags & SF_Repeatable) && exploded )
		return;

	EventData ed;
	ed.id = CE_Explosion;
	ed.fparm = explosionRadius;
	ed.parm = 0;
	ed.parm2 = 0; // use builtin explosion preset 1
	ed.vparm = Vector( 0, 0, 1 );
	ed.sound = explosionSound;

	// Create the explosion
	gameWorld->EmitComplexEvent( GetOrigin(), Vector::Zero, ed );

	// Apply radius damage
	if ( !(spawnFlags & SF_NoDamage) )
	{
		RadiusDamage( explosionDamage, explosionRadius );
	}

	exploded = true;
}

void EnvExplosion::RadiusDamage( float damageAmount, float radius )
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
