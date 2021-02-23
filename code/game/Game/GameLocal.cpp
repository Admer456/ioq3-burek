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

extern gclient_t g_clients[MAX_CLIENTS];

extern vmCvar_t g_logfile;
extern vmCvar_t g_logfileSync;
extern vmCvar_t g_listEntity;
extern vmCvar_t g_podiumDist;
extern vmCvar_t g_podiumDrop;

typedef struct
{
	vmCvar_t* vmCvar;
	char* cvarName;
	char* defaultString;
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	qboolean	trackChange;	    // track this variable, and announce if changed
	qboolean teamShader;        // track and if changed, update shader state
} cvarTable_t;

static cvarTable_t gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0, qfalse },

	// noset vars
	{ NULL, "gamename", GAMEVERSION , CVAR_SERVERINFO | CVAR_ROM, 0, qfalse  },
	{ NULL, "gamedate", PRODUCT_DATE , CVAR_ROM, 0, qfalse  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, qfalse  },

	// latched vars
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH, 0, qfalse  },

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, qfalse  },

	// change anytime vars
	{ &g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, qtrue  },
	{ &g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },
	{ &g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, qtrue },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, qfalse  },

	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, qtrue  },

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_doWarmup, "g_doWarmup", "0", CVAR_ARCHIVE, 0, qtrue  },
	{ &g_logfile, "g_log", "games.log", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_logfileSync, "g_logsync", "0", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, qfalse  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, qfalse  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, qfalse  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, qfalse },

	{ &g_dedicated, "dedicated", "0", 0, 0, qfalse  },

	{ &g_speed, "g_speed", "320", 0, 0, qtrue  },
	{ &g_gravity, "g_gravity", "800", 0, 0, qtrue  },
	{ &g_knockback, "g_knockback", "1000", 0, 0, qtrue  },
	{ &g_quadfactor, "g_quadfactor", "3", 0, 0, qtrue  },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, qtrue  },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, qtrue },
	{ &g_forcerespawn, "g_forcerespawn", "20", 0, 0, qtrue },
	{ &g_inactivity, "g_inactivity", "0", 0, 0, qtrue },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, qfalse },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, qfalse },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, qfalse },
	{ &g_motd, "g_motd", "", 0, 0, qfalse },
	{ &g_blood, "com_blood", "1", 0, 0, qfalse },

	{ &g_podiumDist, "g_podiumDist", "80", 0, 0, qfalse },
	{ &g_podiumDrop, "g_podiumDrop", "70", 0, 0, qfalse },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, qfalse },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, qfalse },

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, qfalse},
	{ &pmove_fixed, "pmove_fixed", "0", CVAR_SYSTEMINFO, 0, qfalse},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, qfalse},

	{ &g_rankings, "g_rankings", "0", 0, 0, qfalse},
	{ &g_localTeamPref, "g_localTeamPref", "", 0, 0, qfalse }
};

static int gameCvarTableSize = ARRAY_LEN( gameCvarTable );

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
	EntityClassInfo::SetupSuperClasses();

	gameWorld = new GameWorld();

	G_Printf( "------- Game Initialization -------\n" );
	G_Printf( "gamename: %s\n", GAMEVERSION );
	G_Printf( "gamedate: %s\n", PRODUCT_DATE );

	srand( randomSeed );

	RegisterCVars();

	G_ProcessIPBans();

	// set some level globals
	memset( &level, 0, sizeof( level ) );
	level.time = levelTime;
	level.startTime = levelTime;

	level.snd_fry = G_SoundIndex( "sound/player/fry.wav" );	// FIXME standing in lava / slime

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_logfile.string[0] ) 
	{
		if ( g_logfileSync.integer ) 
		{
			Util::FileOpen( g_logfile.string, &level.logFile, FS_APPEND_SYNC );
		}
		
		else 
		{
			Util::FileOpen( g_logfile.string, &level.logFile, FS_APPEND );
		}
		
		if ( !level.logFile ) 
		{
			G_Printf( "WARNING: Couldn't open logfile: %s\n", g_logfile.string );
		}
	
		else 
		{
			char serverinfo[MAX_INFO_STRING];

			gameImports->GetServerInfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf( "------------------------------------------------------------\n" );
			G_LogPrintf( "InitGame: %s\n", serverinfo );
		}
	}
	
	else 
	{
		G_Printf( "Not logging to disk.\n" );
	}

	//InitWorldSession();

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
		/*reinterpret_cast<sharedEntity_t*>(level.gentities)*/ nullptr, level.num_entities, /*sizeof( gentity_t )*/ 0,
		level.entities, level.numEntities, sizeof( Entities::IEntity* ),
		&level.clients[0].ps, sizeof( level.clients[0] ) );

	// reserve some spots for dead player bodies
	//InitBodyQue();

	//ClearRegisteredItems();

	// parse the key/value pairs and spawn gentities
	gameWorld->SpawnEntities();
	//G_SpawnEntitiesFromString();

	// general initialization
	FindTeams();

	// TODO: Implement this stuff at some point
	//// make sure we have flags for CTF, etc
	//if ( g_gametype.integer >= GT_TEAM ) 
	//{
	//	G_CheckTeamItems();
	//}

	//SaveRegisteredItems();

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
	Util::Print( "==== ShutdownGame ====\n" );

	if ( level.logFile ) {
		G_LogPrintf( "ShutdownGame:\n" );
		G_LogPrintf( "------------------------------------------------------------\n" );
		Util::FileClose( level.logFile );
		level.logFile = 0;
	}

	// write all the client session data so we can get it back
	//WriteSessionData();

	//if ( gameImports->ConsoleVariable_GetInteger( "bot_enable" ) ) 
	//{
	//	BotAIShutdown( restart );
	//}

	gameWorld->Shutdown();
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
	if ( isBot ) 
	{
		ent->GetShared()->svFlags |= SVF_BOT;
		//if ( !G_BotConnect( clientNum, (qboolean)!firstTime ) ) 
		//{
			return "BotConnectfailed";
		//}
	}

	// read or initialize the session data
	if ( firstTime || level.newSession ) 
	{
		InitSessionData( client, userinfo );
	}
	
	ReadSessionData( client );

	// get and distribute relevant parameters
	G_LogPrintf( "ClientConnect: %i\n", clientNum );
	ClientUserInfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) 
	{
		gameImports->SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname ) );
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

	ent = gEntities[clientNum];

	client = &level.clients[clientNum];

	// Is this entity linked into the world?
	if ( ent->GetShared()->linked )
	{
		gameImports->UnlinkEntity( ent );
	}

	player = static_cast<Entities::BasePlayer*>(ent);

	player->GetShared()->ownerNum = ENTITYNUM_NONE;
	player->SetClient( client );

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

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

	for ( int i = 0; i < GameWorld::MaxEntities; i++ )
	{
		if ( gEntities[i] )
			gEntities[i]->OnClientBegin( clientNum );
	}

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
	Entities::IEntity* ent;
	int	teamTask, teamLeader, health;
	char* s;
	char model[MAX_QPATH];
	char headModel[MAX_QPATH];
	char oldname[MAX_STRING_CHARS];
	gclient_t* client;
	char c1[MAX_INFO_STRING];
	char c2[MAX_INFO_STRING];
	char redTeam[MAX_INFO_STRING];
	char blueTeam[MAX_INFO_STRING];
	char userinfo[MAX_INFO_STRING];

	ent = gEntities[clientNum];
	client = static_cast<Entities::BasePlayer*>(ent)->GetClient();

	gameImports->GetUserInfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if ( !Info_Validate( userinfo ) ) 
	{
		strcpy( userinfo, "\\name\\badinfo" );
		// don't keep those clients and userinfo
		gameImports->DropClient( clientNum, "Invalid userinfo" );
	}

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if ( !atoi( s ) ) 
	{
		client->pers.predictItemPickup = qfalse;
	}
	else 
	{
		client->pers.predictItemPickup = qtrue;
	}

	// set name
	Q_strncpyz( oldname, client->pers.netname, sizeof( oldname ) );
	s = Info_ValueForKey( userinfo, "name" );
	ClientCleanName( s, client->pers.netname, sizeof( client->pers.netname ) );

	if ( client->sess.sessionTeam == TEAM_SPECTATOR ) 
	{
		if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) 
		{
			Q_strncpyz( client->pers.netname, "scoreboard", sizeof( client->pers.netname ) );
		}
	}

	if ( client->pers.connected == CON_CONNECTED ) 
	{
		if ( strcmp( oldname, client->pers.netname ) ) 
		{
			gameImports->SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
									client->pers.netname ) );
		}
	}

	// set max health
	health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	client->pers.maxHealth = health;
	if ( client->pers.maxHealth < 1 || client->pers.maxHealth > 100 ) 
	{
		client->pers.maxHealth = 100;
	}
	client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

	// set model
	if ( g_gametype.integer >= GT_TEAM ) 
	{
		Q_strncpyz( model, Info_ValueForKey( userinfo, "team_model" ), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey( userinfo, "team_headmodel" ), sizeof( headModel ) );
	}
	else 
	{
		Q_strncpyz( model, Info_ValueForKey( userinfo, "model" ), sizeof( model ) );
		Q_strncpyz( headModel, Info_ValueForKey( userinfo, "headmodel" ), sizeof( headModel ) );
	}

	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	
	if ( !*s || atoi( s ) != 0 ) 
	{
		client->pers.teamInfo = qtrue;
	}
	else 
	{
		client->pers.teamInfo = qfalse;
	}

	// team task (0 = none, 1 = offence, 2 = defence)
	teamTask = atoi( Info_ValueForKey( userinfo, "teamtask" ) );

	// team Leader (1 = leader, 0 is normal player)
	teamLeader = client->sess.teamLeader;

	// colors
	Q_strncpyz( c1, Info_ValueForKey( userinfo, "color1" ), sizeof( c1 ) );
	Q_strncpyz( c2, Info_ValueForKey( userinfo, "color2" ), sizeof( c2 ) );

	Q_strncpyz( redTeam, Info_ValueForKey( userinfo, "g_redteam" ), sizeof( redTeam ) );
	Q_strncpyz( blueTeam, Info_ValueForKey( userinfo, "g_blueteam" ), sizeof( blueTeam ) );

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	if ( ent->GetShared()->svFlags & SVF_BOT )
	{
		s = va( "n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d",
				client->pers.netname, client->sess.sessionTeam, model, headModel, c1, c2,
				client->pers.maxHealth, client->sess.wins, client->sess.losses,
				Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader );
	}
	else
	{
		s = va( "n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d",
				client->pers.netname, client->sess.sessionTeam, model, headModel, redTeam, blueTeam, c1, c2,
				client->pers.maxHealth, client->sess.wins, client->sess.losses, teamTask, teamLeader );
	}

	gameImports->SetConfigString( CS_PLAYERS + clientNum, s );

	// this is not the userinfo, more like the configstring actually
	G_LogPrintf( "GameLocal::ClientUserInfoChanged: %i %s\n", clientNum, s );
}

void GameLocal::ClientDisconnect( int clientNum )
{
	Entities::BasePlayer* ent;
	//Entities::IEntity* tent;
	int i;

	// Bot support missing -Admer
	//// cleanup if we are kicking a bot that
	//// hasn't spawned yet
	//G_RemoveQueuedBotBegin( clientNum );

	if ( nullptr == gEntities[clientNum] )
		return;

	for ( int i = 0; i < GameWorld::MaxEntities; i++ )
	{
		if ( gEntities[i] )
			gEntities[i]->OnClientDisconnect( clientNum );
	}

	ent = static_cast<Entities::BasePlayer*>( gEntities[clientNum] );

	if ( !ent->GetClient() || ent->GetClient()->pers.connected == CON_DISCONNECTED ) 
	{
		return;
	}

	// stop any following clients
	for ( i = 0; i < level.maxclients; i++ ) 
	{
		if ( level.clients[i].sess.sessionTeam == TEAM_SPECTATOR
			 && level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
			 && level.clients[i].sess.spectatorClient == clientNum ) 
		{
			Entities::BasePlayer* follower = static_cast<Entities::BasePlayer*>( gEntities[i] );
			follower->StopFollowing();
		}
	}

	// Temp entity stuff is missing -Admer
	//// send effect if they were completely connected
	//if ( ent->GetClient()->pers.connected == CON_CONNECTED
	//	 && ent->GetClient()->sess.sessionTeam != TEAM_SPECTATOR ) 
	//{
	//	tent = G_TempEntity( ent->GetClient()->ps.origin, EV_PLAYER_TELEPORT_OUT );
	//	tent->s.clientNum = ent->s.clientNum;
	//
	//	// They don't get to take powerups with them!
	//	// Especially important for stuff like CTF flags
	//	TossClientItems( ent );
	//}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	// if we are playing in tourney mode and losing, give a win to the other player
	if ( (g_gametype.integer == GT_TOURNAMENT)
		 && !level.intermissiontime
		 && !level.warmupTime && level.sortedClients[1] == clientNum ) 
	{
		level.clients[level.sortedClients[0]].sess.wins++;
		ClientUserInfoChanged( level.sortedClients[0] );
	}

	if ( g_gametype.integer == GT_TOURNAMENT &&
		 ent->GetClient()->sess.sessionTeam == TEAM_FREE &&
		 level.intermissiontime ) 
	{

		engine->SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
		level.restarted = qtrue;
		level.changemap = NULL;
		level.intermissiontime = 0;
	}

	gameImports->UnlinkEntity( ent );
	ent->GetState()->modelindex = 0;
	ent->className = "disconnected";
	ent->GetClient()->pers.connected = CON_DISCONNECTED;
	ent->GetClient()->ps.persistant[PERS_TEAM] = TEAM_FREE;
	ent->GetClient()->sess.sessionTeam = TEAM_FREE;

	gameImports->SetConfigString( CS_PLAYERS + clientNum, "" );

	CalculateRanks();

	// Bots aren't supported yet -Admer
	//if ( ent->r.svFlags & SVF_BOT ) 
	//{
	//	BotAIShutdownClient( clientNum, qfalse );
	//}
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
	int	i;

	// if we are waiting for the level to restart, do nothing
	if ( level.restarted ) 
	{
		return;
	}

	level.framenum++;
	level.previousTime = level.time;
	level.time = levelTime;

	// get any cvar changes
	UpdateCVars();

	Entities::BaseEntity* gent = nullptr;
	for ( auto ent : gEntities )
	{
		if ( nullptr == ent )
			continue;

		gent = static_cast<Entities::BaseEntity*>(ent);

		if ( ent->GetFlags() & FL_REMOVE_ME )
		{
			gameWorld->FreeEntity( ent );
			continue;
		}

		// clear events that are too old
		if ( level.time - gent->eventTime > EVENT_VALID_MSEC ) 
		{
			if ( ent->GetState()->event ) 
			{
				ent->GetState()->event = 0;	// &= EV_EVENT_BITS;
				if ( ent->IsClass( Entities::BasePlayer::ClassInfo ) ) 
				{
					Entities::BasePlayer* player = dynamic_cast<Entities::BasePlayer*>(ent);

					static_cast<Entities::BasePlayer*>(ent)->GetClient()->ps.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if ( gent->freeAfterEvent ) 
			{
				// tempEntities or dropped items completely go away after their event
				gameWorld->FreeEntity( ent );
				continue;
			}
			
			else if ( gent->unlinkAfterEvent ) 
			{
				// items that will respawn will hide themselves after their pickup event
				gent->unlinkAfterEvent = qfalse;
				gameImports->UnlinkEntity( ent );
			}
		}

		if ( ent->GetEntityIndex() < MAX_CLIENTS )
		{
			gameWorld->RunClient( static_cast<Entities::BasePlayer*>( ent ) );
			continue;
		}

		// Clear the force start animation flag
		ent->GetState()->animationFlags &= ~AnimFlag_ForceStart;

		ent->Think();
	}

	// Perform final fixups on the players
	for ( i = 0; i < level.maxclients; i++ )
	{
		Entities::IEntity* ent = gEntities[i];
		if ( nullptr == ent )
			continue;

		auto player = static_cast<Entities::BasePlayer*>(ent);
		gameWorld->ClientEndFrame( player );
	}

	//// See if it is time to do a tournement restart
	//::CheckTournament();

	// See if it is time to end the level
	CheckExitRules();

	//// Update to team status?
	//::CheckTeamStatus();

	//// Cancel vote if timed out
	//::CheckVote();

	//// Check team votes
	//::CheckTeamVote( TEAM_RED );
	//::CheckTeamVote( TEAM_BLUE );

	// For tracking changes
	CheckCVars();

	if ( g_listEntity.integer ) 
	{
		for ( i = 0; i < MAX_GENTITIES; i++ ) 
		{
			G_Printf( "%4i: %s\n", i, g_entities[i].classname );
		}
		
		gameImports->ConsoleVariable_Set( "g_listEntity", "0" );
	}
}

bool GameLocal::ConsoleCommand( void )
{
	return ::ConsoleCommand() != 0;
}

int GameLocal::BotAI_StartFrame( int time )
{
	return 0;
	//return BotAIStartFrame( time );
}

void GameLocal::FindTeams()
{
	return;

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

void GameLocal::ClientCleanName( const char* in, char* out, int outSize )
{
	int outpos = 0, colorlessLen = 0, spaces = 0;

	// discard leading spaces
	for ( ; *in == ' '; in++ );

	for ( ; *in && outpos < outSize - 1; in++ )
	{
		out[outpos] = *in;

		if ( *in == ' ' )
		{
			// don't allow too many consecutive spaces
			if ( spaces > 2 )
				continue;

			spaces++;
		}
		else if ( outpos > 0 && out[outpos - 1] == Q_COLOR_ESCAPE )
		{
			if ( Q_IsColorString( &out[outpos - 1] ) )
			{
				colorlessLen--;

				if ( ColorIndex( *in ) == 0 )
				{
					// Disallow color black in names to prevent players
					// from getting advantage playing in front of black backgrounds
					outpos--;
					continue;
				}
			}
			else
			{
				spaces = 0;
				colorlessLen++;
			}
		}
		else
		{
			spaces = 0;
			colorlessLen++;
		}

		outpos++;
	}

	out[outpos] = '\0';

	// don't allow empty names
	if ( *out == '\0' || colorlessLen == 0 )
		Q_strncpyz( out, "UnnamedPlayer", outSize );
}

void GameLocal::InitSessionData( gclient_t* client, char* userInfo )
{
	clientSession_t* sess;
	const char* value;

	sess = &client->sess;

	// check for team preference, mainly for bots
	value = Info_ValueForKey( userInfo, "teampref" );

	// check for human's team preference set by start server menu
	if ( !value[0] && g_localTeamPref.string[0] && client->pers.localClient ) {
		value = g_localTeamPref.string;

		// clear team so it's only used once
		gameImports->ConsoleVariable_Set( "g_localTeamPref", "" );
	}

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) 
	{
		// always spawn as spectator in team games
		sess->sessionTeam = TEAM_SPECTATOR;
		sess->spectatorState = SPECTATOR_FREE;

		if ( value[0] || g_teamAutoJoin.integer ) 
		{
			//SetTeam( &g_entities[client - level.clients], value );
		}
	}
	else 
	{
		if ( value[0] == 's' ) 
		{
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		}
		else 
		{
			switch ( g_gametype.integer ) 
			{
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 &&
					 level.numNonSpectatorClients >= g_maxGameClients.integer ) 
				{
					sess->sessionTeam = TEAM_SPECTATOR;
				}
				else 
				{
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( level.numNonSpectatorClients >= 2 ) 
				{
					sess->sessionTeam = TEAM_SPECTATOR;
				}
				else 
				{
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}

		sess->spectatorState = SPECTATOR_FREE;
	}

	//AddTournamentQueue( client );

	WriteClientSessionData( client );
}

void GameLocal::ReadSessionData( gclient_t* client )
{
	char s[MAX_STRING_CHARS];
	const char* var;
	int teamLeader;
	int spectatorState;
	int sessionTeam;

	var = va( "session%i", (int)(client - level.clients) );
	gameImports->ConsoleVariable_GetString( var, s, sizeof( s ) );

	sscanf( s, "%i %i %i %i %i %i %i",
			&sessionTeam,
			&client->sess.spectatorNum,
			&spectatorState,
			&client->sess.spectatorClient,
			&client->sess.wins,
			&client->sess.losses,
			&teamLeader
	);

	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;
}

void GameLocal::WriteClientSessionData( gclient_t* client )
{
	const char* s;
	const char* var;

	s = va( "%i %i %i %i %i %i %i",
			client->sess.sessionTeam,
			client->sess.spectatorNum,
			client->sess.spectatorState,
			client->sess.spectatorClient,
			client->sess.wins,
			client->sess.losses,
			client->sess.teamLeader
	);

	var = va( "session%i", (int)(client - level.clients) );

	gameImports->ConsoleVariable_Set( var, s );
}

void GameLocal::WriteSessionData()
{
	int	i;

	gameImports->ConsoleVariable_Set( "session", va( "%i", g_gametype.integer ) );

	for ( i = 0; i < level.maxclients; i++ ) 
	{
		if ( level.clients[i].pers.connected == CON_CONNECTED ) 
		{
			WriteClientSessionData( &level.clients[i] );
		}
	}
}

void GameLocal::RegisterCVars()
{
	int			i;
	cvarTable_t* cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++ ) {
		gameImports->ConsoleVariable_Register( cv->vmCvar, cv->cvarName,
							cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;

		if ( cv->teamShader ) {
			remapped = qtrue;
		}
	}

	// check some things
	if ( g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE ) {
		G_Printf( "g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer );
		gameImports->ConsoleVariable_Set( "g_gametype", "0" );
		gameImports->ConsoleVariable_Update( &g_gametype );
	}

	level.warmupModificationCount = g_warmup.modificationCount;
}

void GameLocal::UpdateCVars()
{
	int i;
	cvarTable_t* cv;
	qboolean remapped = qfalse;

	for ( i = 0, cv = gameCvarTable; i < gameCvarTableSize; i++, cv++ )
	{
		if ( cv->vmCvar ) 
		{
			gameImports->ConsoleVariable_Update( cv->vmCvar );
		
			if ( cv->modificationCount != cv->vmCvar->modificationCount ) 
			{
				cv->modificationCount = cv->vmCvar->modificationCount;

				if ( cv->trackChange ) 
				{
					gameImports->SendServerCommand( -1, va( "print \"Server: %s changed to %s\n\"",
											cv->cvarName, cv->vmCvar->string ) );
				}

				if ( cv->teamShader ) 
				{
					remapped = qtrue;
				}
			}
		}
	}

	//if ( remapped ) 
	//{
	//	G_RemapTeamShaders();
	//}
}

void GameLocal::CheckCVars()
{
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) 
	{
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) 
		{
			gameImports->ConsoleVariable_Set( "g_needpass", "1" );
		}
		
		else 
		{
			gameImports->ConsoleVariable_Set( "g_needpass", "0" );
		}
	}
}

void GameLocal::BroadcastTeamChange( gclient_t* client, int oldTeam )
{
	if ( client->sess.sessionTeam == TEAM_RED ) {
		gameImports->SendServerCommand( -1, va( "cp \"%s" S_COLOR_WHITE " joined the red team.\n\"",
								client->pers.netname ) );
	}
	else if ( client->sess.sessionTeam == TEAM_BLUE ) {
		gameImports->SendServerCommand( -1, va( "cp \"%s" S_COLOR_WHITE " joined the blue team.\n\"",
										client->pers.netname ) );
	}
	else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
		gameImports->SendServerCommand( -1, va( "cp \"%s" S_COLOR_WHITE " joined the spectators.\n\"",
										client->pers.netname ) );
	}
	else if ( client->sess.sessionTeam == TEAM_FREE ) {
		gameImports->SendServerCommand( -1, va( "cp \"%s" S_COLOR_WHITE " joined the battle.\n\"",
								client->pers.netname ) );
	}
}

void GameLocal::CalculateRanks()
{
	int	i;
	int	rank;
	int	score;
	int	newScore;
	gclient_t* cl;

	level.follow1 = -1;
	level.follow2 = -1;
	level.numConnectedClients = 0;
	level.numNonSpectatorClients = 0;
	level.numPlayingClients = 0;
	level.numVotingClients = 0;		// don't count bots

	for ( i = 0; i < ARRAY_LEN( level.numteamVotingClients ); i++ )
		level.numteamVotingClients[i] = 0;

	for ( i = 0; i < level.maxclients; i++ ) 
	{
		if ( level.clients[i].pers.connected != CON_DISCONNECTED ) 
		{
			level.sortedClients[level.numConnectedClients] = i;
			level.numConnectedClients++;

			if ( level.clients[i].sess.sessionTeam != TEAM_SPECTATOR ) 
			{
				level.numNonSpectatorClients++;

				// decide if this should be auto-followed
				if ( level.clients[i].pers.connected == CON_CONNECTED ) 
				{
					level.numPlayingClients++;
					if ( !(g_entities[i].r.svFlags & SVF_BOT) ) 
					{
						level.numVotingClients++;
						if ( level.clients[i].sess.sessionTeam == TEAM_RED )
							level.numteamVotingClients[0]++;
						else if ( level.clients[i].sess.sessionTeam == TEAM_BLUE )
							level.numteamVotingClients[1]++;
					}
					
					if ( level.follow1 == -1 ) 
					{
						level.follow1 = i;
					}

					else if ( level.follow2 == -1 ) 
					{
						level.follow2 = i;
					}
				}
			}
		}
	}

	//qsort( level.sortedClients, level.numConnectedClients,
	//	   sizeof( level.sortedClients[0] ), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if ( g_gametype.integer >= GT_TEAM ) 
	{
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( i = 0; i < level.numConnectedClients; i++ ) 
		{
			cl = &level.clients[level.sortedClients[i]];
			if ( level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE] ) 
			{
				cl->ps.persistant[PERS_RANK] = 2;
			}
			else if ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) 
			{
				cl->ps.persistant[PERS_RANK] = 0;
			}
			else 
			{
				cl->ps.persistant[PERS_RANK] = 1;
			}
		}
	}
	else 
	{
		rank = -1;
		score = 0;
		for ( i = 0; i < level.numPlayingClients; i++ ) 
		{
			cl = &level.clients[level.sortedClients[i]];
			newScore = cl->ps.persistant[PERS_SCORE];

			if ( i == 0 || newScore != score ) 
			{
				rank = i;
				// assume we aren't tied until the next client is checked
				level.clients[level.sortedClients[i]].ps.persistant[PERS_RANK] = rank;
			}
			else 
			{
				// we are tied with the previous client
				level.clients[level.sortedClients[i - 1]].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				level.clients[level.sortedClients[i]].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			
			score = newScore;
			
			if ( g_gametype.integer == GT_SINGLE_PLAYER && level.numPlayingClients == 1 ) 
			{
				level.clients[level.sortedClients[i]].ps.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if ( g_gametype.integer >= GT_TEAM ) 
	{
		gameImports->SetConfigString( CS_SCORES1, va( "%i", level.teamScores[TEAM_RED] ) );
		gameImports->SetConfigString( CS_SCORES2, va( "%i", level.teamScores[TEAM_BLUE] ) );
	}

	else 
	{
		if ( level.numConnectedClients == 0 ) 
		{
			gameImports->SetConfigString( CS_SCORES1, va( "%i", SCORE_NOT_PRESENT ) );
			gameImports->SetConfigString( CS_SCORES2, va( "%i", SCORE_NOT_PRESENT ) );
		}
		else if ( level.numConnectedClients == 1 ) 
		{
			gameImports->SetConfigString( CS_SCORES1, va( "%i", level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE] ) );
			gameImports->SetConfigString( CS_SCORES2, va( "%i", SCORE_NOT_PRESENT ) );
		}
		else 
		{
			gameImports->SetConfigString( CS_SCORES1, va( "%i", level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE] ) );
			gameImports->SetConfigString( CS_SCORES2, va( "%i", level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE] ) );
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the new info to everyone
	if ( level.intermissiontime ) 
	{
		//SendScoreboardMessageToAllClients();
	}
}

void GameLocal::RemoveTournamentLoser()
{
	int clientNum;

	if ( level.numPlayingClients != 2 ) 
	{
		return;
	}

	clientNum = level.sortedClients[1];

	if ( level.clients[clientNum].pers.connected != CON_CONNECTED ) 
	{
		return;
	}

	Entities::IEntity* ent = gEntities[clientNum];
	
	if ( nullptr == ent )
		return;

	auto player = static_cast<Entities::BasePlayer*>( ent );

	// Make them a spectator
	player->SetTeam( "s" );
}

void GameLocal::CheckExitRules()
{
	int			i;
	gclient_t* cl;
	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if ( level.intermissiontime ) 
	{
		CheckIntermissionExit();
		return;
	}

	if ( level.intermissionQueued ) 
	{
		if ( level.time - level.intermissionQueued >= INTERMISSION_DELAY_TIME ) 
		{
			level.intermissionQueued = 0;
			BeginIntermission();
		}
		return;
	}

	// check for sudden death
	if ( ScoreIsTied() ) 
	{
		// always wait for sudden death
		return;
	}

	if ( g_timelimit.integer < 0 || g_timelimit.integer > INT_MAX / 60000 ) 
	{
		G_Printf( "timelimit %i is out of range, defaulting to 0\n", g_timelimit.integer );
		gameImports->ConsoleVariable_Set( "timelimit", "0" );
		gameImports->ConsoleVariable_Update( &g_timelimit );
	}

	if ( g_timelimit.integer && !level.warmupTime ) 
	{
		if ( level.time - level.startTime >= g_timelimit.integer * 60000 ) 
		{
			gameImports->SendServerCommand( -1, "print \"Timelimit hit.\n\"" );
			LogExit( "Timelimit hit." );
			return;
		}
	}

	if ( g_fraglimit.integer < 0 ) 
	{
		G_Printf( "fraglimit %i is out of range, defaulting to 0\n", g_fraglimit.integer );
		gameImports->ConsoleVariable_Set( "fraglimit", "0" );
		gameImports->ConsoleVariable_Update( &g_fraglimit );
	}

	if ( g_gametype.integer < GT_CTF && g_fraglimit.integer ) 
	{
		if ( level.teamScores[TEAM_RED] >= g_fraglimit.integer ) 
		{
			gameImports->SendServerCommand( -1, "print \"Red hit the fraglimit.\n\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_fraglimit.integer ) 
		{
			gameImports->SendServerCommand( -1, "print \"Blue hit the fraglimit.\n\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		for ( i = 0; i < g_maxclients.integer; i++ ) 
		{
			cl = level.clients + i;
			if ( cl->pers.connected != CON_CONNECTED ) 
			{
				continue;
			}
			if ( cl->sess.sessionTeam != TEAM_FREE ) 
			{
				continue;
			}

			if ( cl->ps.persistant[PERS_SCORE] >= g_fraglimit.integer ) 
			{
				LogExit( "Fraglimit hit." );
				gameImports->SendServerCommand( -1, va( "print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\"",
										cl->pers.netname ) );
				return;
			}
		}
	}

	if ( g_capturelimit.integer < 0 ) 
	{
		G_Printf( "capturelimit %i is out of range, defaulting to 0\n", g_capturelimit.integer );
		gameImports->ConsoleVariable_Set( "capturelimit", "0" );
		gameImports->ConsoleVariable_Update( &g_capturelimit );
	}

	if ( g_gametype.integer >= GT_CTF && g_capturelimit.integer ) 
	{
		if ( level.teamScores[TEAM_RED] >= g_capturelimit.integer ) 
		{
			gameImports->SendServerCommand( -1, "print \"Red hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if ( level.teamScores[TEAM_BLUE] >= g_capturelimit.integer ) 
		{
			gameImports->SendServerCommand( -1, "print \"Blue hit the capturelimit.\n\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
	}
}

void GameLocal::BeginIntermission()
{
	int	i;
	Entities::BasePlayer* client;

	if ( level.intermissiontime ) 
	{
		return; // already active
	}

	// if in tournement mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) 
	{
		
		AdjustTournamentScores();
	}

	level.intermissiontime = level.time;
	
	// move all clients to the intermission point
	for ( i = 0; i < level.maxclients; i++ ) 
	{
		client = static_cast<Entities::BasePlayer*>( gEntities[i] );
		if ( nullptr == client )
			continue;
	
		// respawn if dead
		if ( client->health <= 0 ) 
		{
			gameWorld->ClientRespawn( client );
		}
		
		gameWorld->MoveClientToIntermission( client );
	}
	
	// if single player game
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) 
	{
		//::UpdateTournamentInfo();
		//::SpawnModelsOnVictoryPads();
	}
	
	// send the current scoring to all clients
	SendScoreboardMessageToAll();
}

void GameLocal::CheckIntermissionExit()
{
	int			ready, notReady, playerCount;
	int			i;
	gclient_t* cl;
	int			readyMask;

	if ( g_gametype.integer == GT_SINGLE_PLAYER ) 
	{
		return;
	}

	// see which players are ready
	ready = 0;
	notReady = 0;
	readyMask = 0;
	playerCount = 0;
	for ( i = 0; i < g_maxclients.integer; i++ ) 
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}
		if ( g_entities[i].r.svFlags & SVF_BOT ) 
		{
			continue;
		}

		playerCount++;
		if ( cl->readyToExit ) 
		{
			ready++;
			if ( i < 16 ) 
			{
				readyMask |= 1 << i;
			}
		}
		else 
		{
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for ( i = 0; i < g_maxclients.integer; i++ ) 
	{
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) 
		{
			continue;
		}
		cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( level.time < level.intermissiontime + 5000 ) 
	{
		return;
	}

	// only test ready status when there are real players present
	if ( playerCount > 0 ) 
	{
		// if nobody wants to go, clear timer
		if ( !ready ) 
		{
			level.readyToExit = qfalse;
			return;
		}

		// if everyone wants to go, go now
		if ( !notReady ) 
		{
			ExitLevel();
			return;
		}
	}

	// the first person to ready starts the ten second timeout
	if ( !level.readyToExit ) 
	{
		level.readyToExit = qtrue;
		level.exitTime = level.time;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( level.time < level.exitTime + 10000 ) 
	{
		return;
	}

	ExitLevel();
}

void GameLocal::AdjustTournamentScores()
{
	int clientNum;

	clientNum = level.sortedClients[0];
	if ( level.clients[clientNum].pers.connected == CON_CONNECTED ) 
	{
		level.clients[clientNum].sess.wins++;
		ClientUserInfoChanged( clientNum );
	}

	clientNum = level.sortedClients[1];
	if ( level.clients[clientNum].pers.connected == CON_CONNECTED ) 
	{
		level.clients[clientNum].sess.losses++;
		ClientUserInfoChanged( clientNum );
	}
}

void GameLocal::UpdateTournamentInfo()
{
	int i;
	Entities::BasePlayer* player;
	int	playerClientNum;
	int	n, accuracy, perfect, msglen;
		 
	char buf[32];
	char msg[MAX_STRING_CHARS];

	// find the real player
	player = nullptr;
	for ( i = 0; i < level.maxclients; i++ ) 
	{
		if ( nullptr == gEntities[i] )
			continue;

		player = static_cast<Entities::BasePlayer*>( gEntities[i] );
		
		if ( !(player->GetShared()->svFlags & SVF_BOT) ) 
		{
			break;
		}
	}

	// this should never happen!
	if ( !player || i == level.maxclients ) 
	{
		return;
	}

	playerClientNum = i;

	CalculateRanks();

	if ( level.clients[playerClientNum].sess.sessionTeam == TEAM_SPECTATOR ) 
	{
		Com_sprintf( msg, sizeof( msg ), "postgame %i %i 0 0 0 0 0 0", level.numNonSpectatorClients, playerClientNum );
	}
	
	else 
	{
		if ( player->GetClient()->accuracy_shots ) 
		{
			accuracy = player->GetClient()->accuracy_hits * 100 / player->GetClient()->accuracy_shots;
		}
	
		else 
		{
			accuracy = 0;
		}
		
		perfect = (level.clients[playerClientNum].ps.persistant[PERS_RANK] == 0 && player->GetClient()->ps.persistant[PERS_KILLED] == 0) ? 1 : 0;
		
		Com_sprintf( msg, sizeof( msg ), "postgame %i %i %i %i %i %i %i %i", level.numNonSpectatorClients, playerClientNum, accuracy,
					 player->GetClient()->ps.persistant[PERS_IMPRESSIVE_COUNT], player->GetClient()->ps.persistant[PERS_EXCELLENT_COUNT],
					 player->GetClient()->ps.persistant[PERS_GAUNTLET_FRAG_COUNT], player->GetClient()->ps.persistant[PERS_SCORE],
					 perfect );
	}

	msglen = strlen( msg );
	for ( i = 0; i < level.numNonSpectatorClients; i++ ) 
	{
		n = level.sortedClients[i];
		Com_sprintf( buf, sizeof( buf ), " %i %i %i", n, level.clients[n].ps.persistant[PERS_RANK], level.clients[n].ps.persistant[PERS_SCORE] );
		msglen += strlen( buf );
	
		if ( msglen >= sizeof( msg ) ) 
		{
			break;
		}
		
		strcat( msg, buf );
	}

	engine->SendConsoleCommand( EXEC_APPEND, msg );
}

void GameLocal::SendScoreboardMessageToAll()
{
	int	i;

	for ( i = 0; i < level.maxclients; i++ ) 
	{
		if ( level.clients[i].pers.connected == CON_CONNECTED ) 
		{
			DeathmatchScoreboardMessage( gEntities[i] );
		}
	}
}

void GameLocal::DeathmatchScoreboardMessage( Entities::IEntity* ent )
{
	char entry[1024];
	char string[1000];
	int	stringlength;
	int	i, j;
	gclient_t* cl;
	int numSorted, scoreFlags, accuracy, perfect;

	// don't send scores to bots, they don't parse it
	if ( ent->GetShared()->svFlags & SVF_BOT ) 
	{
		return;
	}

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = level.numConnectedClients;

	for ( i = 0; i < numSorted; i++ ) 
	{
		int	ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->pers.connected == CON_CONNECTING ) 
		{
			ping = -1;
		}
		
		else 
		{
			ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
		}

		if ( cl->accuracy_shots ) 
		{
			accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
		}
		
		else 
		{
			accuracy = 0;
		}
		
		perfect = (cl->ps.persistant[PERS_RANK] == 0 && cl->ps.persistant[PERS_KILLED] == 0) ? 1 : 0;

		Com_sprintf( entry, sizeof( entry ),
					 " %i %i %i %i %i %i %i %i %i %i %i %i %i %i", level.sortedClients[i],
					 cl->ps.persistant[PERS_SCORE], ping, (level.time - cl->pers.enterTime) / 60000,
					 scoreFlags, g_entities[level.sortedClients[i]].s.powerups, accuracy,
					 cl->ps.persistant[PERS_IMPRESSIVE_COUNT],
					 cl->ps.persistant[PERS_EXCELLENT_COUNT],
					 cl->ps.persistant[PERS_GAUNTLET_FRAG_COUNT],
					 cl->ps.persistant[PERS_DEFEND_COUNT],
					 cl->ps.persistant[PERS_ASSIST_COUNT],
					 perfect,
					 cl->ps.persistant[PERS_CAPTURES] );

		j = strlen( entry );
		if ( stringlength + j >= sizeof( string ) )
			break;
		strcpy( string + stringlength, entry );
		stringlength += j;
	}

	gameImports->SendServerCommand( ent->GetEntityIndex(), va( "scores %i %i %i%s", i,
							level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
							string ) );
}

bool GameLocal::ScoreIsTied()
{
	int	a, b;

	if ( level.numPlayingClients < 2 ) 
	{
		return false;
	}

	if ( g_gametype.integer >= GT_TEAM ) 
	{
		return (level.teamScores[TEAM_RED] == level.teamScores[TEAM_BLUE]);
	}

	a = level.clients[level.sortedClients[0]].ps.persistant[PERS_SCORE];
	b = level.clients[level.sortedClients[1]].ps.persistant[PERS_SCORE];

	return (a == b);
}

void GameLocal::LogExit( const char* string )
{
	int i, numSorted;
	gclient_t* cl;

	G_LogPrintf( "Exit: %s\n", string );

	level.intermissionQueued = level.time;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	gameImports->SetConfigString( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	numSorted = level.numConnectedClients;
	if ( numSorted > 32 ) 
	{
		numSorted = 32;
	}

	if ( g_gametype.integer >= GT_TEAM ) 
	{
		G_LogPrintf( "red:%i  blue:%i\n",
					 level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE] );
	}

	for ( i = 0; i < numSorted; i++ ) 
	{
		int	ping;

		cl = &level.clients[level.sortedClients[i]];

		if ( cl->sess.sessionTeam == TEAM_SPECTATOR ) 
		{
			continue;
		}
		
		if ( cl->pers.connected == CON_CONNECTING ) 
		{
			continue;
		}

		ping = cl->ps.ping < 999 ? cl->ps.ping : 999;

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->ps.persistant[PERS_SCORE], ping, level.sortedClients[i], cl->pers.netname );
	}
}

void GameLocal::ExitLevel()
{
	int	i;
	gclient_t* cl;
	char nextmap[MAX_STRING_CHARS];
	char d1[MAX_STRING_CHARS];

	//bot interbreeding
	//BotInterbreedEndMatch();

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if ( g_gametype.integer == GT_TOURNAMENT ) {
		if ( !level.restarted ) {
			RemoveTournamentLoser();
			engine->SendConsoleCommand( EXEC_APPEND, "map_restart 0\n" );
			level.restarted = qtrue;
			level.changemap = NULL;
			level.intermissiontime = 0;
		}
		return;
	}

	gameImports->ConsoleVariable_GetString( "nextmap", nextmap, sizeof( nextmap ) );
	gameImports->ConsoleVariable_GetString( "d1", d1, sizeof( d1 ) );

	if ( !Q_stricmp( nextmap, "map_restart 0" ) && Q_stricmp( d1, "" ) ) {
		gameImports->ConsoleVariable_Set( "nextmap", "vstr d2" );
		engine->SendConsoleCommand( EXEC_APPEND, "vstr d1\n" );
	}
	else {
		engine->SendConsoleCommand( EXEC_APPEND, "vstr nextmap\n" );
	}

	level.changemap = NULL;
	level.intermissiontime = 0;

	// reset all the scores so we don't enter the intermission again
	level.teamScores[TEAM_RED] = 0;
	level.teamScores[TEAM_BLUE] = 0;
	for ( i = 0; i < g_maxclients.integer; i++ ) {
		cl = level.clients + i;
		if ( cl->pers.connected != CON_CONNECTED ) {
			continue;
		}
		cl->ps.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before changing to CON_CONNECTING
	WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for ( i = 0; i < g_maxclients.integer; i++ ) 
	{
		if ( level.clients[i].pers.connected == CON_CONNECTED ) 
		{
			level.clients[i].pers.connected = CON_CONNECTING;
		}
	}
}
