#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Components/IComponent.hpp"
#include "Components/SharedComponent.hpp"

using namespace Entities;
using namespace Components;

DefineEntityClass( "func_nothing", BaseQuakeEntity, IEntity );

KeyValueElement BaseQuakeEntity::keyValues[] =
{
	//KeyValueElement( "origin",	offsetof( BaseQuakeEntity, shared.s.origin ),	KVHandlers::Vector ),
	//KeyValueElement( "angles",	0,												KVHandlers::Vector ),
	KeyValueElement( "spawnflags",	offsetof( BaseQuakeEntity, spawnFlags ),		KVHandlers::Int ),
	KeyValueElement( "spawnflags2", offsetof( BaseQuakeEntity, spawnFlagsExtra ),	KVHandlers::Int ),
	KeyValueElement()
};

void BaseQuakeEntity::Spawn()
{
	shared.s.number = GetEntityIndex();

	const char* model = spawnArgs->GetCString( "model", nullptr );
	Vector keyOrigin = spawnArgs->GetVector( "origin", Vector::Zero );
	Vector keyAngles = spawnArgs->GetVector( "angles", Vector::Zero );

	engine->Print( va( "Spawned a BaseQuakeEntity at %i\n", GetEntityIndex() ) );

	keyOrigin.CopyToArray( shared.s.origin );
	keyAngles.CopyToArray( shared.s.angles );

	shared.s.eType = ET_GENERAL;

	if ( !model )
	{
		engine->Print( "This entity has no model\n" );
	}
	else
	{
		engine->Print( "Setting model " );
		engine->Print( model );
		engine->Print( "\n" );

		if ( model[0] == '*' )
		{
			gameImports->SetBrushModel( this, model );
		}

		//else
		//{
		//	engine->Print( "This is not a brush model, not yet supported\n" );
		//}
	}

	shared.r.svFlags = SVF_USE_CURRENT_ORIGIN;
	gameImports->LinkEntity( this );
	shared.s.pos.trType = TR_STATIONARY;

	engine->Print( va( "Spawned a BaseQuakeEntity at pos %3.2f %3.2f %3.2f\n", keyOrigin.x, keyOrigin.y, keyOrigin.z ) );

	VectorCopy( shared.s.origin, shared.s.pos.trBase );
	VectorCopy( shared.s.origin, shared.r.currentOrigin );
}

void BaseQuakeEntity::PreKeyValue()
{

}

void BaseQuakeEntity::PostKeyValue()
{
	
}

void BaseQuakeEntity::KeyValue()
{
	auto& map = spawnArgs->GetMap();

	PreKeyValue();

	// For every keyvalue string pair
	for ( auto& keyValue : map )
	{
		// Check to see if we can dispatch any of our keyvalues on it
		for ( auto& element : keyValues )
		{
			// Skip this element if it's already handled
			if ( element.IsHandled() )
				continue;

			// Try dispatching the keyvalue
			int result = element.KeyValue( keyValue.first.c_str(), keyValue.second.c_str(), this );
		
			// If it's a success, then stop processing this specific keyvalue
			if ( result == KVHandlers::Success )
				break;
		}
	}

	PostKeyValue();
}

void BaseQuakeEntity::Remove()
{
	flags |= FL_REMOVE_ME;
}

void BaseQuakeEntity::Think()
{
	if ( CheckAndClearEvents() )
		return;

	// Temp ents don't think
	if ( freeAfterEvent )
		return;

	if ( !shared.r.linked && neverFree )
		return;

	if ( nullptr == thinkFunction )
	{
		return;
	}

	if ( nextThink > (level.time * 0.001f) )
	{
		return;
	}
		
	(this->*thinkFunction)();
}

void BaseQuakeEntity::Use( IEntity* activator, IEntity* caller, float value )
{
	if ( useFunction )
		(this->*useFunction)( activator, caller, value );
}

void BaseQuakeEntity::Touch( IEntity* other, trace_t* trace )
{
	if ( touchFunction )
		(this->*touchFunction)( other, trace );
}

const char* BaseQuakeEntity::GetName() const
{
	return targetName.c_str();
}

const char* BaseQuakeEntity::GetClassname() const
{
	return className.c_str();
}

const char* BaseQuakeEntity::GetTarget() const
{
	return target.c_str();
}

Vector BaseQuakeEntity::GetOrigin() const
{
	return Vector( shared.s.origin );
}

void BaseQuakeEntity::SetOrigin( const Vector& newOrigin )
{
	newOrigin.CopyToArray( shared.s.origin );
}

Vector BaseQuakeEntity::GetAngles() const
{
	return Vector( shared.s.angles );
}

void BaseQuakeEntity::SetAngles( const Vector& newAngles )
{
	newAngles.CopyToArray( shared.s.angles );
}

Vector BaseQuakeEntity::GetVelocity() const
{
	return shared.s.pos.trDelta;
}

void BaseQuakeEntity::SetVelocity( const Vector& newVelocity )
{
	newVelocity.CopyToArray( shared.s.pos.trDelta );
}

Vector BaseQuakeEntity::GetMins() const
{
	return shared.r.mins;
}

Vector BaseQuakeEntity::GetMaxs() const
{
	return shared.r.maxs;
}

Vector BaseQuakeEntity::GetAverageOrigin() const
{
	const Vector& mins = GetMins();
	const Vector& maxs = GetMaxs();
	return (mins + maxs) / 2.0f;
}

const int& BaseQuakeEntity::GetSpawnflags() const
{
	return spawnFlags;
}

void BaseQuakeEntity::SetSpawnflags( int newFlags )
{
	spawnFlags = newFlags;
}

const int& BaseQuakeEntity::GetFlags() const
{
	return flags;
}

void BaseQuakeEntity::SetFlags( int newFlags )
{
	flags = newFlags;
}

void BaseQuakeEntity::UseTargets( IEntity* activator )
{
	UseTargets( activator, "target" );
}

void BaseQuakeEntity::UseTargets( IEntity* activator, const char* targetName )
{
	IEntity* ent = gameWorld->FindByName( targetName );

	while ( ent = gameWorld->FindByName( targetName, ent ) )
	{
		ent->Use( activator, this, 0 );
	}
}

void BaseQuakeEntity::KillBox( bool onlyPlayers )
{
	int	i, num;
	std::vector<int> touch;
	IEntity* hit;
	Vector mins, maxs;

	touch.reserve( GameWorld::MaxEntities );
	
	mins = GetOrigin();
	maxs = mins;

	mins += Vector( (float*)shared.r.mins );
	maxs += Vector( (float*)shared.r.maxs );

	num = gameImports->EntitiesInBox( mins, maxs, touch.data(), GameWorld::MaxEntities );

	for ( i = 0; i < num; i++ ) 
	{
		hit = gEntities[touch.data()[i]];
		
		if ( nullptr == hit )
			continue;

		if ( onlyPlayers )
		{
			if ( nullptr == dynamic_cast<BasePlayer*>(hit) )
			{
				continue;
			}
		}

		// nail it
		hit->TakeDamage( this, this, DAMAGE_NO_PROTECTION, 100000 );
	}
}

void BaseQuakeEntity::KillBox( const Vector& size, bool onlyPlayers )
{
	int	i, num;
	std::vector<int> touch;
	IEntity* hit;
	Vector mins, maxs;

	touch.reserve( GameWorld::MaxEntities );

	mins = GetOrigin();
	maxs = mins;

	mins += size / 2.0f;
	maxs += size / 2.0f;

	num = gameImports->EntitiesInBox( mins, maxs, touch.data(), GameWorld::MaxEntities );

	for ( i = 0; i < num; i++ )
	{
		hit = gEntities[touch.data()[i]];

		if ( nullptr == hit )
			continue;

		if ( onlyPlayers )
		{
			if ( nullptr == dynamic_cast<BasePlayer*>(hit) )
			{
				continue;
			}
		}

		// nail it
		hit->TakeDamage( this, this, DAMAGE_NO_PROTECTION, 100000 );
	}
}

bool BaseQuakeEntity::IsClass( const EntityClassInfo& eci )
{
	return GetClassInfo()->IsClass( eci );
}

bool BaseQuakeEntity::IsSubclassOf( const EntityClassInfo& eci )
{
	return GetClassInfo()->IsSubclassOf( eci );
}

bool BaseQuakeEntity::CheckAndClearEvents()
{
	if ( level.time - eventTime > EVENT_VALID_MSEC )
	{
		if ( shared.s.event )
		{
			shared.s.event &= EV_EVENT_BITS;
		}

		if ( freeAfterEvent )
		{
			Remove();
			return true;
		}

		else if ( unlinkAfterEvent )
		{
			unlinkAfterEvent = false;
			gameImports->UnlinkEntity( this );
		}
	}

	return false;
}

void BaseEntity_Test::Spawn()
{
	SetThink( &BaseEntity_Test::MyThink );
}

void BaseEntity_Test::MyThink()
{
	static float cycle = 0.0f;
	static float originalZ = 0;
	static float iterator = 0.008f;

	cycle += iterator;
	if ( fabs( cycle ) > 1.000f )
		iterator *= -1.0f;
}
