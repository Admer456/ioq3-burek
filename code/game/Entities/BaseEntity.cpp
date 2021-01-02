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

	targetName = spawnArgs->GetCString( "targetname", "" );
	target = spawnArgs->GetCString( "target", "" );

	const char* model = spawnArgs->GetCString( "model", nullptr );
	Vector keyOrigin = spawnArgs->GetVector( "origin", Vector::Zero );
	Vector keyAngles = spawnArgs->GetVector( "angles", Vector::Zero );

	//engine->Print( va( "Spawned a BaseQuakeEntity at %i\n", GetEntityIndex() ) );

	GetShared()->ownerNum = ENTITYNUM_NONE;
	GetState()->groundEntityNum = ENTITYNUM_NONE;

	keyOrigin.CopyToArray( shared.s.origin );
	keyOrigin.CopyToArray( shared.r.currentOrigin );
	keyOrigin.CopyToArray( shared.s.pos.trBase );
	keyAngles.CopyToArray( shared.s.angles );

	shared.s.eType = ET_GENERAL;

	if ( !model )
	{
		//engine->Print( "This entity has no model\n" );
	}
	else
	{
		//engine->Print( "Setting model " );
		//engine->Print( model );
		//engine->Print( "\n" );

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

	//engine->Print( va( "Spawned a BaseQuakeEntity at pos %3.2f %3.2f %3.2f\n", keyOrigin.x, keyOrigin.y, keyOrigin.z ) );

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
	newOrigin.CopyToArray( shared.r.currentOrigin );
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

void BaseQuakeEntity::UseTargets( IEntity* activator, const char* targetKey )
{
	const char* name = spawnArgs->GetCString( targetKey, "" );

	IEntity* ent = nullptr;

	while ( ent = gameWorld->FindByName( name, ent ) )
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

BaseQuakeEntity* BaseQuakeEntity::TestEntityPosition()
{
	trace_t	tr;
	int		mask;

	if ( clipMask ) {
		mask = clipMask;
	}
	else {
		mask = MASK_SOLID;
	}

	gameImports->Trace( &tr, shared.s.pos.trBase, shared.r.mins, shared.r.maxs, shared.s.pos.trBase, shared.s.number, mask );
	
	if ( tr.startsolid )
		return static_cast<BaseQuakeEntity*>( gEntities[tr.entityNum] );

	return NULL;
}

struct pushed_t
{
	BaseQuakeEntity* ent;
	Vector	origin;
	Vector	angles;
	float	deltayaw;
};

extern pushed_t pushed[MAX_GENTITIES];
extern pushed_t* pushed_p;

extern void G_CreateRotationMatrix( vec3_t angles, vec3_t matrix[3] );
extern void G_TransposeMatrix( vec3_t matrix[3], vec3_t transpose[3] );
extern void G_RotatePoint( vec3_t point, vec3_t matrix[3] );

bool BaseQuakeEntity::TryPushingEntity( IEntity* check, Vector move, Vector amove )
{
	vec3_t matrix[3], transpose[3];
	vec3_t org, org2, move2;
	BaseQuakeEntity* block;
	BasePlayer* player{ nullptr };

	if ( check->IsClass( BasePlayer::ClassInfo ) )
	{
		player = static_cast<BasePlayer*>(check);
	}

	// EF_MOVER_STOP will just stop when contacting another entity
	// instead of pushing it, but entities can still ride on top of it
	if ( (shared.s.eFlags & EF_MOVER_STOP) && check->GetState()->groundEntityNum != shared.s.number ) 
	{
		return false;
	}

	// save off the old position
	if ( pushed_p > & pushed[MAX_GENTITIES] ) 
	{
		G_Error( "pushed_p > &pushed[MAX_GENTITIES]" );
	}

	pushed_p->ent = static_cast<BaseQuakeEntity*>( check );
	VectorCopy( check->GetState()->pos.trBase, pushed_p->origin );
	VectorCopy( check->GetState()->apos.trBase, pushed_p->angles );
	if ( player ) 
	{
		pushed_p->deltayaw = player->GetClient()->ps.delta_angles[YAW];
		VectorCopy( player->GetClient()->ps.origin, pushed_p->origin );
	}

	pushed_p++;

	// try moving the contacted entity 
	// figure movement due to the pusher's amove
	G_CreateRotationMatrix( amove, transpose );
	G_TransposeMatrix( transpose, matrix );

	if ( player )
	{
		VectorSubtract( player->GetClient()->ps.origin, shared.r.currentOrigin, org );
	}
	
	else 
	{
		VectorSubtract( check->GetState()->pos.trBase, shared.r.currentOrigin, org );
	}
	
	VectorCopy( org, org2 );
	G_RotatePoint( org2, matrix );
	VectorSubtract( org2, org, move2 );
	
	// add movement
	VectorAdd( check->GetState()->pos.trBase, move, check->GetState()->pos.trBase );
	VectorAdd( check->GetState()->pos.trBase, move2, check->GetState()->pos.trBase );
	
	if ( player )
	{
		VectorAdd( player->GetClient()->ps.origin, move, player->GetClient()->ps.origin );
		VectorAdd( player->GetClient()->ps.origin, move2, player->GetClient()->ps.origin );
		// make sure the client's view rotates when on a rotating mover
		player->GetClient()->ps.delta_angles[YAW] += ANGLE2SHORT( amove[YAW] );
	}

	// may have pushed them off an edge
	if ( check->GetState()->groundEntityNum != shared.s.number ) 
	{
		check->GetState()->groundEntityNum = ENTITYNUM_NONE;
	}

	block = static_cast<BaseQuakeEntity*>(check)->TestEntityPosition();
	if ( !block ) 
	{
		// pushed ok
		if ( player ) 
		{
			VectorCopy( player->GetClient()->ps.origin, check->GetShared()->currentOrigin );
		}
		
		else 
		{
			VectorCopy( check->GetState()->pos.trBase, check->GetShared()->currentOrigin );
		}
		
		gameImports->LinkEntity( check );
		
		return true;
	}

	// if it is ok to leave in the old position, do it
	// this is only relevant for riding entities, not pushed
	// Sliding trapdoors can cause this.
	VectorCopy( (pushed_p - 1)->origin, check->GetState()->pos.trBase );

	if ( player ) 
	{
		VectorCopy( (pushed_p - 1)->origin, player->GetClient()->ps.origin );
	}

	VectorCopy( (pushed_p - 1)->angles, check->GetState()->apos.trBase );
	block = static_cast<BaseQuakeEntity*>(check)->TestEntityPosition();

	if ( !block ) 
	{
		check->GetState()->groundEntityNum = ENTITYNUM_NONE;
		pushed_p--;
		return true;
	}

	// blocked
	return false;
}

void BaseQuakeEntity::AddEvent( int event, int eventParameter )
{
	int	bits = 0;

	if ( !event )
	{
		G_Printf( "G_AddEvent: zero event added for entity %i\n", GetEntityIndex() );
		return;
	}

	bits = shared.s.event & EV_EVENT_BITS;
	bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;
	shared.s.event = event | bits;
	shared.s.eventParm = eventParameter;

	eventTime = level.time;
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
