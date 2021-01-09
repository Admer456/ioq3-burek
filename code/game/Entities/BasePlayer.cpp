#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Maths/Vector.hpp"

#include "Weapons/BaseWeapon.hpp"

#include "BasePlayer.hpp"

using namespace Entities;

DefineEntityClass_NoMapSpawn( BasePlayer, BaseQuakeEntity );

void BasePlayer::Spawn()
{
	//GetState()->clipFlags |= ClipFlag_HasOriginBrush;
	//GetShared()->svFlags |= SVF_USE_CURRENT_ORIGIN;
}

void BasePlayer::TakeDamage( IEntity* inflictor, IEntity* attacker, int damageFlags, float damageDealt )
{
	int			take;
	//int			asave;
	int			knockback;
	int			max;

	if ( !takeDamage ) 
	{
		return;
	}

	damage = damageDealt;

	// the intermission has already been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) 
	{
		return;
	}

	if ( !inflictor ) 
	{
		inflictor = gEntities[ENTITYNUM_WORLD];
	}

	if ( !attacker ) 
	{
		attacker = gEntities[ENTITYNUM_WORLD];
	}

	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if ( attacker->IsSubclassOf( BasePlayer::ClassInfo ) && attacker != this ) 
	{
		max = static_cast<BasePlayer*>(attacker)->GetClient()->ps.stats[STAT_MAX_HEALTH];
		damage = damage * max / 100;
	}

	if ( client ) 
	{
		if ( client->noclip ) 
		{
			return;
		}
	}

	knockback = damage;
	if ( knockback > 200 ) 
	{
		knockback = 200;
	}
	
	if ( flags & FL_NO_KNOCKBACK ) 
	{
		knockback = 0;
	}
	
	if ( damageFlags & DAMAGE_NO_KNOCKBACK ) 
	{
		knockback = 0;
	}

	// check for completely getting out of the damage
	if ( !(damageFlags & DAMAGE_NO_PROTECTION) ) 
	{
		//// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		//// if the attacker was on the same team	
		//if ( targ != attacker && OnSameTeam( targ, attacker ) ) 
		//{
		//	if ( !g_friendlyFire.integer ) 
		//	{
		//		return;
		//	}
		//}

		// check for godmode
		if ( flags & FL_GODMODE ) 
		{
			return;
		}
	}

	//// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	//if ( attacker->client && client
	//	 && targ != attacker && targ->health > 0
	//	 && targ->s.eType != ET_MISSILE
	//	 && targ->s.eType != ET_GENERAL ) 
	//{
	//	if ( OnSameTeam( targ, attacker ) ) 
	//	{
	//		attacker->client->ps.persistant[PERS_HITS]--;
	//	}
	//	else 
	//	{
	//		attacker->client->ps.persistant[PERS_HITS]++;
	//	}
	//	attacker->client->ps.persistant[PERS_ATTACKEE_ARMOR] = (targ->health << 8) | (client->ps.stats[STAT_ARMOR]);
	//}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( this == attacker ) 
	{
		damage *= 0.5;
	}

	if ( damage < 1 ) 
	{
		damage = 1;
	}
	
	take = damage;

	// TODO: implement armour
	//// save some from armor
	//asave = CheckArmor( targ, take, dflags );
	//take -= asave;

	if ( g_debugDamage.integer ) 
	{
		G_Printf( "%i: client:%i health:%i damage:%i armor:%i\n", level.time, GetEntityIndex(),
				  health, take, 0 /*asave*/ );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) 
	{
		if ( attacker ) 
		{
			client->ps.persistant[PERS_ATTACKER] = attacker->GetEntityIndex();
		}
		else 
		{
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += 0 /*asave*/;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		
		VectorCopy( GetShared()->currentOrigin, client->damage_from );
		client->damage_fromWorld = qtrue;

	}

	if ( client ) 
	{
		// set the last client who damaged the target
		client->lasthurt_client = attacker->GetEntityIndex();
	}

	// do the damage
	if ( take ) 
	{
		health = health - take;

		if ( client ) 
		{
			client->ps.stats[STAT_HEALTH] = health;
		}

		if ( health <= 0 ) 
		{
			if ( client )
				flags |= FL_NO_KNOCKBACK;

			if ( health < -999 )
				health = -999;

			return;
		}
	}
}

gclient_t* BasePlayer::GetClient()
{
	return client;
}

void BasePlayer::SetClient( const gclient_t* playerClient )
{
	client = const_cast<gclient_t*>( playerClient );
}

const Vector& BasePlayer::GetClientViewAngle() const
{
	return Vector( client->ps.viewangles );
}

void BasePlayer::SetClientViewAngle( const Vector& newAngle )
{
	int			i;

	// set the delta angle
	for ( i = 0; i < 3; i++ ) 
	{
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT( newAngle[i] );
		client->ps.delta_angles[i] = cmdAngle - client->pers.cmd.angles[i];
	}

	VectorCopy( newAngle, shared.s.angles );
	VectorCopy( shared.s.angles, client->ps.viewangles );
}

void BasePlayer::AddWeapon( BaseWeapon* weapon )
{
	int weaponID = weapon->GetWeaponInfo().weaponID;

	if ( !HasWeapon( weaponID ) )
	{
		weapons[weaponID] = weapon;
		client->ps.stats[STAT_WEAPONS] |= (1 << weaponID);
	}
}

BaseWeapon* BasePlayer::GetCurrentWeapon()
{
	return currentWeapon;
}

bool BasePlayer::HasAnyWeapon()
{
	for ( BaseWeapon* weapon : weapons )
	{
		if ( weapon )
			return true;
	}

	return false;
}

bool BasePlayer::HasWeapon( int weaponID )
{
	return weapons[weaponID] != nullptr;
}

void BasePlayer::SendWeaponEvent( uint32_t weaponEvent )
{
	if ( nullptr == currentWeapon )
		return;

	switch ( weaponEvent )
	{
	case WE_DoDraw:				return currentWeapon->Draw();
	case WE_DoHolster:			return currentWeapon->Holster();
	case WE_DoPrimaryAttack:	return currentWeapon->PrimaryAttack();
	case WE_DoSecondaryAttack:	return currentWeapon->SecondaryAttack();
	case WE_DoTertiaryAttack:	return currentWeapon->TertiaryAttack();
	case WE_DoReload:			return currentWeapon->Reload();
	}
}

void BasePlayer::ClientCommand()
{
	char cmd[MAX_TOKEN_CHARS];

	if ( !client || client->pers.connected != CON_CONNECTED ) 
	{
		if ( client && client->pers.localClient ) 
		{
			// Handle early team command sent by UI when starting a local
			// team play game.
			engine->ArgV( 0, cmd, sizeof( cmd ) );
			
			if ( Q_stricmp( cmd, "team" ) == 0 ) 
			{
				Command_Team();
			}
		}

		return;		// not fully in game yet
	}

	engine->ArgV( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp( cmd, "say" ) == 0 ) 
	{
		Command_Say( SAY_ALL, false );
		return;
	}

	if ( Q_stricmp( cmd, "say_team" ) == 0 ) 
	{
		Command_Say( SAY_TEAM, false );
		return;
	}

	if ( Q_stricmp( cmd, "tell" ) == 0 ) 
	{
		Command_Tell();
		return;
	}

	if ( Q_stricmp( cmd, "score" ) == 0 ) 
	{
		Command_Score();
		return;
	}

	// ignore all other commands when at intermission
	if ( level.intermissiontime ) 
	{
		Command_Say( 0, true );
		return;
	}

	if ( Q_stricmp( cmd, "give" ) == 0 )
		Command_Give();

	else if ( Q_stricmp( cmd, "god" ) == 0 )
		Command_God();

	else if ( Q_stricmp( cmd, "notarget" ) == 0 )
		Command_Notarget();

	else if ( Q_stricmp( cmd, "noclip" ) == 0 )
		Command_Noclip();

	else if ( Q_stricmp( cmd, "kill" ) == 0 )
		Command_Kill();

	else if ( Q_stricmp( cmd, "teamtask" ) == 0 )
		Command_TeamTask();

	else if ( Q_stricmp( cmd, "levelshot" ) == 0 )
		Command_LevelShot();

	else if ( Q_stricmp( cmd, "follow" ) == 0 )
		Command_Follow();

	else if ( Q_stricmp( cmd, "follownext" ) == 0 )
		FollowCycle( 1 );
	
	else if ( Q_stricmp( cmd, "followprev" ) == 0 )
		FollowCycle( -1 );
	
	else if ( Q_stricmp( cmd, "team" ) == 0 )
		Command_Team();
	
	else if ( Q_stricmp( cmd, "where" ) == 0 )
		Command_Where();
	
	else if ( Q_stricmp( cmd, "callvote" ) == 0 )
		Command_CallVote();
	
	else if ( Q_stricmp( cmd, "vote" ) == 0 )
		Command_Vote();
	
	else if ( Q_stricmp( cmd, "callteamvote" ) == 0 )
		Command_CallTeamVote();
	
	else if ( Q_stricmp( cmd, "teamvote" ) == 0 )
		Command_TeamVote();
	
	else if ( Q_stricmp( cmd, "gc" ) == 0 )
		Command_GameCommand();
	
	else if ( Q_stricmp( cmd, "setviewpos" ) == 0 )
		Command_SetViewpos();
	
	else if ( Q_stricmp( cmd, "stats" ) == 0 )
		Command_Stats();
	
	else
		gameImports->SendServerCommand( GetEntityIndex(), va( "print \"unknown cmd %s\n\"", cmd ) );
}

void BasePlayer::PlayerUse()
{
	isUsing = client->interactionButtons & Interaction_Use;
	wasUsing = client->oldInteractionButtons & Interaction_Use;

	Vector eyes = GetViewOrigin();
	Vector viewAngles = Vector( client->ps.viewangles );
	Vector forward;
	trace_t tr;

	// Calculate the view direction based on view angles
	Vector::AngleVectors( viewAngles, &forward, nullptr, nullptr );

	// TODO: Simplify some of this
	gameImports->Trace( &tr, eyes, nullptr, nullptr, eyes + forward * 64.0f, GetEntityIndex(), CONTENTS_SOLID );

	// Didn't hit anything, return
	if ( tr.fraction == 1.0f )
		return;

	// Hit a player, worldspawn, or none, return
	if ( tr.entityNum < MAX_CLIENTS || tr.entityNum >= ENTITYNUM_MAX_NORMAL )
		return;

	// Use the entity
	if ( gEntities[tr.entityNum] )
		gEntities[tr.entityNum]->Use( this, this, 0 );
}

Vector BasePlayer::GetViewOrigin()
{
	return Vector( client->ps.origin ) + Vector( 0, 0, client->ps.viewheight );
}

void BasePlayer::CopyToBodyQue()
{
	IEntity* body;
	int contents;

	gameImports->UnlinkEntity( this );

	contents = gameImports->PointContents( GetOrigin(), -1 );
	if ( contents & CONTENTS_NODROP )
	{
		return;
	}

	// Todo: do the rest
	// like body = gameWorld->CreateEntity<PlayerCorpse>();
}

void BasePlayer::AddEvent( int event, int eventParameter )
{
	int	bits = 0;

	if ( !event ) 
	{
		G_Printf( "G_AddEvent: zero event added for entity %i\n", GetEntityIndex() );
		return;
	}

	bits = client->ps.externalEvent & EV_EVENT_BITS;
	bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;
	client->ps.externalEvent = event | bits;
	client->ps.externalEventParm = eventParameter;
	client->ps.externalEventTime = level.time;

	eventTime = level.time;
}

void BasePlayer::FireWeapon()
{
	SendWeaponEvent( WE_DoPrimaryAttack );
}

void BasePlayer::SetTeam( const char* teamName )
{

}

void BasePlayer::StopFollowing()
{
	client->ps.persistant[PERS_TEAM] = TEAM_SPECTATOR;
	client->sess.sessionTeam = TEAM_SPECTATOR;
	client->sess.spectatorState = SPECTATOR_FREE;
	client->ps.pm_flags &= ~PMF_FOLLOW;
	GetShared()->svFlags &= ~SVF_BOT;
	client->ps.clientNum = GetEntityIndex();

	SetClientViewAngle( client->ps.viewangles );

	// don't use dead view angles
	if ( client->ps.stats[STAT_HEALTH] <= 0 ) 
	{
		client->ps.stats[STAT_HEALTH] = 1;
	}
}

void BasePlayer::FollowCycle( int dir )
{
	int		clientnum;
	int		original;

	// if they are playing a tournement game, count as a loss
	if ( (g_gametype.integer == GT_TOURNAMENT)
		 && client->sess.sessionTeam == TEAM_FREE ) 
	{
		client->sess.losses++;
	}

	// first set them to spectator
	if ( client->sess.spectatorState == SPECTATOR_NOT ) 
	{
		SetTeam( "spectator" );
	}

	if ( dir != 1 && dir != -1 ) {
		G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
	}

	// if dedicated follow client, just switch between the two auto clients
	if ( client->sess.spectatorClient < 0 ) 
	{
		if ( client->sess.spectatorClient == -1 ) 
		{
			client->sess.spectatorClient = -2;
		}

		else if ( client->sess.spectatorClient == -2 ) 
		{
			client->sess.spectatorClient = -1;
		}

		return;
	}

	clientnum = client->sess.spectatorClient;
	original = clientnum;

	do 
	{
		clientnum += dir;
		if ( clientnum >= level.maxclients ) 
		{
			clientnum = 0;
		}

		if ( clientnum < 0 ) 
		{
			clientnum = level.maxclients - 1;
		}

		// can only follow connected clients
		if ( level.clients[clientnum].pers.connected != CON_CONNECTED ) 
		{
			continue;
		}

		// can't follow another spectator
		if ( level.clients[clientnum].sess.sessionTeam == TEAM_SPECTATOR ) 
		{
			continue;
		}

		// this is good, we can use it
		client->sess.spectatorClient = clientnum;
		client->sess.spectatorState = SPECTATOR_FOLLOW;
		return;
	} while ( clientnum != original );

	// leave it where it was
}

void Entities::BasePlayer::WorldEffects()
{
	bool		envirosuit;
	int			_waterlevel;

	if ( client->noclip ) {
		client->airOutTime = level.time + 12000;	// don't need air
		return;
	}

	_waterlevel = waterLevel;

	envirosuit = (client->ps.powerups[PW_BATTLESUIT] > level.time);

	// check for drowning
	if ( waterLevel == 3 ) 
	{
		// envirosuit give air
		if ( envirosuit ) 
		{
			client->airOutTime = level.time + 10000;
		}

		// if out of air, start drowning
		if ( client->airOutTime < level.time ) 
		{
			// drown!
			client->airOutTime += 1000;
			if ( health > 0 ) 
			{
				// take more damage the longer underwater
				damage += 2;
				if ( damage > 15 )
					damage = 15;

				// don't play a normal pain sound
				painDebounceTime = level.time + 200;

				TakeDamage( nullptr, nullptr, DAMAGE_NO_ARMOR, damage );
			}
		}
	}
	else 
	{
		client->airOutTime = level.time + 12000;
		damage = 2;
	}

	// check for sizzle damage (move to pmove?)
	if ( waterLevel &&
		 (waterType & (CONTENTS_LAVA | CONTENTS_SLIME)) ) 
	{
		if ( health > 0 && painDebounceTime <= level.time ) 
		{

			if ( envirosuit ) 
			{
				AddEvent( EV_POWERUP_BATTLESUIT, 0 );
			}
			else 
			{
				if ( waterType & CONTENTS_LAVA ) 
				{
					TakeDamage( nullptr, nullptr, 0, 30 * waterLevel );
				}

				if ( waterType & CONTENTS_SLIME ) 
				{
					TakeDamage( nullptr, nullptr, 0, 10 * waterLevel );
				}
			}
		}
	}
}

void Entities::BasePlayer::ApplyDamage()
{
	float	count;
	vec3_t	angles;

	if ( client->ps.pm_type == PM_DEAD ) 
	{
		return;
	}

	// total points of damage shot at the player this frame
	count = client->damage_blood + client->damage_armor;
	if ( count == 0 ) 
	{
		return;	// didn't take any damage
	}

	if ( count > 255 ) 
	{
		count = 255;
	}

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if ( client->damage_fromWorld ) 
	{
		client->ps.damagePitch = 255;
		client->ps.damageYaw = 255;

		client->damage_fromWorld = qfalse;
	}
	else 
	{
		vectoangles( client->damage_from, angles );
		client->ps.damagePitch = angles[PITCH] / 360.0 * 256;
		client->ps.damageYaw = angles[YAW] / 360.0 * 256;
	}

	// play an appropriate pain sound
	if ( (level.time > painDebounceTime) && !(GetFlags() & FL_GODMODE) ) 
	{
		painDebounceTime = level.time + 700;
		AddEvent( EV_PAIN, health );
		client->ps.damageEvent++;
	}


	client->ps.damageCount = count;

	// clear totals
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_knockback = 0;
}

void BasePlayer::Teleport( const Vector& toOrigin, const Vector& toAngles )
{
	IEntity* tent;
	bool noAngles;

	noAngles = (GetCurrentAngles()[0] > 999999.0);
	// use temp events at source and destination to prevent the effect
	// from getting dropped by a second player event
	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) 
	{
		tent = gameWorld->CreateTempEntity( client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->GetState()->clientNum = GetState()->clientNum;

		tent = gameWorld->CreateTempEntity( GetCurrentOrigin(), EV_PLAYER_TELEPORT_IN );
		tent->GetState()->clientNum = GetState()->clientNum;
	}

	// unlink to make sure it can't possibly interfere with G_KillBox
	gameImports->UnlinkEntity( this );

	VectorCopy( toOrigin, client->ps.origin );
	client->ps.origin[2] += 1;
	if ( !noAngles ) 
	{
		// spit the player out
		AngleVectors( client->ps.viewangles, client->ps.velocity, NULL, NULL );
		VectorScale( client->ps.velocity, 400, client->ps.velocity );
		client->ps.pm_time = 160;		// hold time
		client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		// set angles
		SetClientViewAngle( toAngles );
	}
	// toggle the teleport bit so the client knows to not lerp
	client->ps.eFlags ^= EF_TELEPORT_BIT;
	
	// kill anything at the destination
	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) 
	{
		KillBox();
	}

	// save results of pmove
	BG_PlayerStateToEntityState( &client->ps, GetState(), qtrue );

	// use the precise origin for linking
	VectorCopy( client->ps.origin, GetShared()->currentOrigin );

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) 
	{
		gameImports->LinkEntity( this );
	}
}

// Players test their entity position from ps.origin
// instead of s.pos.trBase in most other ents
// This fixed the "Stupid Mover Bug"
BaseQuakeEntity* BasePlayer::TestEntityPosition()
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

	gameImports->Trace( &tr, GetClient()->ps.origin, shared.r.mins, shared.r.maxs, GetClient()->ps.origin, GetEntityIndex(), mask );

	if ( tr.startsolid )
		return static_cast<BaseQuakeEntity*>(gEntities[tr.entityNum]);

	return nullptr;
}
