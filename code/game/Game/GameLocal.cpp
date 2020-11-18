#include "Maths/Vector.hpp"
#include "Game/g_local.hpp"

#include "Game/IGame.h"
#include "Game/IGameImports.h"
#include "Game/GameExportImport.h"
#include "Game/GameLocal.h"

#include "../qcommon/IEngineExports.h"
#include "Game/GameWorld.hpp"

#include "Entities/IEntity.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"

extern void G_InitGame( int levelTime, int randomSeed, int restart );
extern void G_ShutdownGame( int restart );
extern void ClientUserinfoChanged( int clientNum );
extern void G_RunFrame( int levelTime );

extern void G_RegisterCvars( void );
extern gclient_t g_clients[MAX_CLIENTS];

extern vmCvar_t	g_logfile;
extern vmCvar_t g_logfileSync;

// This is pretty damn ugly, but it'll work for now
// The engine has just begun to be C++-ified, so
// there's a lot of work to do
// 
// For example, instead of having to call global functions
// like G_InitGame, all related variables should be stored
// as members of the GameLocal class, and all the functionality
// should be moved there as well

void GameLocal::Init( int levelTime, int randomSeed, int restart )
{
	gameWorld = new GameWorld();

	G_Printf( "------- Game Initialization -------\n" );
	G_Printf( "gamename: %s\n", GAMEVERSION );
	G_Printf( "gamedate: %s\n", PRODUCT_DATE );

	srand( randomSeed );

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	level.snd_fry = G_SoundIndex( "sound/player/fry.wav" );	// FIXME standing in lava / slime

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_logfile.string[0] ) 
	{
		if ( g_logfileSync.integer ) 
		{
			trap_FS_FOpenFile( g_logfile.string, &level.logFile, FS_APPEND_SYNC );
		}
		
		else 
		{
			trap_FS_FOpenFile( g_logfile.string, &level.logFile, FS_APPEND );
		}
		
		if ( !level.logFile ) 
		{
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_logfile.string );
		}
	
		else 
		{
			char serverinfo[MAX_INFO_STRING];

			trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf( "------------------------------------------------------------\n" );
			G_LogPrintf( "InitGame: %s\n", serverinfo );
		}
	}
	
	else 
	{
		G_Printf( "Not logging to disk.\n" );
	}

	G_InitWorldSession();

	// initialize all entities for this game
	memset( gEntities, 0, MAX_GENTITIES * sizeof( gEntities[0] ) );
	level.gentities = nullptr;
	level.entities = gEntities;

	// initialize all clients for this game
	level.maxclients = g_maxclients.integer;
	memset( g_clients, 0, MAX_CLIENTS * sizeof( g_clients[0] ) );
	level.clients = g_clients;

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	level.num_entities = MAX_CLIENTS;

	// let the server system know where the entites are
	gameImports->LocateGameData(
		reinterpret_cast<sharedEntity_t*>(level.gentities), level.num_entities, sizeof( gentity_t ),
		level.entities, level.numEntities, sizeof( Entities::IEntity* ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	//InitBodyQue();

	ClearRegisteredItems();

	// parse the key/value pairs and spawn gentities
	gameWorld->SpawnEntities();
	G_SpawnEntitiesFromString();

	// general initialization
	FindTeams();

	// TODO: Implement this stuff at some point
	//// make sure we have flags for CTF, etc
	//if ( g_gametype.integer >= GT_TEAM ) 
	//{
	//	G_CheckTeamItems();
	//}

	SaveRegisteredItems();

	G_Printf( "-----------------------------------\n" );

	if ( g_gametype.integer == GT_SINGLE_PLAYER || gameImports->ConsoleVariable_GetInteger( "com_buildScript" ) )
	{
		G_ModelIndex( SP_PODIUM_MODEL );
	}

	if ( gameImports->ConsoleVariable_GetInteger( "bot_enable" ) )
	{	// TODO: Make bots work again, at some point
		//BotAISetup( restart );
		//BotAILoadMap( restart );
		//G_InitBots( (qboolean)restart );
	}

	gameImports->SetConfigString( CS_INTERMISSION, "" );
}

void GameLocal::Shutdown( bool restart )
{
	return ::G_ShutdownGame( restart );
}

const char* GameLocal::ClientConnect( int clientNum, bool firstTime, bool isBot )
{
	char* value;
	//	char		*areabits;
	gclient_t* client;
	char userinfo[MAX_INFO_STRING];
	Entities::BasePlayer* ent;

	// if a player reconnects quickly after a disconnect, the client disconnect may never be called, thus flag can get lost in the ether
	if ( gEntities[clientNum] )
	{
		G_LogPrintf( "Forcing disconnect on active client: %i\n", clientNum );
		// so lets just fix up anything that should happen on a disconnect
		ClientDisconnect( clientNum );
	}

	ent = gameWorld->CreateEntity<Entities::BasePlayer>( clientNum );

	gameImports->GetUserInfo( clientNum, userinfo, sizeof( userinfo ) );

	// IP filtering
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
	// recommanding PB based IP / GUID banning, the builtin system is pretty limited
	// check to see if they are on the banned IP list
	value = Info_ValueForKey( userinfo, "ip" );
	
	if ( G_FilterPacket( value ) ) 
	{
		return "You are banned from this server.";
	}

	// we don't check password for bots and local client
	// NOTE: local client <-> "ip" "localhost"
	//   this means this client is not running in our current process
	if ( !isBot && (strcmp( value, "localhost" ) != 0) ) 
	{
		// check for a password
		value = Info_ValueForKey( userinfo, "password" );
		if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
			 strcmp( g_password.string, value ) != 0 ) 
		{
			return "Invalid password";
		}
	}
	
	// they can connect
	ent->SetClient( &level.clients[clientNum] );
	client = ent->GetClient();

	memset( client, 0, sizeof( *client ) );

	client->pers.connected = CON_CONNECTING;

	// check for local client
	value = Info_ValueForKey( userinfo, "ip" );
	if ( !strcmp( value, "localhost" ) ) 
	{
		client->pers.localClient = qtrue;
	}

	// Admer: Bots aren't supported a.t.m.
	/*if ( isBot ) 
	{
		ent->GetShared()->svFlags |= SVF_BOT;
		if ( !G_BotConnect( clientNum, (qboolean)!firstTime ) ) 
		{
			return "BotConnectfailed";
		}
	}*/

	// read or initialize the session data
	if ( firstTime || level.newSession ) 
	{
		G_InitSessionData( client, userinfo );
	}
	
	G_ReadSessionData( client );

	// get and distribute relevant parameters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) 
	{
		trap_SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname ) );
	}

	if ( g_gametype.integer >= GT_TEAM && client->sess.sessionTeam != TEAM_SPECTATOR ) 
	{
		BroadcastTeamChange( client, -1 );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	return NULL;
}

void GameLocal::ClientBegin( int clientNum )
{
	Entities::IEntity* ent;
	Entities::BasePlayer* player;
	gclient_t* client;
	int			flags;

	//ent = g_entities + clientNum;
	ent = gEntities[clientNum];

	client = level.clients + clientNum;

	if ( nullptr != ent )
	{
		engine->Error( va( "Entity slot %d taken by another entity, yet it's reserved for clients!\n", clientNum ) );
		return;
	}

	// Does this really need to be checked?
	// Is this entity linked into the world?
	if ( ent->GetShared()->linked ) // Crash happins on dis line
	{
		gameImports->UnlinkEntity( ent );
	}

	ent = gameWorld->CreateEntity<Entities::BasePlayer>( clientNum );
	player = static_cast<Entities::BasePlayer*>(ent);

	player->GetShared()->ownerNum = ENTITYNUM_NONE;
	player->SetClient( client );

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	gameWorld->SpawnClient( player );

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// locate ent at a spawn point
	gameWorld->SpawnClient( player );

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) 
	{
		if ( g_gametype.integer != GT_TOURNAMENT ) 
		{
			gameImports->SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname ) );
		}
	}

	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();
}

void GameLocal::ClientUserInfoChanged( int clientNum )
{
	return ::ClientUserinfoChanged( clientNum );
}

void GameLocal::ClientDisconnect( int clientNum )
{
	return ::ClientDisconnect( clientNum );
}

void GameLocal::ClientCommand( int clientNum )
{
	Entities::IEntity* ent = gEntities[clientNum];

	if ( nullptr == ent )
		return;

	Entities::BasePlayer* player = dynamic_cast<Entities::BasePlayer*>( ent );

	if ( nullptr == player )
		return;

	return player->ClientCommand();
}

void GameLocal::ClientThink( int clientNum )
{
	return gameWorld->ClientThink( clientNum );
}

void GameLocal::RunFrame( int levelTime )
{
	return ::G_RunFrame( levelTime );
}

bool GameLocal::ConsoleCommand( void )
{
	return ::ConsoleCommand() != 0;
}

int GameLocal::BotAI_StartFrame( int time )
{
	return BotAIStartFrame( time );
}

void GameLocal::FindTeams()
{
	Entities::IEntity* e;
	Entities::IEntity* e2;
	
	int		i, j;
	int		c, c2;

	c = 0;
	c2 = 0;
	for ( i = MAX_CLIENTS, e = gEntities[i]; i < level.num_entities; i++, e++ ) {
		if ( nullptr == e )
			continue;
		//if ( !e->team )
		//	continue;
		//if ( e->GetFlags() & FL_TEAMSLAVE )
		//	continue;
		//e->teammaster = e;
		c++;
		c2++;
		for ( j = i + 1, e2 = e + 1; j < level.num_entities; j++, e2++ )
		{
			if ( nullptr == e )
				continue;
			//if ( !e2->team )
			//	continue;
			//if ( e2->GetFlags() & FL_TEAMSLAVE )
			//	continue;
			//if ( !strcmp( e->team, e2->team ) )
			{
				c2++;
				//e2->teamchain = e->teamchain;
				//e->teamchain = e2;
				//e2->teammaster = e;
				//e2->GetFlags() |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->GetName() ) 
				{
					//e->targetname = e2->targetname;
					//e2->targetname = NULL;
				}
			}
		}
	}

	G_Printf( "%i teams with %i entities\n", c, c2 );
}
