#pragma once

namespace Entities
{
	class IEntity;
}

// All the functions that the game DLL will call from the engine
class IGameImports
{
public:
	IGameImports() = default;
	virtual ~IGameImports() = default;

	virtual void			AddConsoleCommand( const char* commandName ) = 0;
	virtual void			RemoveConsoleCommand( const char* commandName ) = 0;
	virtual void			SendClientCommand( const char* s ) = 0;
	virtual void			ConsoleVariable_Register( vmCvar_t* cvar, const char* name, const char* value, int flags ) = 0;
	virtual void			ConsoleVariable_Update( vmCvar_t* cvar ) = 0;
	virtual void			ConsoleVariable_Set( const char* name, const char* value ) = 0;
	virtual int				ConsoleVariable_GetInteger( const char* name ) = 0;
	virtual void			ConsoleVariable_GetString( const char* name, char* buffer, int bufferSize ) = 0;

	virtual void			LocateGameData( sharedEntity_t* gEnts, int numGEntities, int sizeofGEntity, Entities::IEntity** iEnts, int numEntities, int sizeOfEntity, playerState_t* client, int sizeOfClient ) = 0;

	virtual void			DropClient( int clientNum, const char* reason ) = 0;
	virtual void			SendServerCommand( int clientNum, const char* commandText ) = 0;

	virtual void			SetConfigString( int num, const char* string ) = 0;
	virtual void			GetConfigString( int num, char* buffer, int bufferSize ) = 0;

	virtual void			GetUserInfo( int num, char* buffer, int bufferSize ) = 0;
	virtual void			SetUserInfo( int num, const char* buffer ) = 0;

	virtual void			GetServerInfo( char* buffer, int bufferSize ) = 0;

	virtual void			SetBrushModel( Entities::IEntity* ent, const char* name ) = 0;
	virtual void			SetBrushModel( sharedEntity_t* ent, const char* name ) = 0;

	virtual void			Trace( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) = 0;
	virtual void			TraceCapsule( trace_t* results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) = 0;

	virtual int				PointContents( const vec3_t point, int passEntityNum ) = 0;

	virtual bool			IsInPVS( const vec3_t p1, const vec3_t p2 ) = 0;
	virtual bool			InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) = 0;

	virtual void			AdjustAreaPortalState( Entities::IEntity* ent, bool open ) = 0;
	virtual void			AdjustAreaPortalState( sharedEntity_t* ent, bool open ) = 0;
	virtual bool			AreasConnected( int area1, int area2 ) = 0;

	virtual void			LinkEntity( Entities::IEntity* ent ) = 0;
	virtual void			LinkEntity( sharedEntity_t* ent ) = 0;

	virtual void			UnlinkEntity( Entities::IEntity* ent ) = 0;
	virtual void			UnlinkEntity( sharedEntity_t* ent ) = 0;

	virtual int				EntitiesInBox( const vec3_t mins, const vec3_t maxs, int* list, int maxcount ) = 0;

	virtual bool			EntityContact( vec3_t mins, vec3_t maxs, const Entities::IEntity* ent, bool capsule = false ) = 0;
	virtual bool			EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t* ent ) = 0;
	virtual bool			EntityContactCapsule( vec3_t mins, vec3_t maxs, const sharedEntity_t* ent ) = 0;

	virtual int				BotAllocateClient( void ) = 0;
	virtual void			BotFreeClient( int clientNum ) = 0;
	virtual void			GetUsercmd( int clientNum, usercmd_t* cmd ) = 0;

	virtual int				DebugPolygonCreate( int color, int numPoints, vec3_t* points ) = 0;
	virtual void			DebugPolygonDelete( int id ) = 0;

	// BUREKTech new engine functions
	// TODO: make these work on headless servers

	virtual int				TagIndexForName( qhandle_t model, const char* tagName ) = 0;
	virtual const char*		TagNameForIndex( qhandle_t model, int tagId ) = 0;

	// LerpTag - calculates the transform of a bone at a given animation moment
	// returns: false if the tag cannot be found, true if it can
	// tag: output transform
	// fraction: interpolation between startFrame and endFrame, 0.0 to 1.0
	// tagName: for skeletally-animated models, this is the joint name, otherwise it's the tag name
	virtual bool			LerpTag( orientation_t* tag, qhandle_t model, int startFrame, int endFrame, float fraction, const char* tagName ) = 0;

	// ModelBounds - retrieves the bounding box of a model
	virtual void			ModelBounds( qhandle_t model, vec3_t outMins, vec3_t outMaxs ) = 0;

	// LightForPoint - samples lighting from the lightgrid
	// returns: false if there's no light grid data, true if there is
	// point: the world position to sample from
	virtual bool			LightForPoint( vec3_t point, vec3_t outAmbientLight, vec3_t outDirectLight, vec3_t outLightDirection ) = 0;
};
// BotLib is not here

extern IGameImports* gameImports;
