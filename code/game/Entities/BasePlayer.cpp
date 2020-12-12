#include "Game/g_local.hpp"
#include "BaseEntity.hpp"
#include "Entities/KeyValueElement.hpp"
#include "Game/GameWorld.hpp"
#include "../qcommon/IEngineExports.h"
#include "Game/IGameImports.h"
#include "Maths/Vector.hpp"

#include "BasePlayer.hpp"

using namespace Entities;

DefineEntityClass_NoMapSpawn( BasePlayer, BaseQuakeEntity );

void BasePlayer::Spawn()
{
	// Do nothing
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

	// clients need to add the event in playerState_t instead of entityState_t
	if ( client ) 
	{
		bits = client->ps.externalEvent & EV_EVENT_BITS;
		bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;
		client->ps.externalEvent = event | bits;
		client->ps.externalEventParm = eventParameter;
		client->ps.externalEventTime = level.time;
	}
	
	else 
	{
		bits = shared.s.event & EV_EVENT_BITS;
		bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;
		shared.s.event = event | bits;
		shared.s.eventParm = eventParameter;
	}
	
	eventTime = level.time;
}

void BasePlayer::FireWeapon()
{

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
