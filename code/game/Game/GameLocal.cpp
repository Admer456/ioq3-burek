#include "Maths/Vector.hpp"
#include "Game/g_local.hpp"

#include "Game/IGame.h"
#include "Game/IGameImports.h"
#include "Game/GameExportImport.h"
#include "Game/GameLocal.h"

#include "../qcommon/IEngineExports.h"

#include "Entities/IEntity.hpp"
#include "Entities/BaseEntity.hpp"
#include "Entities/BasePlayer.hpp"

#include "Game/GameWorld.hpp"

extern void G_InitGame( int levelTime, int randomSeed, int restart );
extern void G_ShutdownGame( int restart );
extern void ClientUserinfoChanged( int clientNum );
extern void G_RunFrame( int levelTime );

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

	return ::G_InitGame( levelTime, randomSeed, restart );
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
	if ( ent->GetShared()->linked ) { // Crash happins on dis line
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
