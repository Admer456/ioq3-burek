#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "Entities/Env/EnvSprite.hpp"
#include "Mercenary.hpp"

using namespace Entities;

DefineEntityClass( "char_mercenary", Mercenary, BaseEntity );

const Vector humanMins = Vector( -15, -15, 0 );
const Vector humanMaxs = Vector( 15, 15, 64 );

// Here I've placed two utility functions, temporarily, until we get a GameUtils.hpp thing
namespace Util
{
	void Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
	int PointContents( const vec3_t point, int passEntityNum );
}

void Mercenary::Spawn()
{
	BaseEntity::Spawn();
	
	SetModel( "models/characters/mafia/generic.iqm" );
	health = 100;

	humanMins.CopyToArray( GetShared()->mins );
	humanMaxs.CopyToArray( GetShared()->maxs );

	GetShared()->contents = CONTENTS_BODY;
	clipMask = MASK_PLAYERSOLID;

	GetState()->pos.trType = TR_INTERPOLATE;
	GetState()->apos.trTime = TR_INTERPOLATE;

	gameImports->LinkEntity( this );

	memset( &cmd, 0, sizeof( cmd ) );
	memset( &playerState, 0, sizeof( playerState ) );

	GetCurrentOrigin().CopyToArray( playerState.origin );

	KeyValueLibrary kv;
	kv.AddKeyValue( "radius", "3" );

	debugSprite = gameWorld->CreateEntity<EnvSprite>();
	debugSprite->spawnArgs = &kv;
	debugSprite->Precache();
	debugSprite->Spawn();
}

void Mercenary::Think()
{
	if ( nextThink <= (level.time * 0.001f) && nextThink > 0.0f )
	{
		if ( thinkFunction )
			(this->*thinkFunction)();
	}

	Move();
}

BaseEntity* Mercenary::TestEntityPosition()
{
	trace_t	tr;
	int	mask;

	if ( clipMask )
	{
		mask = clipMask;
	}
	else
	{
		mask = MASK_SOLID;
	}

	gameImports->Trace( &tr, playerState.origin, shared.r.mins, shared.r.maxs, playerState.origin, GetEntityIndex(), mask );

	if ( tr.startsolid )
		return static_cast<BaseEntity*>(gEntities[tr.entityNum]);

	return nullptr;
}

float Mercenary::SnapMovementSpeed( float speed, bool smooth )
{
	speed *= 127.0f;

	if ( speed > MovementSpeeds::Running )
		return 1.0f;

	else if ( speed > MovementSpeeds::Walking )
		return MovementSpeeds::Running / 127.0f;

	else if ( speed > MovementSpeeds::Sneaking )
		return MovementSpeeds::Walking / 127.0f;

	else if ( speed > MovementSpeeds::Idle )
		return MovementSpeeds::Sneaking / 127.0f;

	return MovementSpeeds::Idle;
}

usercmd_t Mercenary::CalculateCmdForVelocity( Vector wishVelocity, bool canJump )
{
	usercmd_t rcmd;
	memset( &rcmd, 0, sizeof( rcmd ) );

	Vector currentVelocity = playerState.velocity;
	Vector forward, right, up;
	float dotF, dotR, dotU;
	Vector::AngleVectors( viewAngles, &forward, &right, &up );

	float approach = Q_fabs( MIN( wishVelocity.Length() / 320.0f, 1.0f ) );
	approach = sqrt( approach );

	dotF = wishVelocity.Normalized() * approach * forward;
	dotR = wishVelocity.Normalized() * approach * right;
	dotU = wishVelocity.Normalized() * up;

	if ( canJump )
	{
		if ( dotU > 0.1f )
		{
			rcmd.upmove = 127;
		}
	}

	rcmd.forwardmove = dotF * 127;
	rcmd.rightmove = dotR * 127;

	return rcmd;
}

void Mercenary::MovementLogic()
{
	if ( gEntities[0] )
		moveTarget = gEntities[0]->GetCurrentOrigin() - Vector( 0, 0, 24 );

	if ( playerState.groundEntityNum == ENTITYNUM_NONE )
		return;

	trace_t trForward, trBackward, trRight, trLeft, trUp;
	Vector forward, right, up;
	Vector start = GetCurrentOrigin() + Vector( 0, 0, 12 );
	Vector wishVelocity = moveTarget - GetCurrentOrigin();

	viewAngles = wishVelocity.ToAngles();
	viewAngles.x = 0;

	// Shoot 4 rays to determine how close we are to a wall
	Vector::AngleVectors( GetAngles(), &forward, &right, &up );
	Util::Trace( &trForward,	start, nullptr, nullptr, start + forward * 96.0f,	GetEntityIndex(), MASK_PLAYERSOLID );
	Util::Trace( &trBackward,	start, nullptr, nullptr, start - forward * 96.0f,	GetEntityIndex(), MASK_PLAYERSOLID );
	Util::Trace( &trRight,		start, nullptr, nullptr, start + right * 96.0f,		GetEntityIndex(), MASK_PLAYERSOLID );
	Util::Trace( &trLeft,		start, nullptr, nullptr, start - right * 96.0f,		GetEntityIndex(), MASK_PLAYERSOLID );

	// Calculate base movement
	usercmd_t ccmd = CalculateCmdForVelocity( wishVelocity, false );
	int forwardMove = ccmd.forwardmove - (80 * (1.0 - trForward.fraction)) + (80 * (1.0 - trBackward.fraction));
	int rightMove = ccmd.rightmove - (80 * (1.0 - trRight.fraction)) + (80 * (1.0 - trLeft.fraction));

	{
		// Check if there's a pit
		trace_t trPit;
		trace_t trDepth;
		float heightTolerance = 40.0f;
		float targetHeight = wishVelocity.z;
		float dropHeight = targetHeight - heightTolerance;
		float pitDepth;

		Util::Print( va( "AI: target height: %3.2f   drop height: %3.2f\n", targetHeight, dropHeight ) );

		// 12u are typical stair steps, those are easy to climb
		if ( dropHeight < -12.0f )
		{
			Vector velocityDir = wishVelocity.Normalized();
			velocityDir.z = 0;
			Util::Trace( &trPit, start + velocityDir * 64.0f, nullptr, nullptr, start + velocityDir * 64.0f + Vector( 0, 0, dropHeight ), GetEntityIndex(), MASK_PLAYERSOLID );
			Util::Trace( &trDepth, start + velocityDir * 64.0f, nullptr, nullptr, start + velocity * 64.0f + Vector( 0, 0, -512.0f ), GetEntityIndex(), MASK_PLAYERSOLID );

			pitDepth = trDepth.fraction * -512.0f + 12.0f;

			Util::Print( va( "AI: the pit is %3.2f u deep\n", pitDepth ) );

			// There's a pit, calculate how to go away from it
			if ( trPit.fraction == 1.0f || dropHeight <= -150.0f )
			{
				trace_t trPitForward;
				trace_t trPitRight;
				trace_t trPitLeft;

				Util::Print( "AI: There's a pit, I cannot go...\n" );

				Util::Trace( &trPitForward, start + forward * 40.0f, nullptr, nullptr, start + forward * 40.0f + Vector( 0, 0, dropHeight ), GetEntityIndex(), MASK_PLAYERSOLID );
				Util::Trace( &trPitRight, start + right * 40.0f, nullptr, nullptr, start + right * 40.0f + Vector( 0, 0, dropHeight ), GetEntityIndex(), MASK_PLAYERSOLID );
				Util::Trace( &trPitLeft, start + right * -40.0f, nullptr, nullptr, start + right * -40.0f + Vector( 0, 0, dropHeight ), GetEntityIndex(), MASK_PLAYERSOLID );
						
				// if the pit is deeper than 40u (the maximum height for obstacle jumping), then jump over it
				if ( pitDepth < (-heightTolerance - 5.0f) )
				{
					// If fraction is 1, that means there's a pit
					// So -225 is gonna be enough to go back
					// Backwards isn't accounted for, cuz' you don't have eyes on your back
					forwardMove -= 225 * trPitForward.fraction;
					rightMove -= 255 * trPitRight.fraction;
					rightMove += 255 * trPitLeft.fraction;
				}

				// Check if we can jump over a pit
				{
					trace_t tr;
					Vector end = start + (forward * 192.0f) + Vector( 0, 0, -16 );

					Util::Trace( &tr, start + Vector( 0, 0, 32 ), nullptr, nullptr, end, GetEntityIndex(), MASK_PLAYERSOLID );
					float dotPlane = Vector( 0, 0, 1 ) * Vector( tr.plane.normal );

					debugSprite->SetOrigin( end );
					debugSprite->SetCurrentOrigin( debugSprite->GetOrigin() );
					debugSprite->GetOrigin().CopyToArray( debugSprite->GetState()->pos.trBase );

					if ( tr.fraction < 1.0f )
					{
						debugSprite->SetOrigin( tr.endpos );
						debugSprite->SetCurrentOrigin( debugSprite->GetOrigin() );
						debugSprite->GetOrigin().CopyToArray( debugSprite->GetState()->pos.trBase );

						// check if the landing surface isn't too steep; a dot of 0.79 is basically about 40-ish degrees
						// if the dot was 0, then that means it's a wall...
						if ( dotPlane >= 0.79f )
						{
							// JUMP!!!
							forwardMove = 127;
							rightMove /= 4;
							ccmd.upmove = 127;
							wishVelocity *= 4.0f;

							Vector jump = wishVelocity.Normalized();
							jump *= 300;
							jump.z = 350;

							jump.CopyToArray( playerState.velocity );
						}
					}
				}
			}
		}
	}

	// Check if we can jump over an obstacle
	if ( ccmd.forwardmove && trForward.fraction < 0.35f )
	{
		trace_t trJumpCheck;
		trace_t trObstacle;
		Util::Trace( &trJumpCheck, start + Vector( 0, 0, 29 ), nullptr, nullptr, start + forward * 25.0f + Vector( 0, 0, 29 ), GetEntityIndex(), MASK_PLAYERSOLID );
		Util::Trace( &trObstacle, start + Vector( 0, 0, 29 ), nullptr, nullptr, start + forward * 64.0f + Vector( 0, 0, 29 ), GetEntityIndex(), MASK_PLAYERSOLID );

		debugSprite->SetOrigin( trJumpCheck.endpos );
		debugSprite->SetCurrentOrigin( debugSprite->GetOrigin() );
		debugSprite->GetOrigin().CopyToArray( debugSprite->GetState()->pos.trBase );

		bool canJump = (trJumpCheck.fraction == 1.0f) && (trJumpCheck.entityNum == ENTITYNUM_NONE) && (trObstacle.entityNum >= ENTITYNUM_WORLD);

		if ( canJump )
		{
			forwardMove = 127;
			ccmd.upmove = 127;
		}
	}
	
	cmd.forwardmove = MAX( MIN( forwardMove, 127 ), -128 );
	cmd.rightmove = MAX( MIN( rightMove, 127 ), -128 );
	cmd.upmove = ccmd.upmove;

	playerState.speed = 600 * SnapMovementSpeed( wishVelocity.Length2D() / 2.0f );
}

void Mercenary::CalculateMoveParameters()
{
	cmd.serverTime = level.time;

	playerState.pm_flags |= PMF_NPC;
	playerState.stats[STAT_HEALTH] = health;
	playerState.clientNum = GetEntityIndex();
	playerState.commandTime = cmd.serverTime - 17;
	playerState.gravity = g_gravity.value * 2.0f;

	SetAngles( viewAngles );
	SetCurrentAngles( viewAngles );
	viewAngles.CopyToArray( GetState()->apos.trBase );
	viewAngles.CopyToArray( playerState.viewangles );
	
	cmd.angles[0] = ANGLE2SHORT( viewAngles.x );
	cmd.angles[1] = ANGLE2SHORT( viewAngles.y );
	cmd.angles[2] = ANGLE2SHORT( viewAngles.z );

	MovementLogic();
}

void Mercenary::Move()
{
	pmove_t pm;
	memset( &pm, 0, sizeof( pm ) );

	CalculateMoveParameters();

	pm.ps = &playerState;
	pm.cmd = cmd;

	pm.trace = Util::Trace;
	pm.pointcontents = Util::PointContents;
	(humanMins + Vector(0,0,24)).CopyToArray( pm.mins );
	(humanMaxs + Vector(0,0,24)).CopyToArray( pm.maxs );

	pm.debugLevel = 0;
	pm.noFootsteps = 0;

	pm.pmove_fixed = pmove_fixed.integer;
	pm.pmove_msec = pmove_msec.integer;

	pm.tracemask = MASK_PLAYERSOLID;

	// Player movement code works surprisingly well ;)
	Pmove( &pm );

	BG_PlayerStateToEntityState( &playerState, GetState(), false );
	GetState()->eType = ET_GENERAL;

	VectorCopy( pm.mins, GetShared()->mins );
	VectorCopy( pm.maxs, GetShared()->maxs );

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( GetState()->pos.trBase, GetShared()->currentOrigin );

	Vector playerOrigin = playerState.origin;
	Vector amax = playerOrigin + humanMaxs;
	Vector amin = playerOrigin + humanMins;

	// Set state origin for linking
	// https://github.com/Admer456/ioq3-burek/issues/34
	SetOrigin( playerOrigin );

	// link entity now, after any personal teleporters have been used
	gameImports->LinkEntity( this );

	// Unset it
	SetOrigin( Vector::Zero );

	amax.CopyToArray( GetShared()->absmax );
	amin.CopyToArray( GetShared()->absmin );

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	SetCurrentOrigin( playerState.origin );
}
