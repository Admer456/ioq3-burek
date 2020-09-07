#include "../server/server.hpp"
//#include "../botlib/botlib.hpp"
#include "../qcommon/vm_local.hpp"

struct gentity_s;
struct gentity_t;

#include "../game/Game/IGameImports.h"
#include "GameImportsLocal.h"
//#include "../sys/sys_loadlib.hpp"

extern void SV_GameSendServerCommand( int clientNum, const char* text );
extern qboolean SV_EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t* gEnt, int capsule );
extern void SV_GetUsercmd( int clientNum, usercmd_t* cmd );
extern void SV_LocateGameData( sharedEntity_t* gEnts, int numGEntities, int sizeofGEntity_t, playerState_t* clients, int sizeofGameClient );
extern void SV_GameDropClient( int clientNum, const char* reason );
extern void SV_GetServerinfo( char* buffer, int bufferSize );
extern void SV_SetBrushModel( sharedEntity_t* ent, const char* name );
extern qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
extern void SV_AdjustAreaPortalState( sharedEntity_t* ent, qboolean open );

void GameImportsLocal::AddConsoleCommand( const char* commandName )
{
	// TODO: Implement
}

void GameImportsLocal::RemoveConsoleCommand( const char* commandName )
{
	// Refer to AddConsoleCommand
}

void GameImportsLocal::SendClientCommand( const char* s )
{
	// TODO: Implement
}

void GameImportsLocal::ConsoleVariable_Register( vmCvar_t* cvar, const char* name, const char* value, int flags )
{
	Cvar_Register( cvar, name, value, flags );
}

void GameImportsLocal::ConsoleVariable_Update( vmCvar_t* cvar )
{
	Cvar_Update( cvar );
}

void GameImportsLocal::ConsoleVariable_Set( const char* name, const char* value )
{
	Cvar_SetSafe( name, value );
}

int GameImportsLocal::ConsoleVariable_GetInteger( const char* name )
{
	return Cvar_VariableIntegerValue( name );
}

void GameImportsLocal::ConsoleVariable_GetString( const char* name, char* buffer, int bufferSize )
{
	Cvar_VariableStringBuffer( name, buffer, bufferSize );
}

void GameImportsLocal::LocateGameData( sharedEntity_t* gEnts, int numEntities, int sizeOfEntity, playerState_t* client, int sizeOfClient )
{
	SV_LocateGameData( gEnts, numEntities, sizeOfEntity, client, sizeOfClient );
}

void GameImportsLocal::DropClient( int clientNum, const char* reason )
{
	SV_GameDropClient( clientNum, reason );
}

void GameImportsLocal::SendServerCommand( int clientNum, const char* commandText )
{
	SV_GameSendServerCommand( clientNum, commandText );
}

void GameImportsLocal::SetConfigString( int num, const char* string )
{
	SV_SetConfigstring( num, string );
}

void GameImportsLocal::GetConfigString( int num, char* buffer, int bufferSize )
{
	SV_GetConfigstring( num, buffer, bufferSize );
}

void GameImportsLocal::GetUserInfo( int num, char* buffer, int bufferSize )
{
	SV_GetUserinfo( num, buffer, bufferSize );
}

void GameImportsLocal::SetUserInfo( int num, const char* buffer )
{
	SV_SetUserinfo( num, buffer );
}

void GameImportsLocal::GetServerInfo( char* buffer, int bufferSize )
{
	SV_GetServerinfo( buffer, bufferSize );
}

void GameImportsLocal::SetBrushModel( sharedEntity_t* ent, const char* name )
{
	SV_SetBrushModel( ent, name );
}

void GameImportsLocal::Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask )
{
	vec3_t _mins = { mins[0], mins[1], mins[2] };
	vec3_t _maxs = { maxs[0], maxs[1], maxs[2] };

	SV_Trace( results, start, _mins, _maxs, end, passEntityNum, contentmask, false );
}

void GameImportsLocal::TraceCapsule( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask )
{
	vec3_t _mins = { mins[0], mins[1], mins[2] };
	vec3_t _maxs = { maxs[0], maxs[1], maxs[2] };

	SV_Trace( results, start, _mins, _maxs, end, passEntityNum, contentmask, true );
}

int GameImportsLocal::PointContents( const vec3_t point, int passEntityNum )
{
	return SV_PointContents( point, passEntityNum );
}

bool GameImportsLocal::IsInPVS( const vec3_t p1, const vec3_t p2 )
{
	return SV_inPVS( p1, p2 );
}

bool GameImportsLocal::InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 )
{
	return SV_inPVSIgnorePortals( p1, p2 );
}

void GameImportsLocal::AdjustAreaPortalState( sharedEntity_t* ent, bool open )
{
	SV_AdjustAreaPortalState( ent, open );
}

bool GameImportsLocal::AreasConnected( int area1, int area2 )
{
	return CM_AreasConnected( area1, area2 );
}

void GameImportsLocal::LinkEntity( sharedEntity_t* ent )
{
	SV_LinkEntity( ent );
}

void GameImportsLocal::UnlinkEntity( sharedEntity_t* ent )
{
	SV_UnlinkEntity( ent );
}

int GameImportsLocal::EntitiesInBox( const vec3_t mins, const vec3_t maxs, int* list, int maxcount )
{
	return SV_AreaEntities( mins, maxs, list, maxcount );
}

bool GameImportsLocal::EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t* ent )
{
	return SV_EntityContact( mins, maxs, ent, false );	
}

bool GameImportsLocal::EntityContactCapsule( vec3_t mins, vec3_t maxs, const sharedEntity_t* ent )
{
	return SV_EntityContact( mins, maxs, ent, true );
}

int GameImportsLocal::BotAllocateClient( void )
{
	return SV_BotAllocateClient();
}

void GameImportsLocal::BotFreeClient( int clientNum )
{
	SV_BotFreeClient( clientNum );
}

void GameImportsLocal::GetUsercmd( int clientNum, usercmd_t* cmd )
{
	SV_GetUsercmd( clientNum, cmd );
}

int GameImportsLocal::DebugPolygonCreate( int color, int numPoints, vec3_t* points )
{
	return BotImport_DebugPolygonCreate( color, numPoints, points );
}

void GameImportsLocal::DebugPolygonDelete( int id )
{
	BotImport_DebugPolygonDelete( id );
}
