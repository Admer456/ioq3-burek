#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class Vector;

class KeyValueLibrary final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	KeyValueLibrary() = default;
	~KeyValueLibrary();

	void			AddKeyValue( const char* key, const char* value );
	std::string&	GetValue( const char* keyName );
	KVMap&			GetMap() { return library; }

	// Templ8s didn't work here for some odd reason so I'm gonna specialise
	const char*		GetCString( const char* keyName, const char* defaultValue );
	float			GetFloat( const char* keyName, float defaultValue );
	int				GetInt( const char* keyName, int defaultValue );
	bool			GetBool( const char* keyName, int defaultValue );
	Vector			GetVector( const char* keyName, Vector defaultValue );

	std::string& operator [] ( const char* val )
	{
		return library[val];
	}

private:
	KVMap library;
};

namespace Entities
{
	class BasePlayer;
}

// ============================
// SpawnRegistry
// Keeps records of spawn point locations
// in the map, also supports dynamically
// moving spawnpoints.
//
// The idea is to clean up some server entity 
// slots, so info_player_start,
// info_player_deathmatch etc. don't take them up
// ============================
class SpawnRegistry final
{
public:
	struct SpawnInfo
	{
		size_t entityId{ENTITYNUM_NONE};
		uint32_t spawnPointFlags{ 0 };
		Vector spawnPointPosition{ Vector::Zero };
	};

	// For each classname, there's an array of SpawnInfos
	using SpawnMap = std::unordered_map<EntityClassInfo*, std::vector<SpawnInfo>>;

	SpawnRegistry();
	~SpawnRegistry() = default;

	void			Add( Entities::IEntity* ent, bool isDynamic = false );
	SpawnInfo		GetRandomFurthest( const EntityClassInfo& eci, const Vector& avoidPoint, bool isBot = false );

private:
	SpawnMap		map;
};

// ============================
// GameWorld
// Manages whatever is concerned by the
// game's world - player management,
// entity spawning and other facilities
// ============================
class GameWorld final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	constexpr static size_t MaxEntities = MAX_GENTITIES;
	constexpr static size_t MaxSpawnPoints = 128;

public:
	GameWorld();
	~GameWorld();

	// Deletes entities and stuff
	void			Shutdown();

	// ------ Entity spawning facilities ------

	// Spawns worldspawn, goes on to do some more stuff
	void			SpawnEntities();

	// Spawns worldspawn
	void			SpawnWorldspawn();

	// Dispatches the spawn of an individual entity, from keyvalues
	void			SpawnEntity( KeyValueLibrary& map );

	// Allocates an entity
	template<typename entityType>
	entityType* CreateEntity()
	{
		for ( unsigned int i = MAX_CLIENTS; i < MaxEntities; i++ )
		{
			if ( nullptr == gEntities[i] )
			{
				gEntities[i] = new entityType();
				gEntities[i]->SetEntityIndex( i );
				return static_cast<entityType*>(gEntities[i]);
			}
		}

		engine->Error( "Exceeded maximum number of entities\n" );
		return nullptr;
	}

	// Allocates an entity at a specified index
	template<typename entityType>
	entityType* CreateEntity( const uint16_t& index )
	{
		if ( nullptr != gEntities[index] )
		{
#ifdef _DEBUG
			engine->Error( va( "Entity slot %d is taken, please consult your programmer\n", index ) );
#else
			engine->Print( va( "Entity slot %d is taken, please consult your programmer\n", index ) );
#endif
			return nullptr;
		}

		gEntities[index] = new entityType();
		gEntities[index]->SetEntityIndex( index );
		gEntities[index]->GetState()->number = index;
		return static_cast<entityType*>(gEntities[index]);
	}
	
	// Allocates a temporary entity, for events
	Entities::IEntity* CreateTempEntity( const Vector& origin, int event );

	// Allocates a temporary entity to send a complex event
	void			EmitComplexEvent( const Vector& origin, const Vector& angles, const EventData& ed );

	// Deletes an entity from memory
	void			FreeEntity( Entities::IEntity* ent );

	// ------ Entity interaction ------
	
	// Sweep all entities this entity is touching
	void			TouchTriggers( Entities::IEntity* ent );
	
	// Entity lookup tools

	// Find an entity by its targetname
	Entities::IEntity* FindByName( const char* entityName, Entities::IEntity* lastEntity = nullptr );

	// Find an entity by its classname
	Entities::IEntity* FindByClassname( const char* className, Entities::IEntity* lastEntity = nullptr );

	// Find an entity by keyvalue
	Entities::IEntity* FindByKeyvalue( const char* name, const char* value, Entities::IEntity* lastEntity = nullptr );

	// Find a random entity by its targetname
	Entities::IEntity* FindByNameRandom( const char* entityName );

	// Find a random entity by its classname
	Entities::IEntity* FindByClassnameRandom( const char* className );

	// Retrieves all entities within a radius
	// radius: scan radius
	// bbox: use mins and maxs instead of origin
	// unlinked: include entities that aren't linked into the world too
	std::vector<Entities::IEntity*> EntitiesInRadius( Vector position, float radius, bool bbox = false, bool unlinked = false ) const;

	// ------ Asset loading methods ------
	// NOTE: if the asset is already precached, they will just return its index

	// Models: bsp, iqm, md3, mdr, lwo etc.
	uint32_t		PrecacheModel( const char* name );
	// Sounds: wav, ogg, mp3 etc.
	uint32_t		PrecacheSound( const char* name );
	// Materials a.k.a. Q3 shaders
	uint32_t		PrecacheMaterial( const char* name );

	uint32_t		FindConfigstringIndex( const char* name, uint32_t start, uint32_t max, bool create );

	// ------ Client & players methods ------

	// Locate a client at a spawnpoint
	void			SpawnClient( Entities::BasePlayer* player );
	void			ClientRespawn( Entities::BasePlayer* player );
	void			MoveClientToIntermission( Entities::BasePlayer* player );

	// Spawnpoint registry
	SpawnRegistry*	GetSpawnRegistry();

	// Simple spawnpoint finder
	template<typename entityType>
	SpawnRegistry::SpawnInfo FindSpawnPoint( Vector avoidPoint, bool isBot )
	{
		return spawnRegistry.GetRandomFurthest( entityType::ClassInfo, avoidPoint, isBot );
	}

	// Runs if g_synchronousClients is 0
	void			ClientThink( const uint16_t& clientNum );
	void			ClientThink( Entities::BasePlayer* player );

	// Runs if g_synchronousClients is 1
	void			RunClient( Entities::BasePlayer* player );

private: // Private client methods, some of which will get moved to a gamemode interface eventually
	// TODO: move to something like BasePlayer::PreThink 
	void			ClientThinkReal( Entities::BasePlayer* player );
	// Timer actions to decrease health if over 100 etc.
	void			ClientTimerActions( Entities::BasePlayer* player, int msec );
	// Touch entities nearby
	void			ClientImpacts( Entities::BasePlayer* player, pmove_t* pm );
	// Parse shared events
	void			ClientEvents( Entities::BasePlayer* player, int oldEventSequence );
	// Idle timer
	bool			ClientInactivityTimer( Entities::BasePlayer* player );

	void			SendPendingPredictableEvents( Entities::BasePlayer* player );

	void			FindIntermissionPoint();
	void			ClientIntermissionThink( Entities::BasePlayer* player );
	void			SpectatorThink( Entities::BasePlayer* player );
	void			SpectatorClientEndFrame( Entities::BasePlayer* player );

public:
	void			ClientEndFrame( const uint16_t& clientNum );
	void			ClientEndFrame( Entities::BasePlayer* player );

	// ------ Other ------

	// Reads out keyvalues and populates the stuff
	bool			ParseKeyValues();

	char*			AddKeyvalueToken( const char* string );

	// All the entity keyvalue pairs in the map
	// Each KV library represents one entity
	std::vector<KeyValueLibrary> keyValueLibraries;

private:
	SpawnRegistry	spawnRegistry;
};

extern GameWorld* gameWorld;
