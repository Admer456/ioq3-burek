#include "../server/server.hpp"
//#include "../botlib/botlib.hpp"
#include "../qcommon/vm_local.hpp"

struct gentity_s;
struct gentity_t;

#include "../game/Game/IGameImports.h"
#include "GameImportsLocal.h"
//#include "../sys/sys_loadlib.hpp"
#include "../game/Entities/IEntity.hpp"

typedef struct worldSector_s {
	int		axis;		// -1 = leaf node
	float	dist;
	struct worldSector_s* children[2];
	svEntity_t* entities;
} worldSector_t;

#define	AREA_DEPTH	4
#define	AREA_NODES	64

extern worldSector_t	sv_worldSectors[AREA_NODES];
extern int				sv_numworldSectors;

#define MAX_TOTAL_ENT_LEAFS		128

extern void SV_GameSendServerCommand( int clientNum, const char* text );
extern qboolean SV_EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t* gEnt, int capsule );
extern void SV_GetUsercmd( int clientNum, usercmd_t* cmd );
extern void SV_LocateGameData( sharedEntity_t* gEnts, int numGEntities, int sizeofGEntity_t, playerState_t* clients, int sizeofGameClient );
extern void SV_GameDropClient( int clientNum, const char* reason );
extern void SV_GetServerinfo( char* buffer, int bufferSize );
extern void SV_SetBrushModel( sharedEntity_t* ent, const char* name );
extern qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2 );
extern void SV_AdjustAreaPortalState( sharedEntity_t* ent, qboolean open );

using namespace Entities;

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

void GameImportsLocal::LocateGameData( 
	sharedEntity_t*		gEnts,	int numGEntities,	int sizeofGEntity, 
	Entities::IEntity** iEnts,	int numEntities,	int sizeOfEntity, 
	playerState_t*		client,	int sizeOfClient )
{
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity;
	sv.num_entities = numGEntities;

	sv.entities = iEnts;
	sv.entitySize = sizeOfEntity;
	sv.numEntities = numEntities;

	sv.gameClients = client;
	sv.gameClientSize = sizeOfClient;
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

void GameImportsLocal::SetBrushModel( IEntity* ent, const char* name )
{
	clipHandle_t	h;
	vec3_t			mins, maxs;

	/* Oh, the utopia...
	auto brushComponent = ent->GetComponent<BrushComponent>();
	if ( nullptr == brushComponent )
	{
		Error( "SetBrushModel: entity is not a brush entity!" );
	}
	*/

	if ( !name ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if ( name[0] != '*' ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}

	ent->GetState()->modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->GetState()->modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy( mins, ent->GetEngineShared()->mins );
	VectorCopy( maxs, ent->GetEngineShared()->maxs );
	ent->GetEngineShared()->bmodel = qtrue;

	ent->GetEngineShared()->contents = -1;		// we don't know exactly what is in the brushes

	LinkEntity( ent ); // FIXME: remove
}

void GameImportsLocal::SetBrushModel( sharedEntity_t* ent, const char* name )
{
	SV_SetBrushModel( ent, name );
}

void GameImportsLocal::Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask )
{
	vec_t* _mins = const_cast<vec_t*>( mins );
	vec_t* _maxs = const_cast<vec_t*>( maxs );

	SV_Trace( results, start, _mins, _maxs, end, passEntityNum, contentmask, false );
}

void GameImportsLocal::TraceCapsule( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask )
{
	vec_t* _mins = const_cast<vec_t*>(mins);
	vec_t* _maxs = const_cast<vec_t*>(maxs);

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

void GameImportsLocal::AdjustAreaPortalState( Entities::IEntity* ent, bool open )
{
	svEntity_t* svEnt;

	svEnt = ServerEntityForEntity( ent );
	if ( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}

void GameImportsLocal::AdjustAreaPortalState( sharedEntity_t* ent, bool open )
{
	SV_AdjustAreaPortalState( ent, open );
}

bool GameImportsLocal::AreasConnected( int area1, int area2 )
{
	return CM_AreasConnected( area1, area2 );
}

void GameImportsLocal::LinkEntity( IEntity* gEnt )
{
	worldSector_t* node;
	int			leafs[MAX_TOTAL_ENT_LEAFS];
	int			cluster;
	int			num_leafs;
	int			i, j, k;
	int			area;
	int			lastLeaf;
	float* origin, * angles;
	svEntity_t* ent;

	ent = ServerEntityForEntity( gEnt );

	if ( ent->worldSector ) {
		UnlinkEntity( gEnt );	// unlink from old position
	}

	// encode the size into the entityState_t for client prediction
	if ( gEnt->GetEngineShared()->bmodel ) 
	{
		gEnt->GetState()->solid = SOLID_BMODEL;		// a solid_box will never create this value
	}
	
	else if ( gEnt->GetEngineShared()->contents & (CONTENTS_SOLID | CONTENTS_BODY) )
	{
		// assume that x/y are equal and symetric
		i = gEnt->GetEngineShared()->maxs[0];
		if ( i < 1 )
			i = 1;
		if ( i > 255 )
			i = 255;

		// z is not symetric
		j = (-gEnt->GetEngineShared()->mins[2]);
		if ( j < 1 )
			j = 1;
		if ( j > 255 )
			j = 255;

		// and z maxs can be negative...
		k = (gEnt->GetEngineShared()->maxs[2] + 32);
		if ( k < 1 )
			k = 1;
		if ( k > 255 )
			k = 255;

		gEnt->GetState()->solid = (k << 16) | (j << 8) | i;
	}
	
	else 
	{
		gEnt->GetState()->solid = 0;
	}

	// get the position
	origin = gEnt->GetEngineShared()->currentOrigin;
	angles = gEnt->GetEngineShared()->currentAngles;

	// set the abs box
	if ( gEnt->GetEngineShared()->bmodel && (angles[0] || angles[1] || angles[2]) ) 
	{
		// expand for rotation
		float		max;

		max = RadiusFromBounds( gEnt->GetEngineShared()->mins, gEnt->GetEngineShared()->maxs );
		for ( i = 0; i < 3; i++ ) 
		{
			gEnt->GetEngineShared()->absmin[i] = origin[i] - max;
			gEnt->GetEngineShared()->absmax[i] = origin[i] + max;
		}
	}

	else 
	{
		// normal
		VectorAdd( origin, gEnt->GetEngineShared()->mins, gEnt->GetEngineShared()->absmin );
		VectorAdd( origin, gEnt->GetEngineShared()->maxs, gEnt->GetEngineShared()->absmax );
	}

	// because movement is clipped an epsilon away from an actual edge,
	// we must fully check even when bounding boxes don't quite touch
	gEnt->GetEngineShared()->absmin[0] -= 1;
	gEnt->GetEngineShared()->absmin[1] -= 1;
	gEnt->GetEngineShared()->absmin[2] -= 1;
	gEnt->GetEngineShared()->absmax[0] += 1;
	gEnt->GetEngineShared()->absmax[1] += 1;
	gEnt->GetEngineShared()->absmax[2] += 1;

	// link to PVS leafs
	ent->numClusters = 0;
	ent->lastCluster = 0;
	ent->areanum = -1;
	ent->areanum2 = -1;

	//get all leafs, including solids
	num_leafs = CM_BoxLeafnums( gEnt->GetEngineShared()->absmin, gEnt->GetEngineShared()->absmax,
		leafs, MAX_TOTAL_ENT_LEAFS, &lastLeaf );

	// if none of the leafs were inside the map, the
	// entity is outside the world and can be considered unlinked
	if ( !num_leafs ) {
		return;
	}

	// set areas, even from clusters that don't fit in the entity array
	for ( i = 0; i < num_leafs; i++ ) 
	{
		area = CM_LeafArea( leafs[i] );
		if ( area != -1 ) 
		{
			// doors may legally straggle two areas,
			// but nothing should evern need more than that
			if ( ent->areanum != -1 && ent->areanum != area ) 
			{
				if ( ent->areanum2 != -1 && ent->areanum2 != area && sv.state == SS_LOADING ) 
				{
					Com_DPrintf( "Object %i touching 3 areas at %f %f %f\n",
						gEnt->GetState()->number,
						gEnt->GetEngineShared()->absmin[0], gEnt->GetEngineShared()->absmin[1], gEnt->GetEngineShared()->absmin[2] );
				}
				ent->areanum2 = area;
			}

			else 
			{
				ent->areanum = area;
			}
		}
	}

	// store as many explicit clusters as we can
	ent->numClusters = 0;
	for ( i = 0; i < num_leafs; i++ ) 
	{
		cluster = CM_LeafCluster( leafs[i] );
		if ( cluster != -1 ) 
		{
			ent->clusternums[ent->numClusters++] = cluster;
			if ( ent->numClusters == MAX_ENT_CLUSTERS ) 
			{
				break;
			}
		}
	}

	// store off a last cluster if we need to
	if ( i != num_leafs ) 
	{
		ent->lastCluster = CM_LeafCluster( lastLeaf );
	}

	gEnt->GetEngineShared()->linkcount++;

	// find the first world sector node that the ent's box crosses
	node = sv_worldSectors;
	while ( 1 )
	{
		if ( node->axis == -1 )
			break;
		if ( gEnt->GetEngineShared()->absmin[node->axis] > node->dist )
			node = node->children[0];
		else if ( gEnt->GetEngineShared()->absmax[node->axis] < node->dist )
			node = node->children[1];
		else
			break;		// crosses the node
	}

	// link it in
	ent->worldSector = node;
	ent->nextEntityInWorldSector = node->entities;
	node->entities = ent;

	gEnt->GetEngineShared()->linked = qtrue;
}

void GameImportsLocal::LinkEntity( sharedEntity_t* ent )
{
	SV_LinkEntity( ent );
}

void GameImportsLocal::UnlinkEntity( IEntity* gEnt )
{
	svEntity_t* ent;
	svEntity_t* scan;
	worldSector_t* ws;

	ent = ServerEntityForEntity( gEnt );

	gEnt->GetEngineShared()->linked = qfalse;

	ws = ent->worldSector;
	if ( !ws ) {
		return;		// not linked in anywhere
	}
	ent->worldSector = NULL;

	if ( ws->entities == ent ) {
		ws->entities = ent->nextEntityInWorldSector;
		return;
	}

	for ( scan = ws->entities; scan; scan = scan->nextEntityInWorldSector ) {
		if ( scan->nextEntityInWorldSector == ent ) {
			scan->nextEntityInWorldSector = ent->nextEntityInWorldSector;
			return;
		}
	}

	Com_Printf( "WARNING: SV_UnlinkEntity: not found in worldSector\n" );
}

void GameImportsLocal::UnlinkEntity( sharedEntity_t* ent )
{
	SV_UnlinkEntity( ent );
}

int GameImportsLocal::EntitiesInBox( const vec3_t mins, const vec3_t maxs, int* list, int maxcount )
{
	return SV_AreaEntities( mins, maxs, list, maxcount );
}

bool GameImportsLocal::EntityContact( vec3_t mins, vec3_t maxs, const IEntity* ent, bool capsule )
{
	const float* origin, * angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = ent->GetEngineShared()->currentOrigin;
	angles = ent->GetEngineShared()->currentAngles;

	ch = ClipHandleForEntity( ent );
	CM_TransformedBoxTrace( &trace, vec3_origin, vec3_origin, mins, maxs,
		ch, -1, origin, angles, capsule );

	return trace.startsolid;
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

clipHandle_t GameImportsLocal::ClipHandleForEntity( const IEntity* ent )
{
	if ( ent->GetEngineShared()->bmodel ) 
	{
		// explicit hulls in the BSP model
		return CM_InlineModel( ent->GetState()->modelindex );
	}

	if ( ent->GetEngineShared()->svFlags & SVF_CAPSULE ) {
		// create a temp capsule from bounding box sizes
		return CM_TempBoxModel( ent->GetEngineShared()->mins, ent->GetEngineShared()->maxs, qtrue );
	}

	// create a temp tree from bounding box sizes
	return CM_TempBoxModel( ent->GetEngineShared()->mins, ent->GetEngineShared()->maxs, qfalse );
}

svEntity_t* GameImportsLocal::ServerEntityForEntity( IEntity* ent )
{
	if ( !ent || ent->GetState()->number < 0 || ent->GetState()->number >= MAX_GENTITIES )
	{
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}

	return &sv.svEntities[ent->GetState()->number];
}

IEntity* GameImportsLocal::EntityForServerEntity( svEntity_t* ent )
{
	int		num;
	IEntity* iEnt;

	num = ent - sv.svEntities;
	iEnt = (IEntity*)((byte*)sv.entities + sv.entitySize * (num));

	return iEnt;
}
