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

	// Create the explosion
	// EV_MISSILE_HIT uses bytedirs indices as the parameter,
	// so the 5 here means "the explosion will face upwards"
	AddEvent( EV_MISSILE_HIT, 5 );

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

	Vector mins = GetOrigin() - Vector( radius, radius, radius );
	Vector maxs = GetOrigin() + Vector( radius, radius, radius );

	IEntity* ent{ nullptr };

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

		// Bang
		ent->TakeDamage( activator, this, 0, damage );
	}
}
