#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Entities/BasePlayer.hpp"

#include "BaseMover.hpp"

using namespace Entities;

DefineAbstractEntityClass( BaseMover, BaseEntity );

pushed_t pushed[MAX_GENTITIES];
pushed_t* pushed_p;

void BaseMover::Spawn()
{
	BaseEntity::Spawn();

	GetState()->eType = ET_MOVER;

	trajectory_t* tr = &GetState()->pos;

	GetShared()->svFlags |= SVF_USE_CURRENT_ORIGIN;

	GetOrigin().CopyToArray( tr->trBase );
	SetCurrentOrigin( GetOrigin() );
	gameImports->LinkEntity( this );
	SetOrigin( Vector::Zero );
	SetCurrentAngles( GetState()->apos.trBase );
	//GetState()->clipFlags |= ClipFlag_ManualAbsoluteBox;
	//GetState()->clipFlags |= ClipFlag_HasOriginBrush;
}

void BaseMover::Think()
{
	if ( CheckAndClearEvents() )
		return;

	// Temp ents don't think
	if ( freeAfterEvent )
		return;

	if ( GetState()->pos.trType == TR_INTERPOLATE && GetState()->apos.trType == TR_INTERPOLATE )
	{
		CustomMoverThink();
	}
	else
	{
		MoverThink();
	}

	SetOrigin( GetCurrentOrigin() );

	//Vector angles;
	//BG_EvaluateTrajectory( &GetState()->apos, level.time, angles );
	//SetAngles( angles );
	//SetCurrentAngles( angles );

	gameImports->LinkEntity( this );

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

void BaseMover::MoverThink()
{
	// If not stationary at one of the positions, MOVE
	if ( GetState()->pos.trType == TR_STATIONARY && GetState()->apos.trType == TR_STATIONARY )
		return;

	Vector		move, amove;
	BaseMover* part;
	BaseEntity* obstacle;
	Vector		origin, angles;

	obstacle = nullptr;

	// make sure all team slaves can move before committing
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	for ( part = this; part; part = static_cast<BaseMover*>(part->chain) )
	{
		// get current position
		BG_EvaluateTrajectory( &part->GetState()->pos, level.time, origin );
		BG_EvaluateTrajectory( &part->GetState()->apos, level.time, angles );
		VectorSubtract( origin, part->GetShared()->currentOrigin, move );
		VectorSubtract( angles, part->GetShared()->currentAngles, amove );

		if ( !part->MoverPush( move, amove, &obstacle ) )
		{
			break;	// move was blocked
		}
	}

	if ( part )
	{
		// go back to the previous position
		for ( part = this; part; part = static_cast<BaseMover*>(part->chain) )
		{
			part->GetState()->pos.trTime += level.time - level.previousTime;
			part->GetState()->apos.trTime += level.time - level.previousTime;
			BG_EvaluateTrajectory( &part->GetState()->pos, level.time, part->GetShared()->currentOrigin );
			BG_EvaluateTrajectory( &part->GetState()->apos, level.time, part->GetShared()->currentAngles );
			gameImports->LinkEntity( part );
		}

		// if the pusher has a "blocked" function, call it
		Blocked( obstacle );
		return;
	}

	// the move succeeded
	for ( part = this; part; part = static_cast<BaseMover*>(part->chain) )
	{
		// call the reached function if time is at or past end point
		if ( part->GetState()->pos.trType == TR_LINEAR_STOP )
		{
			if ( level.time >= part->GetState()->pos.trTime + part->GetState()->pos.trDuration )
			{
				//if ( part->reached ) 
				//{
				//	part->reached( part );
				//}
			}
		}
	}
}

void BaseMover::CustomMoverThink()
{
	BaseEntity* obstacle{ nullptr };

	trajectory_t* pos = &GetState()->pos;
	trajectory_t* apos = &GetState()->apos;

	float delta = (level.time - level.previousTime) * 0.001f;

	if ( pos->trType == TR_INTERPOLATE && velocity != Vector::Zero )
	{
		Vector currentPos = pos->trBase;
		currentPos += velocity * delta;

		currentPos.CopyToArray( pos->trBase );
		(velocity * delta).CopyToArray( pos->trDelta );
		pos->trDuration = delta * 1000;

		SetOrigin( currentPos );
		SetCurrentOrigin( currentPos );
	}

	if ( apos->trType == TR_INTERPOLATE && angularVelocity != Vector::Zero )
	{
		Vector currentPos = apos->trBase;
		currentPos += angularVelocity * delta;

		currentPos.CopyToArray( apos->trBase );
		(angularVelocity * delta).CopyToArray( apos->trDelta );
		pos->trDuration = delta * 1000;

		SetAngles( currentPos );
		SetCurrentAngles( currentPos );
	}

	if ( velocity != Vector::Zero || angularVelocity != Vector::Zero )
		if ( !MoverPush( velocity * delta, angularVelocity * delta, &obstacle ) )
			Blocked( obstacle );
}

/*
============
MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If false is returned, *obstacle will be the blocking entity
============
*/
bool BaseMover::MoverPush( Vector move, Vector amove, BaseEntity** obstacle )
{
	int i, e;
	BaseEntity* check;
	vec3_t mins, maxs;
	pushed_t* p;
	int entityList[MAX_GENTITIES];
	int listedEntities;
	vec3_t totalMins, totalMaxs;

	*obstacle = nullptr;

	// mins/maxs are the bounds at the destination
	// totalMins / totalMaxs are the bounds for the entire move
	if ( GetShared()->currentAngles[0] || GetShared()->currentAngles[1] || GetShared()->currentAngles[2]
		 || amove[0] || amove[1] || amove[2] )
	{
		float radius;

		radius = RadiusFromBounds( GetShared()->mins, GetShared()->maxs );
		for ( i = 0; i < 3; i++ )
		{
			mins[i] = GetShared()->currentOrigin[i] + move[i] - radius;
			maxs[i] = GetShared()->currentOrigin[i] + move[i] + radius;
			totalMins[i] = mins[i] - move[i];
			totalMaxs[i] = maxs[i] - move[i];
		}
	}
	else
	{
		for ( i = 0; i < 3; i++ )
		{
			mins[i] = GetShared()->absmin[i] + move[i];
			maxs[i] = GetShared()->absmax[i] + move[i];
		}

		VectorCopy( GetShared()->absmin, totalMins );
		VectorCopy( GetShared()->absmax, totalMaxs );

		for ( i = 0; i < 3; i++ )
		{
			if ( move[i] > 0 )
			{
				totalMaxs[i] += move[i];
			}
			else
			{
				totalMins[i] += move[i];
			}
		}
	}

	// unlink the pusher so we don't get it in the entityList
	gameImports->UnlinkEntity( this );

	listedEntities = gameImports->EntitiesInBox( totalMins, totalMaxs, entityList, MAX_GENTITIES );

	// move the pusher to its final position
	VectorAdd( GetShared()->currentOrigin, move, GetShared()->currentOrigin );
	VectorAdd( GetShared()->currentAngles, amove, GetShared()->currentAngles );
	
	gameImports->LinkEntity( this );

	// see if any solid entities are inside the final position
	for ( e = 0; e < listedEntities; e++ )
	{
		check = static_cast<BaseEntity*>(gEntities[entityList[e]]);
		
		if ( nullptr == check )
			continue;

		// only push items and players
		if ( check->GetState()->eType != ET_ITEM && check->GetState()->eType != ET_PLAYER && !check->isPhysicsObject )
		{
			continue;
		}

		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->GetState()->groundEntityNum != GetState()->number ) 
		{
			// see if the ent needs to be tested
			if ( check->GetShared()->absmin[0] >= maxs[0]
				 || check->GetShared()->absmin[1] >= maxs[1]
				 || check->GetShared()->absmin[2] >= maxs[2]
				 || check->GetShared()->absmax[0] <= mins[0]
				 || check->GetShared()->absmax[1] <= mins[1]
				 || check->GetShared()->absmax[2] <= mins[2] ) 
			{
				continue;
			}
			// see if the ent's bbox is inside the pusher's final position
			// this does allow a fast moving object to pass through a thin entity...
			if ( !check->TestEntityPosition() )
			{
				continue;
			}
		}

		// the entity needs to be pushed
		if ( TryPushingEntity( check, move, amove ) )
		{
			continue;
		}

		// the move was blocked an entity

		// bobbing entities are instant-kill and never get blocked
		if ( GetState()->pos.trType == TR_SINE || GetState()->apos.trType == TR_SINE )
		{
			check->TakeDamage( this, this, DAMAGE_NO_PROTECTION, 99999 );
			continue;
		}

		// save off the obstacle so we can call the block function (crush, etc)
		*obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for ( p = pushed_p - 1; p >= pushed; p-- )
		{
			VectorCopy( p->origin, p->ent->GetState()->pos.trBase );
			VectorCopy( p->angles, p->ent->GetState()->apos.trBase );

			if ( p->ent->IsClass( BasePlayer::ClassInfo ) )
			{
				static_cast<BasePlayer*>(p->ent)->GetClient()->ps.delta_angles[YAW] = p->deltayaw;
				VectorCopy( p->origin, static_cast<BasePlayer*>(p->ent)->GetClient()->ps.origin );
			}
			gameImports->LinkEntity( p->ent );
		}
		
		return false;
	}

	return true;
}
