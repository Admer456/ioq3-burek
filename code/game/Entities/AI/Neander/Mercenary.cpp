#include "Maths/Vector.hpp"

#include "Game/g_local.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"

#include "Entities/AI/AI_Common.hpp"
#include "Node.hpp"
#include "Mercenary.hpp"

using namespace Entities;
using namespace AI;

// MemoryFrame

void MemoryFrame::Reset()
{
	lastSeen = Vector::Zero;
	lastHeard = Vector::Zero;
	flags = 0;
	time = -1.0f;
}

// EntityMemory

const EntityMemory EntityMemory::NoMemory = EntityMemory();

EntityMemory::EntityMemory()
{
	entityIndex = ENTITYNUM_NONE;
	relationship = Relationship::None;
}

EntityMemory::EntityMemory( BaseEntity* ent, AI::Relationship relationship )
{
	if ( nullptr == ent )
		return;

	entityIndex = ent->GetEntityIndex();
	this->relationship = relationship;
}

void EntityMemory::AddFrame( const MemoryFrame& frame )
{
	for ( MemoryFrame& f : frames )
	{
		if ( f.time < 0.0f )
		{
			f = frame;
			return;
		}
	}

	// If that fails, then look for the oldest memory to wipe it out
	float oldestTime = 99999999.0f;
	MemoryFrame* oldest = nullptr;
	for ( MemoryFrame& f : frames )
	{
		if ( f.time < oldestTime )
		{
			oldestTime = f.time;
			oldest = &f;
		}
	}

	*oldest = frame;
}

void EntityMemory::Update( const float& time )
{
	for ( MemoryFrame& frame : frames )
	{
		if ( frame.time < 0.0f )
		{
			continue;
		}

		// Forget after 40-ish seconds
		if ( (time - frame.time) > 40.0f )
		{
			frame.Reset();
			continue;
		}
	}

	// Drop the awareness level if enough time has passed
	if ( (time - lastAware) > AwarenessExpiration[awareness] )
	{
		lastAware = time;
		DecreaseAwareness();
	}
}

Vector EntityMemory::LastSeen() const
{
	float latestTime = 0.0f;
	const MemoryFrame* latest = nullptr;

	for ( const MemoryFrame& frame : frames )
	{
		if ( frame.flags & MFF_Seen && frame.time > latestTime )
		{
			latestTime = frame.time;
			latest = &frame;
		}
	}

	if ( nullptr == latest )
		return Vector::Zero;

	return latest->lastSeen;
}

Vector EntityMemory::LastHeard() const
{
	float latestTime = 0.0f;
	const MemoryFrame* latest = nullptr;

	for ( const MemoryFrame& frame : frames )
	{
		if ( frame.flags & MFF_Heard && frame.time > latestTime )
		{
			latestTime = frame.time;
			latest = &frame;
		}
	}

	if ( nullptr == latest )
		return Vector::Zero;

	return latest->lastHeard;
}

float EntityMemory::LastMemoryTime() const
{
	float latestTime = 0.0f;

	for ( const MemoryFrame& frame : frames )
	{
		if ( frame.time > latestTime )
		{
			latestTime = frame.time;
		}
	}

	return latestTime;
}

void EntityMemory::IncreaseAwareness( const float& time )
{
	if ( awareness < AI::Awareness::MaximumCertainty )
		awareness++;

	lastAware = time;
}

void EntityMemory::DecreaseAwareness()
{
	if ( awareness > AI::Awareness::None )
		awareness--;
}

// Mercenary

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
	moveIdeal = GetCurrentOrigin();
	moveTarget = moveIdeal;

	species = spawnArgs->GetInt( "species", AI::Species_Human );
	faction = spawnArgs->GetInt( "species", AI::Faction_Mafia );
}

void Mercenary::Think()
{
	const float time = level.time * 0.001f;

	if ( nextThink <= time && nextThink > 0.0f )
	{
		if ( thinkFunction )
			(this->*thinkFunction)();
	}

	// Scan for visible entities
	if ( nextSightQuery <= time )
	{
		SightQuery();
		nextSightQuery = time + 0.5f + crandom() * 0.1f;
	}

	if ( nextDecision <= time )
	{
		EvaluateSituation();
		nextDecision = time + 0.2f + crandom() * 0.1f;
	}

	AimAtTarget();

	// Lastly, move
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

Vector Mercenary::GetHeadOffset() const
{
	return Vector( 0, 0, 56 );
}

AI::Relationship Mercenary::Relationship( BaseEntity* entity ) const
{
	if ( entity->IsSubclassOf( Mercenary::ClassInfo ) )
	{
		Mercenary* merc = static_cast<Mercenary*>(entity);
		return Relationship( merc->GetSpecies(), merc->GetFaction() );
	}
	else if ( entity->IsClass( BasePlayer::ClassInfo ) )
	{
		return Relationship( Species_Human, Faction_Agency );
	}
	else
	{
		return AI::Relationship::None;
	}
}

AI::Relationship Mercenary::Relationship( uint8_t spec, uint8_t fac ) const
{
	// to save some typing
	using R = AI::Relationship;

	constexpr static AI::Relationship table[Faction_MAX][Faction_MAX] =
	{//	Faction:       None        Mafia          Criminals   Police      Agency         Aliens    Rogue
		/*None*/     { R::Neutral, R::Neutral,    R::Neutral, R::Neutral, R::Neutral,    R::Enemy, R::Enemy },
		/*Mafia*/    { R::Neutral, R::BestAlly,   R::Enemy,   R::Enemy,   R::WorstEnemy, R::Enemy, R::Enemy },
		/*Criminals*/{ R::Neutral, R::Enemy,      R::Ally,    R::Enemy,   R::WorstEnemy, R::Enemy, R::Enemy },
		/*Police*/   { R::Neutral, R::Enemy,      R::Enemy,   R::Ally,    R::Enemy,      R::Enemy, R::Enemy },
		/*Agency*/   { R::Neutral, R::WorstEnemy, R::Enemy,   R::Enemy,   R::Ally,       R::Enemy, R::Enemy },
		/*Aliens*/   { R::Enemy,   R::Enemy,      R::Enemy,   R::Enemy,   R::Enemy,      R::Ally,  R::Enemy },
		/*Rogue*/    { R::Enemy,   R::Enemy,      R::Enemy,   R::Enemy,   R::Enemy,      R::Enemy, R::Enemy },
	};//Species^

	switch ( spec )
	{
	case Species_Object:
	case Species_Plant:
		return R::None;
	}

	return table[faction][fac];
}

float Mercenary::SnapMovementSpeed( float speed, bool smooth )
{
	speed *= 127.0f;

	if ( speed > MovementSpeeds::Running )
		return 1.0f;

	else if ( speed > MovementSpeeds::Walking )
		return MovementSpeeds::Running / 127.0f;

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
	if ( playerState.groundEntityNum == ENTITYNUM_NONE )
		return;

	Vector start = GetCurrentOrigin() + Vector( 0, 0, 12 );
	Vector wishVelocity = moveTarget - GetCurrentOrigin();

	// Calculate base movement
	usercmd_t ccmd = CalculateCmdForVelocity( wishVelocity, false );
	int forwardMove = ccmd.forwardmove;
	int rightMove = ccmd.rightmove;

#if 0
	trace_t trForward, trBackward, trRight, trLeft, trUp;
	Vector forward, right, up;
	viewAngles = wishVelocity.ToAngles();
	viewAngles.x = 0;

	// Shoot 4 rays to determine how close we are to a wall
	Vector::AngleVectors( GetAngles(), &forward, &right, &up );
	Util::Trace( &trForward,	start, nullptr, nullptr, start + forward * 96.0f,	GetEntityIndex(), MASK_PLAYERSOLID );
	Util::Trace( &trBackward,	start, nullptr, nullptr, start - forward * 96.0f,	GetEntityIndex(), MASK_PLAYERSOLID );
	Util::Trace( &trRight,		start, nullptr, nullptr, start + right * 96.0f,		GetEntityIndex(), MASK_PLAYERSOLID );
	Util::Trace( &trLeft,		start, nullptr, nullptr, start - right * 96.0f,		GetEntityIndex(), MASK_PLAYERSOLID );

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

		// 12u are typical stair steps, those are easy to climb
		if ( dropHeight < -12.0f )
		{
			Vector velocityDir = wishVelocity.Normalized();
			velocityDir.z = 0;
			Util::Trace( &trPit, start + velocityDir * 64.0f, nullptr, nullptr, start + velocityDir * 64.0f + Vector( 0, 0, dropHeight ), GetEntityIndex(), MASK_PLAYERSOLID );
			Util::Trace( &trDepth, start + velocityDir * 64.0f, nullptr, nullptr, start + velocity * 64.0f + Vector( 0, 0, -512.0f ), GetEntityIndex(), MASK_PLAYERSOLID );

			pitDepth = trDepth.fraction * -512.0f + 12.0f;

			// There's a pit, calculate how to go away from it
			if ( trPit.fraction == 1.0f || dropHeight <= -150.0f )
			{
				trace_t trPitForward;
				trace_t trPitRight;
				trace_t trPitLeft;

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

					if ( tr.fraction < 1.0f )
					{
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

		bool canJump = (trJumpCheck.fraction == 1.0f) && (trJumpCheck.entityNum == ENTITYNUM_NONE) && (trObstacle.entityNum >= ENTITYNUM_WORLD);

		if ( canJump )
		{
			forwardMove = 127;
			ccmd.upmove = 127;
		}
	}
#endif

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

	Vector bodyAngles = Vector( 0, viewAngles.y, 0 );
	SetAngles( bodyAngles );
	SetCurrentAngles( bodyAngles );
	bodyAngles.CopyToArray( GetState()->apos.trBase );
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

	// Correct the body angles
	Vector bodyAngles = Vector( 0, viewAngles.y, 0 );
	SetAngles( bodyAngles );
	SetCurrentAngles( bodyAngles );
	bodyAngles.CopyToArray( GetState()->apos.trBase );

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

void Mercenary::UpdatePath()
{
	// Step 1: get the destination and current nearest nodes
	Node* node = Node::GetNearest( moveIdeal );
	Node* nearest = Node::GetNearest( GetCurrentOrigin() );
	
	if ( nullptr == currentNode )
		currentNode = nearest;

	// Step 2: snap to the current nearest node
	float distance = (nearest->GetCurrentOrigin() - GetCurrentOrigin()).Length();
	if ( distance < 64.0f )
	{
		currentNode = nearest;
	}

	if ( nullptr == node || nullptr == nearest )
	{	// "Automatic" pathfinding
		moveTarget = moveIdeal;
		Util::Print( "node and/or nearest are nullptr\n" );
		return;
	}
	
	Node* nextNode = currentNode->Next( currentNode, node );
	if ( nullptr == nextNode )
	{
		Util::Print( "nextNode is nullptr\n" );
		moveTarget = moveIdeal;
		return;
	}

	moveTarget = nextNode->GetCurrentOrigin();
}

void Mercenary::SightQuery()
{
	auto entities = gameWorld->EntitiesInRadius( GetCurrentOrigin(), 1024 );

	Vector forward;
	Vector::AngleVectors( viewAngles, &forward, nullptr, nullptr );

	for ( IEntity*& ent : entities )
	{
		// Step 0: see if it's a character of any kind
		if ( !ent->IsSubclassOf( Mercenary::ClassInfo ) && !ent->IsClass( BasePlayer::ClassInfo ) )
		{
			continue;
		}

		// Step 1: determine if in FOV
		Vector dir = (ent->GetCurrentOrigin() - GetCurrentOrigin()).Normalized();
		float dot = forward * dir;

		// Hardcoded 120-degree FOV, todo: make more customisable
		if ( dot < 0.5f )
		{
			// Not visible
			continue;
		}

		// Step 2: determine if visible, trace from head to head, todo: trace several other bodyparts too
		Vector start = GetCurrentOrigin() + GetHeadOffset();
		Vector end = ent->GetAverageOrigin() + ent->GetCurrentOrigin();
		trace_t tr;

		Util::Trace( &tr, start, nullptr, nullptr, end, GetEntityIndex(), MASK_SHOT );
		if ( tr.entityNum != ent->GetEntityIndex() )
		{
			// Not visible
			if ( ent->IsClass( BasePlayer::ClassInfo ) )
			{
				Util::PrintDev( va( "Can't see the player, blocked by some object (id %i)\n", tr.entityNum ), 2 );
			}
			continue;
		}

		// Step 3: determine if this is a friend or foe, "remember" them if possible
		RegisterVisibleEntity( static_cast<BaseEntity*>( ent ) );
	}
}

void Mercenary::RegisterVisibleEntity( BaseEntity* ent )
{
	AI::Relationship rel = Relationship( ent );

	// If this entity is not in memory, remember it
	if ( !IsInMemory( ent ) )
	{
		memories.push_back( EntityMemory( ent, rel ) );
	}

	// Get some data about the entity to remember for a while
	MemoryFrame mf;
	EntityMemory* em = GetMemory( ent );

	// Since RegisterVisibleEntity gets called from SightQuery, 
	// it only makes sense to set lastSeen
	mf.lastSeen = ent->GetCurrentOrigin();
	mf.time = level.time * 0.001f;
	
	em->AddFrame( mf );
	em->Update( level.time * 0.001f );
	em->IncreaseAwareness( level.time * 0.001f );
}

bool Mercenary::IsInMemory( BaseEntity* ent )
{
	for ( EntityMemory& em : memories )
	{
		if ( em.entityIndex == ent->GetEntityIndex() )
			return true;
	}

	return false;
}

EntityMemory* Mercenary::GetMemory( BaseEntity* ent )
{
	for ( EntityMemory& em : memories )
	{
		if ( em.entityIndex == ent->GetEntityIndex() )
		{
			return &em;
		}
	}

	return nullptr;
}

void Mercenary::EvaluateSituation()
{
	switch ( situation )
	{
	case AI::ST_Casual: Situation_Casual(); break;
	case AI::ST_Combat: Situation_Combat(); break;
	}

	UpdatePath();
}

void Mercenary::Situation_Casual()
{
	// Check if there are any enemies so we can transition to combat mode
	BaseEntity* enemy = GetEnemy();

	if ( enemy )
	{
		situation = AI::ST_Combat;
		targetEntity = enemy;
		return;
	}
}

void Mercenary::Situation_Combat()
{
	moveIdeal = targetEntity->GetCurrentOrigin();
	lookTarget = targetEntity->GetCurrentOrigin();
}

BaseEntity* Mercenary::GetEnemy()
{
	// The most important enemy is retrieved by the following criteria:
	// - distance (the closer the more important)
	// - relationship (WorstEnemy > Enemy)
	// - awareness (MaximumCertainty)
	// - life (must be alive)
	// - time (must be a recent enough memory)
	
	const float time = level.time * 0.001f;

	// First, make a list of entities that match the criteria
	std::vector<EntityMemory*> ems;
	ems.reserve( 16U );

	for ( EntityMemory& em : memories )
	{
		if ( !em.alive )
			continue;

		if ( (time - em.LastMemoryTime()) > 100.0f )
			continue;

		if ( em.awareness != Awareness::MaximumCertainty )
			continue;

		if ( em.relationship != Relationship::Enemy && em.relationship != Relationship::WorstEnemy )
			continue;
	
		ems.push_back( &em );
	}

	bool worstEnemy = false;
	float closest = 5000.0f;
	EntityMemory* closestEnemy = nullptr;

	for ( EntityMemory* em : ems )
	{
		if ( em->relationship == Relationship::WorstEnemy )
		{
			worstEnemy = true;
		}
		
		// Skip regular enemies if we have worst enemies around
		if ( worstEnemy && em->relationship != Relationship::WorstEnemy )
		{
			continue;
		}

		float distance = (em->LastSeen() - GetCurrentOrigin()).Length();
		if ( distance < closest )
		{
			closest = distance;
			closestEnemy = em;
		}
	}

	if ( nullptr == closestEnemy )
	{
		return nullptr;
	}

	IEntity* ent = gEntities[closestEnemy->entityIndex];
	if ( nullptr == ent )
	{
		return nullptr;
	}

	return static_cast<BaseEntity*>(ent);
}

void Mercenary::AimAtTarget()
{
	if ( nullptr != targetEntity )
	{
		lookTarget = targetEntity->GetCurrentOrigin();
	}

	Vector dir = (lookTarget - GetCurrentOrigin() + GetHeadOffset()).Normalized();
	lookDirection = lookDirection * 0.94f + dir * 0.06f;
	
	viewAngles = lookDirection.ToAngles();
}
